#include "headers_builder.h"

namespace Envoy {
namespace Platform {

HeadersBuilder& HeadersBuilder::add(absl::string_view name, absl::string_view value) {
  if (this->isRestrictedHeader(name)) {
    return *this;
  }
  this->headers_[name].push_back(std::string(value));
  return *this;
}

HeadersBuilder& HeadersBuilder::set(absl::string_view name,
                                    const std::vector<std::string>& values) {
  if (this->isRestrictedHeader(name)) {
    return *this;
  }
  this->headers_[name] = values;
  return *this;
}

HeadersBuilder& HeadersBuilder::remove(absl::string_view name) {
  if (this->isRestrictedHeader(name)) {
    return *this;
  }
  this->headers_.erase(name);
  return *this;
}

HeadersBuilder::HeadersBuilder() {}

HeadersBuilder& HeadersBuilder::internalSet(absl::string_view name,
                                            const std::vector<std::string>& values) {
  this->headers_[name] = values;
  return *this;
}

const RawHeaderMap& HeadersBuilder::allHeaders() const { return this->headers_; }

bool HeadersBuilder::isRestrictedHeader(absl::string_view name) const {
  return name.find(":") == 0 || name.find("x-envoy-mobile") == 0;
}

} // namespace Platform
} // namespace Envoy
