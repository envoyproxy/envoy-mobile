#pragma once

#include "envoy/server/lifecycle_notifier.h"

#include "source/common/common/logger.h"
#include "source/common/upstream/logical_dns_cluster.h"

#include "absl/base/call_once.h"
#include "extension_registry.h"
#include "library/common/common/lambda_logger_delegate.h"
#include "library/common/engine_common.h"
#include "library/common/http/client.h"
#include "library/common/types/c_types.h"

namespace Envoy {

class Engine : public Logger::Loggable<Logger::Id::main> {
public:
  /**
   * Constructor for a new engine instance.
   * @param callbacks, the callbacks to use for engine lifecycle monitoring.
   * @param logger, the callbacks to use for engine logging.
   * @param preferred_network, hook to obtain the preferred network for new streams.
   */
  Engine(envoy_engine_callbacks callbacks, envoy_logger logger,
         std::atomic<envoy_network_t>& preferred_network);

  /**
   * Engine destructor.
   */
  ~Engine();

  /**
   * Run the engine with the provided configuration.
   * @param config, the Envoy bootstrap configuration to use.
   * @param log_level, the log level.
   */
  envoy_status_t run(std::string config, std::string log_level);

  /**
   * Immediately terminate the engine, if running.
   */
  envoy_status_t terminate();

  /**
   * Accessor for the provisional event dispatcher.
   * @return Event::ProvisionalDispatcher&, the engine dispatcher.
   */
  Event::ProvisionalDispatcher& dispatcher();

  /**
   * Accessor for the http client. Must be called from the dispatcher's context.
   * @return Http::Client&, the (default) http client.
   */
  Http::Client& httpClient();

  /**
   * Increment a counter with a given string of elements and by the given count.
   * @param elements, joined elements of the timeseries.
   * @param tags, custom tags of the reporting stat.
   * @param count, amount to add to the counter.
   */
  envoy_status_t recordCounterInc(const std::string& elements, envoy_stats_tags tags,
                                  uint64_t count);

  /**
   * Set a gauge of a given string of elements with the given value.
   * @param elements, joined elements of the timeseries.
   * @param tags, custom tags of the reporting stat.
   * @param value, value to set to the gauge.
   */
  envoy_status_t recordGaugeSet(const std::string& elements, envoy_stats_tags tags, uint64_t value);

  /**
   * Add to the gauge with the given string of elements and by the given amount.
   * @param elements, joined elements of the timeseries.
   * @param tags, custom tags of the reporting stat.
   * @param amount, amount to add to the gauge.
   */
  envoy_status_t recordGaugeAdd(const std::string& elements, envoy_stats_tags tags,
                                uint64_t amount);

  /**
   * Subtract from the gauge with the given string of elements and by the given amount.
   * @param elements, joined elements of the timeseries.
   * @param tags, custom tags of the reporting stat.
   * @param amount, amount to subtract from the gauge.
   */
  envoy_status_t recordGaugeSub(const std::string& elements, envoy_stats_tags tags,
                                uint64_t amount);

  /**
   * Record a value for the histogram with the given string of elements and unit measurement
   * @param elements, joined elements of the timeseries.
   * @param tags, custom tags of the reporting stat.
   * @param value, value to add to the aggregated distribution of values for quantile calculations
   * @param unit_measure, the unit of measurement (e.g. milliseconds, bytes, etc.)
   */
  envoy_status_t recordHistogramValue(const std::string& elements, envoy_stats_tags tags,
                                      uint64_t value, envoy_histogram_stat_unit_t unit_measure);

  /**
   * Flush the stats sinks outside of a flushing interval.
   * Note: stat flushing is done asynchronously, this function will never block.
   * This is a noop if called before the underlying EnvoyEngine has started.
   */
  void flushStats();

private:
  envoy_status_t main(std::string config, std::string log_level);

  Event::Dispatcher* event_dispatcher_{};
  Stats::ScopePtr client_scope_;
  Stats::StatNameSetPtr stat_name_set_;
  envoy_engine_callbacks callbacks_;
  envoy_logger logger_;
  Thread::MutexBasicLockable mutex_;
  Thread::CondVar cv_;
  Http::ClientPtr http_client_;
  Event::ProvisionalDispatcherPtr dispatcher_;
  Logger::LambdaDelegatePtr lambda_logger_{};
  Server::Instance* server_{};
  Server::ServerLifecycleNotifier::HandlePtr postinit_callback_handler_;
  std::atomic<envoy_network_t>& preferred_network_;
  // main_thread_ should be destroyed first, hence it is the last member variable. Objects with
  // instructions scheduled on the main_thread_ need to have a longer lifetime.
  std::thread main_thread_{}; // Empty placeholder to be populated later.
};

using EngineSharedPtr = std::shared_ptr<Engine>;
using EngineWeakPtr = std::weak_ptr<Engine>;

} // namespace Envoy
