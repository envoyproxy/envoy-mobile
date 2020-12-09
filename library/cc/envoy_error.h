#pragma once

#include <exception>
#include <optional>
#include <string>

struct EnvoyError {
  int error_code;
  std::string message;
  std::optional<int> attempt_count;
  std::exception cause;
};
