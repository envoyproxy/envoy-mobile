#include <ares.h>
#include <jni.h>
#include <string.h>

#include "main_interface.h"

static JNIEnv* env = nullptr;

// NOLINT(namespace-envoy)

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
  // JNIEnv* env = nullptr;

  if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
    return -1;
  }

  // c-ares jvm init is necessary in order to let c-ares perform DNS resolution in Envoy.
  // More information can be found at:
  // https://c-ares.haxx.se/ares_library_init_android.html
  ares_library_init_jvm(vm);
  return JNI_VERSION_1_6;
}

// JniLibrary

extern "C" JNIEXPORT jlong JNICALL Java_io_envoyproxy_envoymobile_engine_JniLibrary_initEngine(
    JNIEnv* env,
    jclass // class
) {
  return init_engine();
}

extern "C" JNIEXPORT jint JNICALL
Java_io_envoyproxy_envoymobile_engine_JniLibrary_runEngine(JNIEnv* env,
                                                           jobject, // this
                                                           jstring config, jstring log_level) {
  return run_engine(env->GetStringUTFChars(config, nullptr),
                    env->GetStringUTFChars(log_level, nullptr));
}

// AndroidJniLibrary

extern "C" JNIEXPORT jint JNICALL
Java_io_envoyproxy_envoymobile_engine_AndroidJniLibrary_initialize(JNIEnv* env,
                                                                   jclass, // class
                                                                   jobject connectivity_manager) {
  // See note above about c-ares.
  return ares_library_init_android(connectivity_manager);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_io_envoyproxy_envoymobile_engine_AndroidJniLibrary_isAresInitialized(JNIEnv* env,
                                                                          jclass // class
) {
  return ares_library_android_initialized() == ARES_SUCCESS;
}

extern "C" JNIEXPORT jstring JNICALL
Java_io_envoyproxy_envoymobile_engine_JniLibrary_templateString(JNIEnv* env,
                                                                jclass // class
) {
  jstring result = env->NewStringUTF(config_template);
  return result;
}

// EnvoyHttpStream

static void pass_headers(envoy_headers headers, jobject j_context) {
  jclass jcls_JvmObserverContext = env->GetObjectClass(j_context);
  jmethodID jmid_passHeader = env->GetMethodID(jcls_JvmObserverContext, "passHeader", "([B[BZ)V");
  env->PushLocalFrame(headers.length * 2);
  for (envoy_header_size_t i = 0; i < headers.length; i++) {
    // Note this is just an initial implementation, and we will pass a more optimized structure in
    // the future.

    // Note the JNI function NewStringUTF would appear to be an appealing option here, except it
    // requires a null-terminated *modified* UTF-8 string.

    // Create platform byte array for header key
    jbyteArray key = env->NewByteArray(headers.headers[i].key.length);
    void* critical_key = env->GetPrimitiveArrayCritical(key, 0);
    memcpy(critical_key, headers.headers[i].key.bytes, headers.headers[i].key.length);
    env->ReleasePrimitiveArrayCritical(key, critical_key, 0);

    // Create platform byte array for header value
    jbyteArray value = env->NewByteArray(headers.headers[i].value.length);
    void* critical_value = env->GetPrimitiveArrayCritical(value, 0);
    memcpy(critical_value, headers.headers[i].value.bytes, headers.headers[i].value.length);
    env->ReleasePrimitiveArrayCritical(value, critical_value, 0);

    // Pass this header pair to the platform
    env->CallVoidMethod(j_context, jmid_passHeader, key, value, i != headers.length - 1);
  }
  env->PopLocalFrame(nullptr);
  release_envoy_headers(headers);
}

static void jvm_on_headers(envoy_headers headers, bool end_stream, void* context) {
  jobject j_context = static_cast<jobject>(context);
  jclass jcls_JvmObserverContext = env->GetObjectClass(j_context);
  jmethodID jmid_onHeaders = env->GetMethodID(jcls_JvmObserverContext, "onHeaders", "(JZ)V");
  env->CallVoidMethod(j_context, jmid_onHeaders, headers.length);
  pass_headers(headers, j_context);
}

static envoy_headers to_native_headers(jobjectArray headers) {
  // FIXME: Right now we're using an array of jstrings, which we can access as a
  // char array of modified UTF-8 bytes. This is technically incorrect and could
  // lead to problems down the road (though it's unlikely we'll encounter any issues in the near
  // term). Fix by converting the strings to true UTF-8 byte arrays in java, and memcpying the array
  // critical here.

  // Note that headers is a flattened array of key/value pairs.
  // Therefore, the length of the native header array is n envoy_data or n/2 envoy_header.
  envoy_header_size_t length = env->GetArrayLength(headers);
  envoy_header* header_array = (envoy_header*)malloc(sizeof(envoy_header) * length / 2);

  for (envoy_header_size_t i = 0; i < length; i += 2) {
    // Copy native byte array for header key
    jbyteArray j_key = (jbyteArray)env->GetObjectArrayElement(headers, i);
    size_t key_length = env->GetArrayLength(j_key);
    uint8_t* native_key = (uint8_t*)malloc(key_length);
    void* critical_key = env->GetPrimitiveArrayCritical(j_key, 0);
    memcpy(native_key, critical_key, key_length);
    env->ReleasePrimitiveArrayCritical(j_key, critical_key, 0);
    envoy_data header_key = {key_length, native_key, free, native_key};

    // Copy native byte array for header value
    jbyteArray j_name = (jbyteArray)env->GetObjectArrayElement(headers, i + 1);
    size_t name_length = env->GetArrayLength(j_name);
    uint8_t* native_name = (uint8_t*)malloc(name_length);
    void* critical_name = env->GetPrimitiveArrayCritical(j_name, 0);
    memcpy(native_name, critical_name, name_length);
    env->ReleasePrimitiveArrayCritical(j_name, critical_name, 0);
    envoy_data header_name = {name_length, native_name, free, native_name};

    header_array[i] = {header_key, header_name};
  }

  envoy_headers native_headers = {length / 2, header_array};
  return native_headers;
}

extern "C" JNIEXPORT jlong JNICALL Java_io_envoyproxy_envomobile_engine_JniLibrary_initStream(
    JNIEnv* env, jlong engine_handle,
    jclass // class
) {
  return init_stream(static_cast<envoy_engine_t>(engine_handle));
}

extern "C" JNIEXPORT jint JNICALL Java_io_envoyproxy_envomobile_engine_JniLibrary_startStream(
    JNIEnv* env, jlong stream_handle, jobject j_context,
    jclass // class
) {
  envoy_observer native_obs = {jvm_on_headers, nullptr, nullptr,  nullptr,
                               nullptr,        nullptr, j_context};
  return start_stream(static_cast<envoy_stream_t>(stream_handle), native_obs);
}

extern "C" JNIEXPORT jint JNICALL Java_io_envoyproxy_envomobile_engine_JniLibrary_sendHeaders(
    JNIEnv* env, jlong stream_handle, jobjectArray headers, jboolean end_stream,
    jclass // class
) {
  return send_headers(static_cast<envoy_stream_t>(stream_handle), to_native_headers(headers), end_stream);
}