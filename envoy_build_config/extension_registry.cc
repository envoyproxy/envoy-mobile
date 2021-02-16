#include "extension_registry.h"

#include "common/network/socket_interface_impl.h"
#include "common/upstream/logical_dns_cluster.h"

#include "extensions/clusters/dynamic_forward_proxy/cluster.h"
#include "extensions/compression/gzip/decompressor/config.h"
#include "extensions/filters/http/buffer/config.h"
#include "extensions/filters/http/decompressor/config.h"
#include "extensions/filters/http/dynamic_forward_proxy/config.h"
#include "extensions/filters/http/router/config.h"
#include "extensions/filters/network/http_connection_manager/config.h"
#include "extensions/stat_sinks/metrics_service/config.h"
#include "extensions/transport_sockets/raw_buffer/config.h"
#include "extensions/transport_sockets/tls/config.h"
#include "extensions/upstreams/http/generic/config.h"

#include "library/common/extensions/filters/http/assertion/config.h"
#include "library/common/extensions/filters/http/platform_bridge/config.h"

namespace Envoy {

void ExtensionRegistry::registerFactories() {
  Envoy::Extensions::Clusters::DynamicForwardProxy::forceRegisterClusterFactory();
  Envoy::Extensions::Compression::Gzip::Decompressor::forceRegisterGzipDecompressorLibraryFactory();
  Envoy::Extensions::HttpFilters::Assertion::forceRegisterAssertionFilterFactory();
  Envoy::Extensions::HttpFilters::Decompressor::forceRegisterDecompressorFilterFactory();
  Envoy::Extensions::HttpFilters::BufferFilter::forceRegisterBufferFilterFactory();
  Envoy::Extensions::HttpFilters::DynamicForwardProxy::
      forceRegisterDynamicForwardProxyFilterFactory();
  Envoy::Extensions::HttpFilters::LocalError::forceRegisterLocalErrorFilterFactory();
  Envoy::Extensions::HttpFilters::PlatformBridge::forceRegisterPlatformBridgeFilterFactory();
  Envoy::Extensions::HttpFilters::RouterFilter::forceRegisterRouterFilterConfig();
  Envoy::Extensions::NetworkFilters::HttpConnectionManager::
      forceRegisterHttpConnectionManagerFilterConfigFactory();
  Envoy::Extensions::StatSinks::MetricsService::forceRegisterMetricsServiceSinkFactory();
  Envoy::Extensions::TransportSockets::RawBuffer::forceRegisterUpstreamRawBufferSocketFactory();
  Envoy::Extensions::TransportSockets::Tls::forceRegisterUpstreamSslSocketFactory();
  Envoy::Extensions::Upstreams::Http::Generic::forceRegisterGenericGenericConnPoolFactory();
  Envoy::Upstream::forceRegisterLogicalDnsClusterFactory();

  // TODO: add a "force initialize" function to the upstream code, or clean up the upstream code
  // in such a way that does not depend on the statically initialized variable.
  // The current setup exposes in iOS the same problem as the one described in:
  // https://github.com/envoyproxy/envoy/pull/7185 with the static variable declared in:
  // https://github.com/envoyproxy/envoy/pull/11380/files#diff-8a5c90e5a39b2ea975170edc4434345bR138.
  // For now force the compilation unit to run by creating an instance of the class declared in
  // socket_interface_impl.h and immediately destroy.
  auto ptr = std::make_unique<Network::SocketInterfaceImpl>();
  ptr.reset(nullptr);
}

} // namespace Envoy
