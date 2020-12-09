#pragma once

#include <string>
#include <unordered_map>
#include <vector>

using RawHeaders = std::unordered_map<std::string, std::vector<std::string>>;

class Headers {
public:
  virtual ~Headers() {}

  const std::vector<std::string>& operator[](const std::string& key) const;
  const RawHeaders& all_headers() const;

protected:
  Headers(const RawHeaders& headers);

private:
  RawHeaders headers_;
};
