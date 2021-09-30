#pragma once

#include "envoy/upstream/retry.h"

#include "library/common/extensions/retry/options/interface_binding/predicate.pb.h"
#include "library/common/extensions/retry/options/interface_binding/predicate.pb.validate.h"

namespace Envoy {
namespace Extensions {
namespace Retry {
namespace Options {

class InterfaceBindingRetryOptionsPredicate : public Upstream::RetryOptionsPredicate {
public:
  explicit InterfaceBindingRetryOptionsPredicate(
      const envoymobile::extensions::retry::options::interface_binding::
          InterfaceBindingOptionsPredicate&,
      Upstream::RetryExtensionFactoryContext&) {}

  UpdateOptionsReturn updateOptions(const UpdateOptionsParameters&) const { return {}; }
};

} // namespace Options
} // namespace Retry
} // namespace Extensions
} // namespace Envoy
