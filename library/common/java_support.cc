#include <jni.h>

// NOLINT(namespace-envoy)

jint platform_setup(JavaVM*, jobject) { return 0; }

void jni_log(const char* tag, const char* text) {
  // FIXME: https://github.com/lyft/envoy-mobile/issues/120
}
