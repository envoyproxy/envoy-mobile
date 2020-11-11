#include "library/common/extensions/stat_sinks/metrics_service/config.h"

#include "common/grpc/async_client_impl.h"

#include "extensions/stat_sinks/metrics_service/grpc_metrics_service_impl.h"

#include "library/common/extensions/stat_sinks/metrics_service/metrics_service_config.pb.h"
#include "library/common/extensions/stat_sinks/metrics_service/metrics_service_config.pb.validate.h"
#include "library/common/extensions/stat_sinks/metrics_service/metrics_service_with_response.h"

namespace Envoy {
namespace Extensions {
namespace StatSinks {
namespace EnvoyMobileMetricsService {

Stats::SinkPtr EnvoyMobileMetricsServiceSinkFactory::createStatsSink(
    const Protobuf::Message& config, Server::Configuration::ServerFactoryContext& server) {
  const auto& sink_config =
      MessageUtil::downcastAndValidate<const envoymobile::extensions::config::StatSinks::
                                           MetricsService::EnvoyMobileMetricsServiceConfig&>(
          config, server.messageValidationContext().staticValidationVisitor());
  const auto& grpc_service = sink_config.grpc_service();
  ENVOY_LOG(debug, "Metrics Service gRPC service configuration: {}", grpc_service.DebugString());

  std::shared_ptr<MetricsService::GrpcMetricsStreamer<
      envoymobile::extensions::StatSinks::MetricsService::EnvoyMobileStreamMetricsMessage,
          envoymobile::extensions::StatSinks::MetricsService::EnvoyMobileStreamMetricsResponse>>
      grpc_metrics_streamer = std::make_shared<GrpcMetricsEnvoyMobileStreamerImpl>(
          server.clusterManager().grpcAsyncClientManager().factoryForGrpcService(
              grpc_service, server.scope(), false),
          server.localInfo(), server.api().randomGenerator());

  return std::make_unique<MetricsService::MetricsServiceSink<
      envoymobile::extensions::StatSinks::MetricsService::EnvoyMobileStreamMetricsMessage,
          envoymobile::extensions::StatSinks::MetricsService::EnvoyMobileStreamMetricsResponse>>(
      grpc_metrics_streamer,
      PROTOBUF_GET_WRAPPED_OR_DEFAULT(sink_config, report_counters_as_deltas, false));
}

ProtobufTypes::MessagePtr EnvoyMobileMetricsServiceSinkFactory::createEmptyConfigProto() {
  return std::unique_ptr<
      envoymobile::extensions::config::StatSinks::MetricsService::EnvoyMobileMetricsServiceConfig>(
      std::make_unique<envoymobile::extensions::config::StatSinks::MetricsService::
                           EnvoyMobileMetricsServiceConfig>());
}

std::string EnvoyMobileMetricsServiceSinkFactory::name() const {
  return "envoymobile.stat_sinks.metrics_service";
}

/**
 * Static registration for the this sink factory. @see RegisterFactory.
 */
REGISTER_FACTORY(EnvoyMobileMetricsServiceSinkFactory, Server::Configuration::StatsSinkFactory);

} // namespace EnvoyMobileMetricsService
} // namespace StatSinks
} // namespace Extensions
} // namespace Envoy
