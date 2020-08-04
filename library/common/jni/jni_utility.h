#include <jni.h>

// NOLINT(namespace-envoy)

// Platform wide initialization.
jint platform_setup(JavaVM* jvm, jobject connectivity_manager);

// Logging from JNI functions.
void jni_log(const char* tag, const char* text);

// Get JNIEnv.
JNIEnv* get_env();
