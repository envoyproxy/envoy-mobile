#pragma once

#include "envoy/event/deferred_deletable.h"
#include "envoy/event/dispatcher.h"

#include "common/common/logger.h"
#include "common/common/thread.h"
#include "common/common/thread_synchronizer.h"

#include "library/common/types/c_types.h"

namespace Envoy {
namespace Event {

/**
 * Wrapper around Envoy's Event::Dispatcher that queues callbacks until drain() is called. Future versions may support correct calling semantics after the underlying dispatcher has been terminated/deleted or before it has been created.
 */
class ProvisionalDispatcher : public Logger::Loggable<Logger::Id::main> {
public:
  ProvisionalDispatcher(Event::Dispatcher& event_dispatcher) : event_dispatcher_(event_dispatcher) {}

  /**
   * Drains all queued callbacks to the real dispatcher. Must be called after the underlying
   * dispatcher is running. Further posts will be transparently passed through.
   */
  void drain();

  //TODO(goaway): return ENVOY_FAILURE after the underlying dispatcher has exited.
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

  /**
   * Submits an item for deferred delete. Must be called from context where ProvisionalDispatcher::isThreadSafe() is true.
   */
  void deferredDelete(DeferredDeletablePtr&& to_delete);

  // Used for testing.
  Thread::ThreadSynchronizer& synchronizer() { return synchronizer_; }

private:
  // TODO(goaway): This class supports a straightforward case-specific lock-free implementation, but uses heavyweight synchronization for expediency at present.
  Thread::MutexBasicLockable state_lock_;
  bool drained_ GUARDED_BY(state_lock_){};
  std::list<Event::PostCb> init_queue_ GUARDED_BY(state_lock_);
  Event::Dispatcher& event_dispatcher_;
  Thread::ThreadSynchronizer synchronizer_;
};

} // namespace Event
} // namespace Envoy
