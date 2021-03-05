#include "library/common/event/provisional_dispatcher.h"

#include "common/common/lock_guard.h"

#include "library/common/thread/lock_guard.h"

namespace Envoy {
namespace Event {

/**
 * IMPORTANT: stream closure semantics in envoy mobile depends on the fact that the HCM fires a
 * stream reset when the remote side of the stream is closed but the local side remains open.
 * In other words the HCM (like the rest of Envoy) dissallows locally half-open streams.
 * If this changes in Envoy, this file will need to change as well.
 * For implementation details @see Dispatcher::DirectStreamCallbacks::closeRemote.
 */

/**
 * Drain initial queue into the Event::Dispatcher and begin pass-through dispatch.
 */
void ProvisionalDispatcher::start(Event::Dispatcher& event_dispatcher) {
  Thread::LockGuard lock(ready_lock_);

  // Drain the init_queue_ into the event_dispatcher_.
  for (const Event::PostCb& cb : init_queue_) {
    event_dispatcher.post(cb);
  }

  event_dispatcher_ = &event_dispatcher;
}

void ProvisionalDispatcher::post(Event::PostCb callback) {
  Thread::LockGuard lock(ready_lock_);

  // If the event_dispatcher_ is set, then post the functor directly to it.
  if (event_dispatcher_ != nullptr) {
    event_dispatcher_->post(callback);
    return;
  }

  // Otherwise, push the functor to the init_queue_ which will be drained once the
  // event_dispatcher_ is ready.
  init_queue_.push_back(callback);
}

} // namespace Event
} // namespace Envoy
