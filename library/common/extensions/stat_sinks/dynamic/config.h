#pragma once

#include "source/server/configuration_impl.h"

namespace Envoy {
namespace Extensions {
namespace StatSinks {
namespace Dynamic {

/**
 * Config registration for the Sink stats sink. @see StatsSinkFactory.
 */
class Factory : Logger::Loggable<Logger::Id::config>,
                public Server::Configuration::StatsSinkFactory {
public:
  Stats::SinkPtr createStatsSink(const Protobuf::Message& config,
                                 Server::Configuration::ServerFactoryContext& server) override;

  ProtobufTypes::MessagePtr createEmptyConfigProto() override;

  std::string name() const override;
};

DECLARE_FACTORY(Factory);

} // namespace Dynamic
} // namespace StatSinks
} // namespace Extensions
} // namespace Envoy
