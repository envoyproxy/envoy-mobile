#include <jni.h>

// NOLINT(namespace-envoy)

jint init_jvm(JavaVM* static_jvm, jobject connectivity_manager);

jint ares_initialized();

void jni_log(const char* tag, const char* text);