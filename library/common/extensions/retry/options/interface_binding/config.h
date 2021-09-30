#pragma once

#include <string>

#include "envoy/upstream/retry.h"

#include "source/common/protobuf/message_validator_impl.h"

#include "library/common/extensions/retry/options/interface_binding/predicate.h"
#include "library/common/extensions/retry/options/interface_binding/predicate.pb.h"
#include "library/common/extensions/retry/options/interface_binding/predicate.pb.validate.h"

namespace Envoy {
namespace Extensions {
namespace Retry {
namespace Options {

class NetworkConfigurationRetryOptionsPredicateFactory : public Upstream::RetryOptionsPredicateFactory {
public:
  Upstream::RetryOptionsPredicateConstSharedPtr
  createOptionsPredicate(const Protobuf::Message& config,
                         Upstream::RetryExtensionFactoryContext& context) override {
    return std::make_shared<NetworkConfigurationRetryOptionsPredicate>(
        MessageUtil::downcastAndValidate<const envoymobile::extensions::retry::options::
                                             interface_binding::NetworkConfigurationOptionsPredicate&>(
            config, ProtobufMessage::getStrictValidationVisitor()),
        context);
  }

  std::string name() const override { return "envoy.retry_options_predicates.interface_binding"; }

  ProtobufTypes::MessagePtr createEmptyConfigProto() override {
    return std::make_unique<envoymobile::extensions::retry::options::interface_binding::
                                NetworkConfigurationOptionsPredicate>();
  }
};

DECLARE_FACTORY(NetworkConfigurationRetryOptionsPredicateFactory);

} // namespace Options
} // namespace Retry
} // namespace Extensions
} // namespace Envoy
