#pragma once

#include <string>

#include "absl/strings/string_view.h"
#include "source/common/common/base_logger.h"

namespace Envoy {
namespace Platform {

using LogLevel = Envoy::Logger::Logger::Levels;

std::string logLevelToString(LogLevel method);
LogLevel logLevelFromString(absl::string_view str);

} // namespace Platform
} // namespace Envoy
