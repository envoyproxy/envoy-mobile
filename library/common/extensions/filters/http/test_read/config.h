#pragma once

#include <string>

#include "source/extensions/filters/http/common/factory_base.h"

#include "library/common/extensions/filters/http/test_read/filter.pb.h"
#include "library/common/extensions/filters/http/test_read/filter.pb.validate.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace TestRead {

/**
 * Config registration for the TestRead filter. @see NamedHttpFilterConfigFactory.
 */
class TestReadFilterFactory
    : public Common::FactoryBase<envoymobile::extensions::filters::http::test_read::TestRead> {
public:
  TestReadFilterFactory() : FactoryBase("test_read") {}

private:
  ::Envoy::Http::FilterFactoryCb createFilterFactoryFromProtoTyped(
      const envoymobile::extensions::filters::http::test_read::TestRead& config,
      const std::string& stats_prefix, Server::Configuration::FactoryContext& context) override;
};

DECLARE_FACTORY(TestReadFilterFactory);

} // namespace TestRead
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
