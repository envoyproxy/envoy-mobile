#include <jni.h>
#include <stdio.h>

extern "C" JNIEXPORT jint JNICALL Java_test_App_f(JNIEnv *env, jclass clazz, jint x) {
  printf("hello world %d\n", x);
  return x;
}