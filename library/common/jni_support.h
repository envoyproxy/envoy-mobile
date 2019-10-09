#include <jni.h>

// NOLINT(namespace-envoy)

// We initialize c-ares on Android and we no-op for Java
jint platform_setup(JavaVM* jvm, jobject connectivity_manager);

void jni_log(const char* tag, const char* text);
