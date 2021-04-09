#include <jni.h>

extern "C" JNIEXPORT jint JNICALL Java_test_kotlin_Main_foo(JNIEnv *, jobject) {
   return 42;
}