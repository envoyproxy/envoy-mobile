#include "library/common/main_interface.h"

#include <atomic>
#include <string>

#include "library/common/api/external.h"
#include "library/common/engine.h"
#include "library/common/extensions/filters/http/platform_bridge/c_types.h"
#include "library/common/http/client.h"

// NOLINT(namespace-envoy)

static std::atomic<envoy_stream_t> current_stream_handle_{0};
static std::atomic<envoy_network_t> preferred_network_{ENVOY_NET_GENERIC};

static Envoy::Engine* engine(envoy_engine_callbacks callbacks = {}) {
  static Envoy::Engine* engine = new Envoy::Engine{callbacks, preferred_network_};
  return engine;
}

envoy_stream_t init_stream(envoy_engine_t) { return current_stream_handle_++; }

envoy_status_t start_stream(envoy_stream_t stream, envoy_http_callbacks callbacks) {
  if (auto e = engine()) {
    return e->dispatcher().post(
        [e, stream, callbacks]() -> void { e->httpClient().startStream(stream, callbacks); });
  }
  return ENVOY_FAILURE;
}

envoy_status_t send_headers(envoy_stream_t stream, envoy_headers headers, bool end_stream) {
  if (auto e = engine()) {
    return e->dispatcher().post([e, stream, headers, end_stream]() -> void {
      e->httpClient().sendHeaders(stream, headers, end_stream);
    });
  }
  return ENVOY_FAILURE;
}

envoy_status_t send_data(envoy_stream_t stream, envoy_data data, bool end_stream) {
  if (auto e = engine()) {
    return e->dispatcher().post([e, stream, data, end_stream]() -> void {
      e->httpClient().sendData(stream, data, end_stream);
    });
  }
  return ENVOY_FAILURE;
}

// TODO: implement.
envoy_status_t send_metadata(envoy_stream_t, envoy_headers) { return ENVOY_FAILURE; }

envoy_status_t send_trailers(envoy_stream_t stream, envoy_headers trailers) {
  if (auto e = engine()) {
    return e->dispatcher().post(
        [e, stream, trailers]() -> void { e->httpClient().sendTrailers(stream, trailers); });
  }
  return ENVOY_FAILURE;
}

envoy_status_t reset_stream(envoy_stream_t stream) {
  if (auto e = engine()) {
    return e->dispatcher().post([e, stream]() -> void { e->httpClient().cancelStream(stream); });
  }
  return ENVOY_FAILURE;
}

envoy_status_t set_preferred_network(envoy_network_t network) {
  preferred_network_.store(network);
  return ENVOY_SUCCESS;
}

envoy_status_t record_counter_inc(envoy_engine_t, const char* elements, uint64_t count) {
  // TODO: use specific engine once multiple engine support is in place.
  // https://github.com/lyft/envoy-mobile/issues/332
  if (auto e = engine()) {
    return e->dispatcher().post(
        [e, elements, count]() -> void { e->recordCounterInc(std::string(elements), count); });
  }
  return ENVOY_FAILURE;
}

envoy_status_t record_gauge_set(envoy_engine_t, const char* elements, uint64_t value) {
  // TODO: use specific engine once multiple engine support is in place.
  // https://github.com/lyft/envoy-mobile/issues/332
  if (auto e = engine()) {
    return e->dispatcher().post(
        [e, elements, value]() -> void { e->recordGaugeSet(std::string(elements), value); });
  }
  return ENVOY_FAILURE;
}

envoy_status_t record_gauge_add(envoy_engine_t, const char* elements, uint64_t amount) {
  // TODO: use specific engine once multiple engine support is in place.
  // https://github.com/lyft/envoy-mobile/issues/332
  if (auto e = engine()) {
    return e->dispatcher().post(
        [e, elements, amount]() -> void { e->recordGaugeAdd(std::string(elements), amount); });
  }
  return ENVOY_FAILURE;
}

envoy_status_t record_gauge_sub(envoy_engine_t, const char* elements, uint64_t amount) {
  // TODO: use specific engine once multiple engine support is in place.
  // https://github.com/lyft/envoy-mobile/issues/332
  if (auto e = engine()) {
    return e->dispatcher().post(
        [e, elements, amount]() -> void { e->recordGaugeSub(std::string(elements), amount); });
  }
  return ENVOY_FAILURE;
}

envoy_status_t record_histogram_value(envoy_engine_t, const char* elements, uint64_t value,
                                      envoy_histogram_stat_unit_t unit_measure) {
  // TODO: use specific engine once multiple engine support is in place.
  // https://github.com/lyft/envoy-mobile/issues/332
  if (auto e = engine()) {
    return e->dispatcher().post([e, elements, value, unit_measure]() -> void {
      e->recordHistogramValue(std::string(elements), value, unit_measure);
    });
  }
  return ENVOY_FAILURE;
}

envoy_status_t register_platform_api(const char* name, void* api) {
  Envoy::Api::External::registerApi(std::string(name), api);
  return ENVOY_SUCCESS;
}

envoy_engine_t init_engine(envoy_engine_callbacks callbacks) {
  // TODO(goaway): return new handle once multiple engine support is in place.
  // https://github.com/lyft/envoy-mobile/issues/332
  envoy_engine_t id = 1;
  engine(callbacks);
  return id;
}

envoy_status_t run_engine(envoy_engine_t, const char* config, const char* log_level) {
  // This will change once multiple engine support is in place.
  // https://github.com/lyft/envoy-mobile/issues/332

  if (auto e = engine()) {
    e->run(config, log_level);
    return ENVOY_SUCCESS;
  }

  return ENVOY_FAILURE;
}

void terminate_engine(envoy_engine_t) {}
