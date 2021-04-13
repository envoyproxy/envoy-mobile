#include <android/log.h>
#include <ares.h>

#include "library/common/jni/jni_support.h"
#include "library/common/main_interface.h"
#include "library/common/jni/jni_utility.h"

// NOLINT(namespace-envoy)

int jni_log_fmt(const char* tag, const char* fmt, void* value) {
  return __android_log_print(ANDROID_LOG_VERBOSE, tag, fmt, value);
}

int jni_log(const char* tag, const char* str) {
  return __android_log_write(ANDROID_LOG_VERBOSE, tag, str);
}

jint attach_jvm(JavaVM* vm, JNIEnv** p_env, void* thr_args) {
  return vm->AttachCurrentThread(p_env, thr_args);
}

int ares_init(jobject connectivity_manager) {
  // See note above about c-ares.
  // c-ares jvm init is necessary in order to let c-ares perform DNS resolution in Envoy.
  // More information can be found at:
  // https://c-ares.haxx.se/ares_library_init_android.html
  ares_library_init_jvm(get_vm());

  return ares_library_init_android(connectivity_manager);
}
