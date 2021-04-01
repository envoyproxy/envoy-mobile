#include <jni.h>
#include <stdio.h>
extern "C" JNIEXPORT void JNICALL Java_test_App_f(JNIEnv *env, jclass clazz, jint x) {
  printf("hello %d\n", x);
}