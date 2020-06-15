#pragma once

#include <string>

namespace Envoy {
namespace Api
namespace External {

/**
 * Register an external runtime API for usage (most likely in extensions).
 * NOTE: This is a proof of concept implementation and a HACK. Registration is NOT thread-safe.
 */
void registerApi(std::string name, void* api);

/**
 * Retrieve an external runtime API for usage (most likely in extensions).
 */
void* retrieveApi(std::string name);

} // namespace External
} // namespace Api
} // namespace Envoy
