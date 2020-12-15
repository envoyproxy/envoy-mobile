#pragma once

// NOLINT(namespace-envoy)

#include <string>

#include "common/common/base_logger.h"

using LogLevel = Envoy::Logger::Logger::Levels;

std::string log_level_to_string(LogLevel method);
LogLevel log_level_from_string(const std::string& str);
