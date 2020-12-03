#pragma once

#include <optional>
#include <string>


struct EnvoyError {
  int error_code;
  std::string message;
  std::optional<int> attempt_count;
  // TODO(crockeo): cause?
};
