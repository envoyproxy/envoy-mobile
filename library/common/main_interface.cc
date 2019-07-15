#include "library/common/main_interface.h"

#include <unordered_map>

#include "common/upstream/logical_dns_cluster.h"

#include "exe/main_common.h"

#include "extensions/filters/http/router/config.h"
#include "extensions/filters/network/http_connection_manager/config.h"
#include "extensions/transport_sockets/raw_buffer/config.h"
#include "extensions/transport_sockets/tls/config.h"

#include "library/common/http/async_stream_manager.h"
#include "library/common/http/header_utility.h"

// NOLINT(namespace-envoy)

static std::unique_ptr<Envoy::MainCommon> main_common_;
static std::unique_ptr<Envoy::Http::MobileAsyncStreamManager> stream_manager_;

envoy_stream start_stream(envoy_observer observer) {
  return {ENVOY_SUCCESS, stream_manager_->createStream(observer)};
}

// Note I did not deal with HeaderMap or Buffer (below) ownership yet. Meaning this functions still
// retain ownership of those objects. We should probably transfer ownership of them to the
// AsyncStream.

// Note as well that this functions did not take ownership of the passed in envoy_headers or
// envoy_data. The caller still has ownership of them.
envoy_status_t send_headers(envoy_stream_t stream_id, envoy_headers headers, bool end_stream) {
  auto stream = stream_manager_->getStream(stream_id);
  if (stream) {
    const auto header_map = Envoy::Http::Utility::transformHeaders(headers);
    stream->sendHeaders(*header_map, end_stream);
    return ENVOY_SUCCESS;
  } else {
    return ENVOY_FAILURE;
  }
}

envoy_status_t send_data(envoy_stream_t stream_id, envoy_data data, bool end_stream) {
  auto stream = stream_manager_->getStream(stream_id);
  if (stream) {
    const auto buffer = Envoy::Http::Utility::transformData(data);
    stream->sendData(*buffer, end_stream);
    return ENVOY_SUCCESS;
  } else {
    return ENVOY_FAILURE;
  }
}

// envoy_status_t send_metadata(envoy_stream_t stream_id, envoy_headers metadata, bool end_stream) {
//   auto stream = stream_manager_->getStream(stream_id);
//   if (stream) {
//     const auto metadata_map = Envoy::Http::Utility::transformHeaders(metadata);
//     stream->sendMetadata(*metadata_map, end_stream);
//     return ENVOY_SUCCESS;
//   } else {
//     return ENVOY_FAILURE;
//   }
// }

envoy_status_t send_trailers(envoy_stream_t stream_id, envoy_headers trailers) {
  auto stream = stream_manager_->getStream(stream_id);
  if (stream) {
    const auto trailers_map = Envoy::Http::Utility::transformHeaders(trailers);
    stream->sendTrailers(*trailers_map);
    return ENVOY_SUCCESS;
  } else {
    return ENVOY_FAILURE;
  }
}

// envoy_status_t locally_close_stream(envoy_stream_t stream_id) {
//   auto stream = stream_manager_->getStream(stream_id);
//   if (stream) {
//     stream->locally_close();
//     return ENVOY_SUCCESS;
//   } else {
//     return ENVOY_FAILURE;
//   }
// }

envoy_status_t reset_stream(envoy_stream_t stream_id) {
  auto stream = stream_manager_->getStream(stream_id);
  if (stream) {
    stream->reset();
    return ENVOY_SUCCESS;
  } else {
    return ENVOY_FAILURE;
  }
}

void setup() {
  Envoy::Http::AsyncClient& api_async_client =
      main_common_->server()->clusterManager().httpAsyncClientForCluster("hello_world_api");
  stream_manager_ = std::make_unique<Envoy::Http::MobileAsyncStreamManager>(api_async_client);
}

/**
 * External entrypoint for library.
 */
envoy_status_t run_engine(const char* config, const char* log_level) {
  char* envoy_argv[] = {strdup("envoy"), strdup("--config-yaml"), strdup(config),
                        strdup("-l"),    strdup(log_level),       nullptr};

  // Ensure static factory registration occurs on time.
  // Envoy's static factory registration happens when main is run.
  // However, when compiled as a library, there is no guarantee that such registration will happen
  // before the names are needed.
  // The following calls ensure that registration happens before the entities are needed.
  // Note that as more registrations are needed, explicit initialization calls will need to be added
  // here.
  Envoy::Extensions::HttpFilters::RouterFilter::forceRegisterRouterFilterConfig();
  Envoy::Extensions::NetworkFilters::HttpConnectionManager::
      forceRegisterHttpConnectionManagerFilterConfigFactory();
  Envoy::Extensions::TransportSockets::RawBuffer::forceRegisterDownstreamRawBufferSocketFactory();
  Envoy::Extensions::TransportSockets::RawBuffer::forceRegisterUpstreamRawBufferSocketFactory();
  Envoy::Extensions::TransportSockets::Tls::forceRegisterUpstreamSslSocketFactory();
  Envoy::Upstream::forceRegisterLogicalDnsClusterFactory();

  // Initialize the server's main context under a try/catch loop and simply
  // return EXIT_FAILURE as needed. Whatever code in the initialization path
  // that fails is expected to log an error message so the user can diagnose.
  // Note that in the Android examples logging will not be seen.
  // This is a known problem, and will be addressed by:
  // https://github.com/lyft/envoy-mobile/issues/34
  try {
    main_common_ = std::make_unique<Envoy::MainCommon>(5, envoy_argv);
  } catch (const Envoy::NoServingException& e) {
    return ENVOY_SUCCESS;
  } catch (const Envoy::MalformedArgvException& e) {
    std::cerr << e.what() << std::endl;
    return ENVOY_FAILURE;
  } catch (const Envoy::EnvoyException& e) {
    std::cerr << e.what() << std::endl;
    return ENVOY_FAILURE;
  }

  // Run the server listener loop outside try/catch blocks, so that unexpected
  // exceptions show up as a core-dumps for easier diagnostics.
  return main_common_->run() ? ENVOY_SUCCESS : ENVOY_FAILURE;
}
