#include "library/common/jni/jni_utility.h"

// NOLINT(namespace-envoy)

// No-op implementations of jni_utility.h to use for local jar execution in testing targets.

jint platform_setup(JavaVM*, jobject) { return 0; }
void jni_log(const char* tag, const char* text) {
  // FIXME: https://github.com/lyft/envoy-mobile/issues/120
}
