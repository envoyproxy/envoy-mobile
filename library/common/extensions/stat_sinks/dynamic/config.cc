#include "extensions/stat_sinks/dynamic/config.h"

#include "extensions/stat_sinks/dynamic/sink.h"

namespace Envoy {
namespace Extensions {
namespace StatSinks {
namespace Loop {

Stats::SinkPtr Factory::createStatsSink(const Protobuf::Message&,
                                        Server::Configuration::ServerFactoryContext&) {
  return std::make_unique<Sink>();
}

ProtobufTypes::MessagePtr Factory::createEmptyConfigProto() {
  return std::unique_ptr<extensions::stat_sinks::loop::loopStatsSinkConfig>(
      std::make_unique<extensions::stat_sinks::loop::loopStatsSinkConfig>());
}

std::string Factory::name() const { return "envoy.stat_sinks.loop"; }

/**
 * Static registration for the this sink factory. @see RegisterFactory.
 */
REGISTER_FACTORY(Factory, Server::Configuration::StatsSinkFactory);

} // namespace Loop
} // namespace StatSinks
} // namespace Extensions
} // namespace Envoy
