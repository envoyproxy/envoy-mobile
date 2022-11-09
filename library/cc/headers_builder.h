#pragma once

#include "headers.h"

namespace Envoy {
namespace Platform {

class HeadersBuilder {
public:
  virtual ~HeadersBuilder() {}

  HeadersBuilder& add(std::string name, std::string value);
  HeadersBuilder& set(std::string name, const std::vector<std::string>& values);
  HeadersBuilder& remove(std::string name);

protected:
  HeadersBuilder();
  HeadersBuilder& internalSet(std::string name, const std::vector<std::string>& values);
  const RawHeaderMap& allHeaders() const;

private:
  bool isRestrictedHeader(std::string name) const;

  RawHeaderMap headers_;
};

} // namespace Platform
} // namespace Envoy
