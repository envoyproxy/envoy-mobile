#include "headers.h"

// NOLINT(namespace-envoy)

const std::vector<std::string>& Headers::operator[](const std::string& key) const {
  return this->headers_.at(key);
}

const RawHeaders& Headers::all_headers() const { return this->headers_; }

bool Headers::contains(const std::string& key) const { return this->headers_.contains(key); }

Headers::Headers(const RawHeaders& headers) : headers_(headers) {}
