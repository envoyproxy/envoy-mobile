#pragma once

#include <string>
#include <vector>

#include "library/common/api/c_types.h"
#include "library/common/extensions/cert_validator/platform_bridge/c_types.h"
#include "library/common/jni/import/jni_import.h"

// NOLINT(namespace-envoy)

/* Calls up through JNI to validate given certificates.
 */
jobject call_jvm_verify_x509_cert_chain(JNIEnv* env, const std::vector<std::string>& cert_chain,
                                        std::string auth_type, std::string host);

/* Returns a group of C functions to do certificates validation using AndroidNetworkLibrary.
 */
envoy_cert_validator* get_android_cert_validator_api();

/* Extracts the certificate verification status from the Java object return by
 * call_jvm_verify_x509_cert_chain.
 * Used only for testing.
 */
envoy_cert_verify_status_t jvm_cert_get_status(JNIEnv* env, jobject j_result);

/* Extracts whether the certificate was issued by a known root from the Java
 * returned by call_jvm_verify_x509_cert_chain.
 * Used only for testing.
 */
bool jvm_cert_is_issued_by_known_root(JNIEnv* env, jobject result);

static constexpr const char* cert_validator_name = "platform_cert_validator";
