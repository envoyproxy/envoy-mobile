#include <string>

#include "library/extensions/filters/http/harness/filter.pb.h"
#include "library/extensions/filters/http/harness/filter.pb.validate.h"

#include "extensions/filters/http/common/factory_base.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Harness {

/**
 * Config registration for the decompressor filter. @see NamedHttpFilterConfigFactory.
 */
class HarnessFilterFactory
    : public Common::FactoryBase<envoymobile::extensions::filters::http::harness::Harness> {
public:
  HarnessFilterFactory() : FactoryBase("harness") {}

private:
  Http::FilterFactoryCb createFilterFactoryFromProtoTyped(
      const envoymobile::extensions::filters::http::harness::Harness& config,
      const std::string& stats_prefix, Server::Configuration::FactoryContext& context) override;
};

DECLARE_FACTORY(HarnessFilterFactory);

}
} // namespace Configuration
} // namespace Server
} // namespace Envoy
