#pragma once

#include <string>

class UpstreamHttpProtocol {
public:
  enum _UpstreamHttpProtocol {
    HTTP1,
    HTTP2,
  };

  static _UpstreamHttpProtocol from_string(std::string upstream_http_protocol);
  static std::string to_string(_UpstreamHttpProtocol upstream_http_protocol);

private:
  UpstreamHttpProtocol() {}
};
