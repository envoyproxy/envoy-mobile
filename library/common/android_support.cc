#include <android/log.h>
#include <ares.h>
#include <jni.h>

// NOLINT(namespace-envoy)

jint init_jvm(JavaVM* static_jvm, jobject connectivity_manager) {
  // See note above about c-ares.
  // c-ares jvm init is necessary in order to let c-ares perform DNS resolution in Envoy.
  // More information can be found at:
  // https://c-ares.haxx.se/ares_library_init_android.html
  ares_library_init_jvm(static_jvm);

  return ares_library_init_android(connectivity_manager);
}

void jni_log(const char* tag, const char* text) {
  __android_log_write(ANDROID_LOG_ERROR, tag, text);
}