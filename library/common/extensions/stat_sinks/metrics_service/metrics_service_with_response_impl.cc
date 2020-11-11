#include "extensions/stat_sinks/metrics_service/grpc_metrics_service_impl.h"

#include "metrics_service_with_response.h"

namespace Envoy {
namespace Extensions {
namespace StatSinks {
namespace EnvoyMobileMetricsService {

GrpcMetricsEnvoyMobileStreamerImpl::GrpcMetricsEnvoyMobileStreamerImpl(
    Grpc::AsyncClientFactoryPtr&& factory, const LocalInfo::LocalInfo& local_info,
    Random::RandomGenerator& random_generator)
    : GrpcMetricsStreamer<
          envoymobile::extensions::StatSinks::MetricsService::EnvoyMobileStreamMetricsMessage,
          envoymobile::extensions::StatSinks::MetricsService::EnvoyMobileStreamMetricsResponse>(
          std::move(factory)),
      local_info_(local_info), random_generator_(random_generator),
      service_method_(*Protobuf::DescriptorPool::generated_pool()->FindMethodByName(
          "envoymobile.extensions.StatSinks.MetricsService.EnvoyMobileMetricsService."
          "EnvoyMobileStreamMetrics")) {}

void GrpcMetricsEnvoyMobileStreamerImpl::send(MetricsService::MetricsPtr&& metrics) {  
  envoymobile::extensions::StatSinks::MetricsService::EnvoyMobileStreamMetricsMessage message;
  message.mutable_envoy_metrics()->Reserve(metrics->size());
  message.mutable_envoy_metrics()->MergeFrom(*metrics);
  std::string uuid = random_generator_.uuid();
  message.set_batch_id(uuid);
  if (stream_ == nullptr) {
    stream_ = client_->start(service_method_, *this, Http::AsyncClient::StreamOptions());
    // for perf reasons, the identifier is only sent on establishing the stream.
    auto* identifier = message.mutable_identifier();
    *identifier->mutable_node() = local_info_.node();
  }
  if (stream_ != nullptr) {
    stream_->sendMessage(message, false);
  }
}

} // namespace EnvoyMobileMetricsService
} // namespace StatSinks
} // namespace Extensions
} // namespace Envoy