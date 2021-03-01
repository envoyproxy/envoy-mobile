#pragma once

#include "envoy/buffer/buffer.h"
#include "envoy/event/deferred_deletable.h"
#include "envoy/event/dispatcher.h"
#include "envoy/http/api_listener.h"
#include "envoy/http/codec.h"
#include "envoy/http/header_map.h"
#include "envoy/stats/stats_macros.h"

#include "common/common/logger.h"
#include "common/common/thread.h"
#include "common/common/thread_synchronizer.h"
#include "common/http/codec_helper.h"

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

  void start(Event::Dispatcher& event_dispatcher);
  void post(Event::PostCb callback);
  void isThreadSafe();

  // Used for testing.
  Thread::ThreadSynchronizer& synchronizer() { return synchronizer_; }

private:
  Thread::MutexBasicLockable ready_lock_;
  std::list<Event::PostCb> init_queue_ GUARDED_BY(ready_lock_);
  Event::Dispatcher* event_dispatcher_ GUARDED_BY(ready_lock_){};
  ApiListener* api_listener_ GUARDED_BY(ready_lock_){};
  // stats_ is not currently const because the Http::Dispatcher is constructed before there is
  // access to MainCommon's stats scope.
  // This can be fixed when queueing logic is moved out of the Http::Dispatcher, as at that point
  // the Http::Dispatcher could be constructed when access to all objects set in
  // Http::Dispatcher::ready() is done; obviously this would require "ready()" to not me a member
  // function of the dispatcher, but rather have a static factory method.
  // Related issue: https://github.com/lyft/envoy-mobile/issues/720
  const std::string stats_prefix_;
  absl::optional<DispatcherStats> stats_ GUARDED_BY(ready_lock_){};
  Thread::ThreadSynchronizer synchronizer_;
};

} // namespace Event
} // namespace Envoy
