#include <string>

#include "library/common/extensions/filters/http/platform_extension/filter.pb.h"
#include "library/common/extensions/filters/http/platform_extension/filter.pb.validate.h"

#include "extensions/filters/http/common/factory_base.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace PlatformExtension {

/**
 * Config registration for the decompressor filter. @see NamedHttpFilterConfigFactory.
 */
class BridgingFilterFactory
    : public Common::FactoryBase<envoymobile::extensions::filters::http::platform_extension::Bridging> {
public:
  BridgingFilterFactory() : FactoryBase("platform_extension") {}

private:
  Http::FilterFactoryCb createFilterFactoryFromProtoTyped(
      const envoymobile::extensions::filters::http::platform_extension::Bridging& config,
      const std::string& stats_prefix, Server::Configuration::FactoryContext& context) override;
};

DECLARE_FACTORY(BridgingFilterFactory);

} // namespace PlatformExtension
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
