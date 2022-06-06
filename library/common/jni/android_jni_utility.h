#pragma once

#include "library/common/types/c_types.h"

class JNIEnv;

// NOLINT(namespace-envoy)

/* For android, calls up through JNI to see if cleartext is permitted for this
 * host.
 * For other platforms simply returns true.
 */
bool is_cleartext_permitted(envoy_data host);
