#pragma once

#include "envoy/upstream/retry.h"

#include "library/common/extensions/retry/options/interface_binding/predicate.pb.h"
#include "library/common/extensions/retry/options/interface_binding/predicate.pb.validate.h"
#include "library/common/network/configurator.h"

namespace Envoy {
namespace Extensions {
namespace Retry {
namespace Options {

class InterfaceBindingRetryOptionsPredicate : public Upstream::RetryOptionsPredicate {
public:
  explicit InterfaceBindingRetryOptionsPredicate(
      const envoymobile::extensions::retry::options::interface_binding::
          InterfaceBindingOptionsPredicate&,
      Upstream::RetryExtensionFactoryContext& context);

  UpdateOptionsReturn updateOptions(const UpdateOptionsParameters&) const { return {}; }

private:
  Network::ConfiguratorSharedPtr network_configurator_;
};

} // namespace Options
} // namespace Retry
} // namespace Extensions
} // namespace Envoy
