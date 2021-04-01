#include <android/log.h>

// NOLINT(namespace-envoy)

int jni_log(const char *tag,  const char *fmt, ...) {
    return __android_log_print(ANDROID_LOG_VERBOSE, tag, fmt, ...);
}
