#include "library/common/engine.h"

#include "common/common/lock_guard.h"

namespace Envoy {

// As a server, Envoy's static factory registration happens when main is run. However, when compiled
// as a library, there is no guarantee that such registration will happen before the names are
// needed. The following calls ensure that registration happens before the entities are needed. Note
// that as more registrations are needed, explicit initialization calls will need to be added here.
static void registerFactories() {
  Envoy::Extensions::Clusters::DynamicForwardProxy::forceRegisterClusterFactory();
  Envoy::Extensions::HttpFilters::DynamicForwardProxy::
      forceRegisterDynamicForwardProxyFilterFactory();
  Envoy::Extensions::HttpFilters::RouterFilter::forceRegisterRouterFilterConfig();
  Envoy::Extensions::NetworkFilters::HttpConnectionManager::
      forceRegisterHttpConnectionManagerFilterConfigFactory();
  Envoy::Extensions::TransportSockets::RawBuffer::forceRegisterDownstreamRawBufferSocketFactory();
  Envoy::Extensions::TransportSockets::RawBuffer::forceRegisterUpstreamRawBufferSocketFactory();
  Envoy::Extensions::TransportSockets::Tls::forceRegisterUpstreamSslSocketFactory();
  Envoy::Upstream::forceRegisterLogicalDnsClusterFactory();
}

absl::once_flag Engine::register_once_;

Engine::Engine(const char* config, const char* log_level,
               std::atomic<envoy_network_t>& preferred_network) {
  // Ensure static factory registration occurs on time.
  absl::call_once(register_once_, registerFactories);

  // Create the Http::Dispatcher first since it contains initial queueing logic.
  // TODO: consider centralizing initial queueing in this class.
  http_dispatcher_ = std::make_unique<Http::Dispatcher>(preferred_network);

  const char* envoy_argv[] = {strdup("envoy"), strdup("--config-yaml"), strdup(config),
                              strdup("-l"),    strdup(log_level),       nullptr};

  // Start the Envoy on a dedicated thread.
  main_thread_ = std::thread(&Engine::run, this, envoy_argv);
}

envoy_status_t Engine::run(const char** envoy_argv) {
  {
    Thread::LockGuard lock(mutex_);
    try {
      main_common_ = std::make_unique<Envoy::MainCommon>(5, envoy_argv);
      cv_.notifyOne();
    } catch (const Envoy::NoServingException& e) {
      return ENVOY_SUCCESS;
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
    auto server = main_common_->server();
    stageone_callback_handler_ = main_common_->server()->lifecycleNotifier().registerCallback(
        Envoy::Server::ServerLifecycleNotifier::Stage::PostInit, [this, server]() -> void {
          http_dispatcher_->ready(server->dispatcher(), server->clusterManager());
        });
  } // mutex_

  // The main run loop must run without holding the mutex, so that the destructor can acquire it.
  return TS_UNCHECKED_READ(main_common_)->run() ? ENVOY_SUCCESS : ENVOY_FAILURE;
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
    if (!main_common_) {
      cv_.wait(mutex_);
    }
    ASSERT(main_common_);
    main_common_->server()->shutdown();
  } // _mutex

  // Now we wait for the main thread to wrap things up.
  main_thread_.join();
}

Http::Dispatcher& Engine::httpDispatcher() { return *http_dispatcher_; }

} // namespace Envoy
