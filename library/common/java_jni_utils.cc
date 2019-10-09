#include <jni.h>

// NOLINT(namespace-envoy)

jint init_jvm(JavaVM* static_jvm, jobject connectivity_manager) {
  return 0;
}

jboolean ares_initialized() {
  return true;
}

void jni_log(const char *tag, const char *text) {

}