#pragma once

#include "envoy/http/filter.h"

#include "source/extensions/filters/http/common/pass_through_filter.h"

#include "library/common/api/c_types.h"
#include "library/common/api/external.h"
#include "library/common/extensions/filters/http/test_kv_store/filter.pb.h"
#include "library/common/extensions/key_value/platform/c_types.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace TestKeyValueStore {

class TestKeyValueStoreFilterConfig {
public:
  TestKeyValueStoreFilterConfig(
      const envoymobile::extensions::filters::http::test_kv_store::TestKeyValueStore& proto_config);

  const envoy_kv_store* keyValueStore() const { return kv_store_; }
  const std::string& testKey() const { return test_key_; }
  const std::string& testValue() const { return test_value_; }

private:
  const envoy_kv_store* kv_store_;
  const std::string test_key_;
  const std::string test_value_;
};

using TestKeyValueStoreFilterConfigSharedPtr = std::shared_ptr<TestKeyValueStoreFilterConfig>;

class TestKeyValueStoreFilter final : public ::Envoy::Http::PassThroughFilter {
public:
  TestKeyValueStoreFilter(TestKeyValueStoreFilterConfigSharedPtr config) : config_(config) {}

  // StreamDecoderFilter
  ::Envoy::Http::FilterHeadersStatus decodeHeaders(::Envoy::Http::RequestHeaderMap& headers,
                                                   bool end_stream) override;
  // StreamEncoderFilter
  ::Envoy::Http::FilterHeadersStatus encodeHeaders(::Envoy::Http::ResponseHeaderMap& headers,
                                                   bool end_stream) override;

private:
  const TestKeyValueStoreFilterConfigSharedPtr config_;
};

} // namespace TestKeyValueStore
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
