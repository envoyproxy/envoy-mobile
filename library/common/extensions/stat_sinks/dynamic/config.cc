#include "library/common/extensions/stat_sinks/dynamic/config.h"

#include "library/common/extensions/stat_sinks/dynamic/sink.h"

namespace Envoy {
namespace Extensions {
namespace StatSinks {
namespace Dynamic {

Stats::SinkPtr Factory::createStatsSink(const Protobuf::Message& config,
                                        Server::Configuration::ServerFactoryContext& server) {
  const auto& sink_config = MessageUtil::downcastAndValidate<
      const extensions::stat_sinks::dynamic::DynamicStatsSinkConfig&>(
      config, server.messageValidationContext().staticValidationVisitor());
  return std::make_unique<Sink>(sink_config.library_name());
}

ProtobufTypes::MessagePtr Factory::createEmptyConfigProto() {
  return std::unique_ptr<extensions::stat_sinks::dynamic::DynamicStatsSinkConfig>(
      std::make_unique<extensions::stat_sinks::dynamic::DynamicStatsSinkConfig>());
}

std::string Factory::name() const { return "envoy.stat_sinks.dynamic"; }

/**
 * Static registration for the this sink factory. @see RegisterFactory.
 */
REGISTER_FACTORY(Factory, Server::Configuration::StatsSinkFactory);

} // namespace Dynamic
} // namespace StatSinks
} // namespace Extensions
} // namespace Envoy
