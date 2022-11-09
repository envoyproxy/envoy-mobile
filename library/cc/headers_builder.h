#pragma once

#include "headers.h"

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Platform {

class HeadersBuilder {
public:
  virtual ~HeadersBuilder() {}

  HeadersBuilder& add(absl::string_view name, absl::string_view value);
  HeadersBuilder& set(absl::string_view name, const std::vector<std::string>& values);
  HeadersBuilder& remove(absl::string_view name);

protected:
  HeadersBuilder();
  HeadersBuilder& internalSet(absl::string_view name, const std::vector<std::string>& values);
  const RawHeaderMap& allHeaders() const;

private:
  bool isRestrictedHeader(absl::string_view name) const;

  RawHeaderMap headers_;
};

} // namespace Platform
} // namespace Envoy
