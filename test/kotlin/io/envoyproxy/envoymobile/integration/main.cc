#include <jni.h>
#include <stdio.h>

JNIEXPORT jint JNICALL Java_Main_foo(JNIEnv *, jobject) {
   return 42;
}