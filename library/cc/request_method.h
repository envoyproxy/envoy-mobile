#pragma once

#include <string>


/**
 * See log_level.h on info about how this weird enum thing works
 */
class RequestMethod {
public:
  enum _RequestMethod {
    DELETE,
    GET,
    HEAD,
    OPTIONS,
    PATCH,
    POST,
    PUT,
    TRACE,
  };

  static _RequestMethod from_string(std::string request_method);
  static std::string to_string(_RequestMethod request_method);

private:
  RequestMethod() {}
};
