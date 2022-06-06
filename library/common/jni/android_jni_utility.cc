#include "library/common/jni/android_jni_utility.h"

#include <stdlib.h>
#include <string.h>

#include "source/common/common/assert.h"

#if defined(__ANDROID_API__)
#include "library/common/jni/import/jni_import.h"
#include "library/common/jni/jni_support.h"
#include "library/common/jni/jni_version.h"
#endif

// NOLINT(namespace-envoy)

bool is_cleartext_permitted(envoy_data host) {
#if defined(__ANDROID_API__)
  JNIEnv* env = get_env();
  jstring java_host = native_data_to_string(env, host);
  jclass jcls_Boolean = env->FindClass("org/chromium/net/AndroidNetworkLibrary");
  jmethodID jmid_isCleartextTrafficPermitted =
      env->GetMethodID(jcls_Boolean, "isCleartextTrafficPermitted", "(Ljava/lang/String;)Z");
  jboolean result = env->CallBooleanMethod(java_host, jmid_isCleartextTrafficPermitted);
  env->DeleteLocalRef(java_host);
  return result == JNI_TRUE;
#else
  UNREFERENCED_PARAMETER(host);
  return true;
#endif
}
