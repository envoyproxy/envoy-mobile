#include <jni.h>

// NOLINT(namespace-envoy)

jint platform_setup(JavaVM* jvm, jobject connectivity_manager) { return 0; }

void jni_log(const char* tag, const char* text) {}
