#include <android/log.h>
#include <ares.h>
#include <jni.h>

// NOLINT(namespace-envoy)

jint platform_setup(JavaVM* jvm, jobject connectivity_manager) {
  // c-ares jvm init is necessary in order to let c-ares perform DNS resolution in Envoy.
  // More information can be found at:
  // https://c-ares.haxx.se/ares_library_init_android.html
  ares_library_init_jvm(jvm);

  return ares_library_init_android(connectivity_manager);
}

void jni_log(const char* tag, const char* text) {
  // FIXME: https://github.com/lyft/envoy-mobile/issues/120
  // For now, this can be used for debugging purposes on Android
  //  __android_log_write(ANDROID_LOG_ERROR, tag, text);
}
