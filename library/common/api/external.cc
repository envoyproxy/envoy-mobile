#include "external.h"

#include <unordered_map>

#include "common/common/assert.h"

namespace Envoy {
namespace Api {
namespace External {

static std::unordered_map<std::string, void*> registry_{};

void registerApi(std::string name, void* api) {
  registry_[name] = api;
}

void* retrieveApi(std::string name) {
  void* api = registry_[name];
  ASSERT(api);
  return api;
}

} // namespace External
} // namespace Api
} // namespace Envoy
