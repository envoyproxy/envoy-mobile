#include "library/common/event/provisional_dispatcher.h"

#include "common/common/lock_guard.h"

#include "library/common/thread/lock_guard.h"

namespace Envoy {
namespace Event {

void ProvisionalDispatcher::drain() {
  Thread::LockGuard lock(state_lock_);

  ASSERT(!drained_);

  for (const Event::PostCb& cb : init_queue_) {
    event_dispatcher_.post(cb);
  }

  drained_ = true;
}

envoy_status_t ProvisionalDispatcher::post(Event::PostCb callback) {
  Thread::LockGuard lock(state_lock_);

  if (drained_) {
    event_dispatcher_.post(callback);
    return ENVOY_SUCCESS;
  }

  init_queue_.push_back(callback);
  return ENVOY_SUCCESS;
}

bool ProvisionalDispatcher::isThreadSafe() {
  // Doesn't require locking because if a thread has a stale view of drained_, then by definition
  // this wasn't a threadsafe call.
  return TS_UNCHECKED_READ(drained_) && isThreadSafe();
}

void ProvisionalDispatcher::deferredDelete(DeferredDeletablePtr&& to_delete) {
  RELEASE_ASSERT(isThreadSafe(),
                 "ProvisionalDispatcher::deferredDelete must be called from a threadsafe context");
  event_dispatcher_.deferredDelete(std::move(to_delete));
}

} // namespace Event
} // namespace Envoy
