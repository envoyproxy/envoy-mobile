#pragma once

#include <functional>
#include <memory>

#include "envoy/common/pure.h"

namespace Envoy {
namespace Platform {

class Executor {
public:
  virtual ~Executor() {}

  virtual void execute(std::function<void()> closure) PURE;
};

using ExecutorSharedPtr = std::shared_ptr<Executor>;

} // namespace Platform
} // namespace Envoy
