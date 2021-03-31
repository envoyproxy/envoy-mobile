#include <android/log.h>
#include <jni.h>

// NOLINT(namespace-envoy)

void jni_log(const char* tag, const char* text) {
  // FIXME: https://github.com/lyft/envoy-mobile/issues/120
  // For now, this can be used for debugging purposes on Android
  //  __android_log_write(ANDROID_LOG_ERROR, tag, text);
}
