#pragma once

#include "headers.h"

namespace Envoy {
namespace Platform {

class HeadersBuilder {
public:
  virtual ~HeadersBuilder() {}

  HeadersBuilder& add(const std::string& name, const std::string& value);
  HeadersBuilder& set(const std::string& name, const std::vector<std::string>& values);
  HeadersBuilder& remove(const std::string& name);

protected:
  HeadersBuilder();
  HeadersBuilder& internal_set(const std::string& name, const std::vector<std::string>& values);
  const RawHeaders& all_headers() const;

private:
  bool is_restricted_header_(const std::string& name) const;

  RawHeaders headers_;
};

} // namespace Platform
} // namespace Envoy
