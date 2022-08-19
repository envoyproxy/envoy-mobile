#pragma once

#include "library/common/types/c_types.h"

// NOLINT(namespace-envoy)

typedef struct {
  envoy_status_t result;
  uint8_t tls_alert;
  const char* error_details;
} envoy_cert_validation_result;

#ifdef __cplusplus
extern "C" { // function pointers
#endif

typedef envoy_cert_validation_result (*envoy_validate_cert_f)(const envoy_data* certs, uint8_t size,
                                                              const char* host_name);

#ifdef __cplusplus
} // function pointers
#endif

typedef struct {
  envoy_validate_cert_f validate_cert;
} envoy_cert_validator;
