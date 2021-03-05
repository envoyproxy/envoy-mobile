#pragma once

#include "envoy/buffer/buffer.h"
#include "envoy/event/deferred_deletable.h"
#include "envoy/event/dispatcher.h"
#include "envoy/http/api_listener.h"

#include "common/common/logger.h"
#include "common/common/thread.h"
#include "common/common/thread_synchronizer.h"

#include "absl/container/flat_hash_map.h"
#include "absl/types/optional.h"
#include "library/common/types/c_types.h"

namespace Envoy {
namespace Event {

/**
 * Wrapper around Envoy's Event::Dispatcher that queues functors until the underlying
 * Event::Dispatcher has started.
 */
class ProvisionalDispatcher : public Logger::Loggable<Logger::Id::http> {
public:
  Dispatcher(std::atomic<envoy_network_t>& preferred_network);

  //TODO(goaway): return ENVOY_FAILURE after the underlying dispatcher has exited.
  /**
   * Drains all queued callbacks to the real dispatcher once it becomes available. Further posts
   * will be transparently passed through to it until it exits.
   * @param event_dispatcher, the underlying dispatcher to run posted callbacks..
   */
  void start(Event::Dispatcher& event_dispatcher);

  /**
   * Before the underlying dispatcher is running, queues posted callbacks; afterwards passes them through.
   * @param callback, the callback to be dispatched.
   * @return should return ENVOY_FAILURE when the underlying dispatcher exits, but at present it always returns ENVOY_SUCCESS.
   */
  envoy_status_t post(Event::PostCb callback);

  /**
   * @return false before the underlying dispatcher is running, otherwise the result of the underlying call to Event::Dispatcher::isThreadSafe().
   */
  bool isThreadSafe();

  // Used for testing.
  Thread::ThreadSynchronizer& synchronizer() { return synchronizer_; }

private:
  Thread::MutexBasicLockable ready_lock_;
  std::list<Event::PostCb> init_queue_ GUARDED_BY(ready_lock_);
  Event::Dispatcher* event_dispatcher_ GUARDED_BY(ready_lock_){};
  Thread::ThreadSynchronizer synchronizer_;
};

} // namespace Event
} // namespace Envoy
