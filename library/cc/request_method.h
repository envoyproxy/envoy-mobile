#pragma once

// NOLINT(namespace-envoy)

#include <string>

enum RequestMethod {
  DELETE,
  GET,
  HEAD,
  OPTIONS,
  PATCH,
  POST,
  PUT,
  TRACE,
};

std::string request_method_to_string(RequestMethod method);
RequestMethod request_method_from_string(const std::string& str);
