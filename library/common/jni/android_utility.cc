#include <android/log.h>
#include <ares.h>
#include <jni.h>

#include "library/common/jni/jni_utility.h"

// NOLINT(namespace-envoy)

extern JavaVM* global_jvm;
extern const jint JNI_VERSION;

jint platform_setup(JavaVM* jvm, jobject connectivity_manager) {
  // c-ares jvm init is necessary in order to let c-ares perform DNS resolution in Envoy.
  // More information can be found at:
  // https://c-ares.haxx.se/ares_library_init_android.html
  ares_library_init_jvm(jvm);

  return ares_library_init_android(connectivity_manager);
}

void jni_log(const char* tag, const char* text) {
  // FIXME: https://github.com/lyft/envoy-mobile/issues/120
  // For now, this can be used for debugging purposes on Android.
  __android_log_write(ANDROID_LOG_ERROR, tag, text);
}

JNIEnv* get_env() {
  JNIEnv* env = nullptr;
  int get_env_res = global_jvm->GetEnv((void**)&env, JNI_VERSION);
  if (get_env_res == JNI_EDETACHED) {
    jni_log("[Envoy]", "environment is JNI_EDETACHED");
    // Note: the only thread that should need to be attached is Envoy's engine std::thread.
    // TODO: harden this piece of code to make sure that we are only needing to attach Envoy
    // engine's std::thread, and that we detach it successfully.
    global_jvm->AttachCurrentThread(&env, nullptr);
    global_jvm->GetEnv((void**)&env, JNI_VERSION);
  }
  return env;
}
