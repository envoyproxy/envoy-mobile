#include "library/common/types/c_types.h"

#include <string.h>

void envoy_noop_release(void* context) { (void)context; }

void release_envoy_headers(envoy_headers headers) {
  for (envoy_header_size_t i = 0; i < headers.length; i++) {
    envoy_header header = headers.headers[i];
    header.key.release(header.key.context);
    header.value.release(header.value.context);
  }
  free(headers.headers);
}

envoy_headers copy_envoy_headers(envoy_headers src) {
  envoy_header* dst_header_array = (envoy_header*)malloc(sizeof(envoy_header) * src.length);
  if (src.length > 0 && dst_header_array == NULL) {
    return (envoy_headers){src.length, NULL};
  }
  for (envoy_header_size_t i = 0; i < src.length; i++) {
    envoy_data key = copy_envoy_data(src.headers[i].key.length, src.headers[i].key.bytes);
    if (key.length > 0 && key.bytes == NULL) {
      envoy_headers partial_headers = {i - 1, dst_header_array};
      release_envoy_headers(partial_headers);
      return (envoy_headers){src.length, NULL};
    }
    envoy_data value = copy_envoy_data(src.headers[i].value.length, src.headers[i].value.bytes);
    if (value.length > 0 && value.bytes == NULL) {
      key.release(key.context);
      envoy_headers partial_headers = {i - 1, dst_header_array};
      release_envoy_headers(partial_headers);
      return (envoy_headers){src.length, NULL};
    }
    envoy_header new_header = {key, value};
    dst_header_array[i] = new_header;
  }
  envoy_headers dst = {src.length, dst_header_array};
  return dst;
}

envoy_data copy_envoy_data(size_t length, const uint8_t* src_bytes) {
  uint8_t* dst_bytes = (uint8_t*)malloc(sizeof(uint8_t) * length);
  if (length > 0 && dst_bytes == NULL) {
    return (envoy_data){length, NULL, envoy_noop_release, NULL};
  }
  memcpy(dst_bytes, src_bytes, length);
  // Note: since this function is copying the bytes over to freshly allocated memory, free is an
  // appropriate release function and dst_bytes is an appropriate context.
  envoy_data dst = {length, dst_bytes, free, dst_bytes};
  return dst;
}

const envoy_data envoy_nodata = {0, NULL, envoy_noop_release, NULL};
