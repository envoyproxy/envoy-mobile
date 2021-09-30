#pragma once

#include "envoy/upstream/retry.h"

#include "library/common/extensions/retry/options/interface_binding/predicate.pb.h"
#include "library/common/extensions/retry/options/interface_binding/predicate.pb.validate.h"
#include "library/common/network/configurator.h"

namespace Envoy {
namespace Extensions {
namespace Retry {
namespace Options {

InterfaceBindingRetryOptionsPredicate::InterfaceBindingRetryOptionsPredicate(
    const envoymobile::extensions::retry::options::interface_binding::
        InterfaceBindingOptionsPredicate&,
    Upstream::RetryExtensionFactoryContext& context) {
  network_configurator_ = context.singletonManager().getTyped<Configurator>(
      SINGLETON_MANAGER_REGISTERED_NAME(network_configurator));
  RELEASE_ASSERT(network_configurator_ != nullptr, "unexpected nullptr network configurator");
}

UpdateOptionsReturn
InterfaceBindingRetryOptionsPredicate::updateOptions(const UpdateOptionsParameters&) const {
  return {network_configurator_->getUpstreamSocketOptions(
      network_configurator_->getPreferredNetwork(), false /* not being used? */)};
}

} // namespace Options
} // namespace Retry
} // namespace Extensions
} // namespace Envoy
