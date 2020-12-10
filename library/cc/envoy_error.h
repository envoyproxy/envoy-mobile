#pragma once

#include <exception>
#include <memory>
#include <optional>
#include <string>

struct EnvoyError {
  int error_code;
  std::string message;
  std::optional<int> attempt_count;
  std::exception cause;
};

using EnvoyErrorSharedPtr = std::shared_ptr<EnvoyError>;
