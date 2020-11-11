#pragma once

#include <memory>

#include "envoy/common/random_generator.h"
#include "envoy/grpc/async_client.h"
#include "envoy/local_info/local_info.h"
#include "envoy/network/connection.h"
#include "envoy/service/metrics/v3/metrics_service.pb.h"
#include "envoy/stats/sink.h"
#include "envoy/upstream/cluster_manager.h"

#include "library/common/extensions/stat_sinks/metrics_service/metrics_service.pb.h"

namespace Envoy {
namespace Extensions {
namespace StatSinks {
namespace EnvoyMobileMetricsService {

/**
 * EnvoyMobile implementation of GrpcMetrics\Streamer
 */
class GrpcMetricsEnvoyMobileStreamerImpl
    : public Singleton::Instance,
      public Logger::Loggable<Logger::Id::filter>,
      public MetricsService::GrpcMetricsStreamer<
          envoymobile::extensions::StatSinks::MetricsService::EnvoyMobileStreamMetricsMessage,
          envoymobile::extensions::StatSinks::MetricsService::EnvoyMobileStreamMetricsResponse> {
public:
  GrpcMetricsEnvoyMobileStreamerImpl(Grpc::AsyncClientFactoryPtr&& factory,
                                     const LocalInfo::LocalInfo& local_info,
                                     Random::RandomGenerator& random_generator);

  void send(MetricsService::MetricsPtr&& metrics) override;

  void onReceiveMessage(
      std::unique_ptr<
          envoymobile::extensions::StatSinks::MetricsService::EnvoyMobileStreamMetricsResponse>&&
          response) override {
    ENVOY_LOG(debug, "EnvoyMobile streamer received batch_id: {}", response->batch_id());
  }

  // Grpc::AsyncStreamCallbacks
  void onRemoteClose(Grpc::Status::GrpcStatus, const std::string&) override { stream_ = nullptr; }

private:
  const LocalInfo::LocalInfo& local_info_;
  Random::RandomGenerator& random_generator_;
  const Protobuf::MethodDescriptor& service_method_;
};

using GrpcMetricsEnvoyMobileStreamerImplPtr = std::unique_ptr<GrpcMetricsEnvoyMobileStreamerImpl>;

} // namespace EnvoyMobileMetricsService
} // namespace StatSinks
} // namespace Extensions
} // namespace Envoy
