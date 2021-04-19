#include "library/common/engine.h"

#include "envoy/stats/histogram.h"

#include "common/common/lock_guard.h"

#include "library/common/data/utility.h"
#include "library/common/stats/utility.h"

namespace Envoy {

Engine::Engine(envoy_engine_callbacks callbacks, envoy_logger logger,
               std::atomic<envoy_network_t>& preferred_network)
    : callbacks_(callbacks), logger_(logger),
      dispatcher_(std::make_unique<Event::ProvisionalDispatcher>()),
      preferred_network_(preferred_network) {
  // Ensure static factory registration occurs on time.
  // TODO: ensure this is only called one time once multiple Engine objects can be allocated.
  // https://github.com/lyft/envoy-mobile/issues/332
  ExtensionRegistry::registerFactories();
}

envoy_status_t Engine::run(const std::string config, const std::string log_level) {
  // Start the Envoy on the dedicated thread. Note: due to how the assignment operator works with
  // std::thread, main_thread_ is the same object after this call, but its state is replaced with
  // that of the temporary. The temporary object's state becomes the default state, which does
  // nothing.
  main_thread_ = std::thread(&Engine::main, this, std::string(config), std::string(log_level));
  return ENVOY_SUCCESS;
}

envoy_status_t Engine::main(const std::string config, const std::string log_level) {
  // Using unique_ptr ensures main_common's lifespan is strictly scoped to this function.
  std::unique_ptr<MobileMainCommon> main_common;
  {
    Thread::LockGuard lock(mutex_);
    try {
      const std::string name = "envoy";
      const std::string config_flag = "--config-yaml";
      const std::string log_flag = "-l";
      const std::string concurrency_option = "--concurrency";
      const std::string concurrency_arg = "0";
      std::vector<const char*> envoy_argv = {name.c_str(),
                                             config_flag.c_str(),
                                             config.c_str(),
                                             concurrency_option.c_str(),
                                             concurrency_arg.c_str(),
                                             log_flag.c_str(),
                                             log_level.c_str(),
                                             nullptr};

      main_common = std::make_unique<MobileMainCommon>(envoy_argv.size() - 1, envoy_argv.data());
      server_ = main_common->server();
      event_dispatcher_ = &server_->dispatcher();
      if (logger_.log) {
        lambda_logger_ =
            std::make_unique<Logger::LambdaDelegate>(logger_, Logger::Registry::getSink());
      }

      cv_.notifyAll();
    } catch (const Envoy::NoServingException& e) {
      std::cerr << e.what() << std::endl;
      return ENVOY_FAILURE;
    } catch (const Envoy::MalformedArgvException& e) {
      std::cerr << e.what() << std::endl;
      return ENVOY_FAILURE;
    } catch (const Envoy::EnvoyException& e) {
      std::cerr << e.what() << std::endl;
      return ENVOY_FAILURE;
    }

    // Note: We're waiting longer than we might otherwise to drain to the main thread's dispatcher.
    // This is because we're not simply waiting for its availability and for it to have started, but
    // also because we're waiting for clusters to have done their first attempt at DNS resolution.
    // When we improve synchronous failure handling and/or move to dynamic forwarding, we only need
    // to wait until the dispatcher is running (and can drain by enqueueing a drain callback on it,
    // as we did previously).
    postinit_callback_handler_ = main_common->server()->lifecycleNotifier().registerCallback(
        Envoy::Server::ServerLifecycleNotifier::Stage::PostInit, [this]() -> void {
          client_scope_ = server_->serverFactoryContext().scope().createScope("pulse.");
          // StatNameSet is lock-free, the benefit of using it is being able to create StatsName
          // on-the-fly without risking contention on system with lots of threads.
          // It also comes with ease of programming.
          stat_name_set_ = client_scope_->symbolTable().makeSet("pulse");
          auto api_listener = server_->listenerManager().apiListener()->get().http();
          ASSERT(api_listener.has_value());
          http_client_ = std::make_unique<Http::Client>(api_listener.value(), *dispatcher_,
                                                        server_->serverFactoryContext().scope(),
                                                        preferred_network_);
          dispatcher_->drain(server_->dispatcher());
          if (callbacks_.on_engine_running != nullptr) {
            callbacks_.on_engine_running(callbacks_.context);
          }
        });
  } // mutex_

  // The main run loop must run without holding the mutex, so that the destructor can acquire it.
  bool run_success = main_common->run();
  // The above call is blocking; at this point the event loop has exited.

  // Ensure destructors run on Envoy's main thread.
  postinit_callback_handler_.reset(nullptr);
  client_scope_.reset(nullptr);
  stat_name_set_.reset();
  lambda_logger_.reset(nullptr);
  main_common.reset(nullptr);

  callbacks_.on_exit(callbacks_.context);

  return run_success ? ENVOY_SUCCESS : ENVOY_FAILURE;
}

Engine::~Engine() {
  // If we're already on the main thread, it should be safe to simply destruct.
  if (!main_thread_.joinable()) {
    return;
  }

  // If we're not on the main thread, we need to be sure that MainCommon is finished being
  // constructed so we can dispatch shutdown.
  {
    Thread::LockGuard lock(mutex_);

    if (!event_dispatcher_) {
      cv_.wait(mutex_);
    }

    ASSERT(event_dispatcher_);

    if (std::this_thread::get_id() == main_thread_.get_id()) {
      event_dispatcher_->shutdown();
    }

    // Exit the event loop and finish up in Engine::run(...)
    event_dispatcher_->exit();
  } // _mutex

  // Now detach the main thread to let it wrap things up.
  main_thread_.detach();
}

envoy_status_t Engine::recordCounterInc(const std::string& elements, envoy_stats_tags tags,
                                        uint64_t count) {
  if (server_ && client_scope_ && stat_name_set_) {
    Stats::StatNameTagVector tags_vctr =
        Stats::Utility::transformToStatNameTagVector(tags, stat_name_set_);
    std::string name = Stats::Utility::sanitizeStatsName(elements);
    server_->dispatcher().post([this, name, tags_vctr, count]() -> void {
      Stats::Utility::counterFromElements(*client_scope_, {Stats::DynamicName(name)}, tags_vctr)
          .add(count);
    });
    return ENVOY_SUCCESS;
  }
  return ENVOY_FAILURE;
}

envoy_status_t Engine::recordGaugeSet(const std::string& elements, envoy_stats_tags tags,
                                      uint64_t value) {
  if (server_ && client_scope_ && stat_name_set_) {
    Stats::StatNameTagVector tags_vctr =
        Stats::Utility::transformToStatNameTagVector(tags, stat_name_set_);
    std::string name = Stats::Utility::sanitizeStatsName(elements);
    server_->dispatcher().post([this, name, tags_vctr, value]() -> void {
      Stats::Utility::gaugeFromElements(*client_scope_, {Stats::DynamicName(name)},
                                        Stats::Gauge::ImportMode::NeverImport, tags_vctr)
          .set(value);
    });
    return ENVOY_SUCCESS;
  }
  return ENVOY_FAILURE;
}

envoy_status_t Engine::recordGaugeAdd(const std::string& elements, envoy_stats_tags tags,
                                      uint64_t amount) {
  if (server_ && client_scope_ && stat_name_set_) {
    Stats::StatNameTagVector tags_vctr =
        Stats::Utility::transformToStatNameTagVector(tags, stat_name_set_);
    std::string name = Stats::Utility::sanitizeStatsName(elements);
    server_->dispatcher().post([this, name, tags_vctr, amount]() -> void {
      Stats::Utility::gaugeFromElements(*client_scope_, {Stats::DynamicName(name)},
                                        Stats::Gauge::ImportMode::NeverImport, tags_vctr)
          .add(amount);
    });
    return ENVOY_SUCCESS;
  }
  return ENVOY_FAILURE;
}

envoy_status_t Engine::recordGaugeSub(const std::string& elements, envoy_stats_tags tags,
                                      uint64_t amount) {
  if (server_ && client_scope_ && stat_name_set_) {
    Stats::StatNameTagVector tags_vctr =
        Stats::Utility::transformToStatNameTagVector(tags, stat_name_set_);
    std::string name = Stats::Utility::sanitizeStatsName(elements);
    server_->dispatcher().post([this, name, tags_vctr, amount]() -> void {
      Stats::Utility::gaugeFromElements(*client_scope_, {Stats::DynamicName(name)},
                                        Stats::Gauge::ImportMode::NeverImport, tags_vctr)
          .sub(amount);
    });
    return ENVOY_SUCCESS;
  }
  return ENVOY_FAILURE;
}

envoy_status_t Engine::recordHistogramValue(const std::string& elements, envoy_stats_tags tags,
                                            uint64_t value,
                                            envoy_histogram_stat_unit_t unit_measure) {
  if (server_ && client_scope_ && stat_name_set_) {
    Stats::StatNameTagVector tags_vctr =
        Stats::Utility::transformToStatNameTagVector(tags, stat_name_set_);
    std::string name = Stats::Utility::sanitizeStatsName(elements);
    Stats::Histogram::Unit envoy_unit_measure = Stats::Histogram::Unit::Unspecified;
    switch (unit_measure) {
    case MILLISECONDS:
      envoy_unit_measure = Stats::Histogram::Unit::Milliseconds;
      break;
    case MICROSECONDS:
      envoy_unit_measure = Stats::Histogram::Unit::Microseconds;
      break;
    case BYTES:
      envoy_unit_measure = Stats::Histogram::Unit::Bytes;
      break;
    case UNSPECIFIED:
      envoy_unit_measure = Stats::Histogram::Unit::Unspecified;
      break;
    }

    server_->dispatcher().post([this, name, tags_vctr, value, envoy_unit_measure]() -> void {
      Stats::Utility::histogramFromElements(*client_scope_, {Stats::DynamicName(name)},
                                            envoy_unit_measure, tags_vctr)
          .recordValue(value);
    });
    return ENVOY_SUCCESS;
  }
  return ENVOY_FAILURE;
}

Event::ProvisionalDispatcher& Engine::dispatcher() { return *dispatcher_; }

Http::Client& Engine::httpClient() {
  RELEASE_ASSERT(dispatcher_->isThreadSafe(),
                 "httpClient must be accessed from dispatcher's context");
  return *http_client_;
}

} // namespace Envoy
