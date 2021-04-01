#include <jni.h>

// NOLINT(namespace-envoy)

int jni_log(const char *tag,  const char *fmt, ...);

jint java_vm_attach_current_thread(JavaVM *vm, JNIEnv** p_env, void *thr_args);
