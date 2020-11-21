#pragma once

#include "library/common/types/c_types.h"

// NOLINT(namespace-envoy)

#ifdef __cplusplus
extern "C" { // function pointers
#endif

typedef envoy_data (*envoy_get_string_f)(void* context);

#ifdef __cplusplus
} // function pointers
#endif

typedef struct {
  envoy_get_string_f get_string;
  void* context;
} envoy_string_accessor;
