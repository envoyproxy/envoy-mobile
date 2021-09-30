#include "library/common/extensions/retry/options/interface_binding/config.h"

#include "envoy/registry/registry.h"

namespace Envoy {
namespace Extensions {
namespace Retry {
namespace Options {

REGISTER_FACTORY(NetworkConfigurationRetryOptionsPredicateFactory,
                 Upstream::RetryOptionsPredicateFactory);

}
} // namespace Retry
} // namespace Extensions
} // namespace Envoy
