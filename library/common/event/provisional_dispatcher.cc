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

} // namespace Event
} // namespace Envoy
