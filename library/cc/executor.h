#pragma once

#include <functional>
#include <memory>

namespace Envoy {
namespace Platform {

class Executor {
public:
  virtual ~Executor() {}

  virtual void execute(std::function<void()> closure) = 0;
};

using ExecutorSharedPtr = std::shared_ptr<Executor>;

} // namespace Platform
} // namespace Envoy
