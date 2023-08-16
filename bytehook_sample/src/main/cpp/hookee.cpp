#include <android/log.h>
#include <fcntl.h>
#include <jni.h>
#include <stdlib.h>
#include <unistd.h>
#include "Vector3.h"
#define HOOKEE_JNI_VERSION    JNI_VERSION_1_6
#define HOOKEE_JNI_CLASS_NAME "com/bytedance/android/bytehook/sample/NativeHookee"
#define HOOKEE_TAG            "bytehook_tag"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#define LOG(fmt, ...) __android_log_print(ANDROID_LOG_INFO, HOOKEE_TAG, fmt, ##__VA_ARGS__)
#pragma clang diagnostic pop

void Vect_Test()
{
    auto* pa = new Vector3;
    pa->Set(10,20,30);
    LOG("%s",pa->toString().c_str());

}

#pragma clang optimize off
static void hookee_test(JNIEnv *env, jobject thiz) {
  (void) env, (void) thiz;

  LOG("libhookee.so PRE malloc");

  Vect_Test();

  LOG("libhookee.so POST malloc");
}
#pragma clang optimize on

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
  (void)reserved;

  if (NULL == vm) return JNI_ERR;

  JNIEnv *env;
  if (JNI_OK != (vm)->GetEnv( (void **)&env, HOOKEE_JNI_VERSION)) return JNI_ERR;
  if (NULL == env || NULL == env) return JNI_ERR;

  jclass cls;
  if (NULL == (cls = (env)->FindClass( HOOKEE_JNI_CLASS_NAME))) return JNI_ERR;

  JNINativeMethod m[] = {{"nativeTest", "()V", (void *)hookee_test}};
  if (0 != (env)->RegisterNatives(cls, m, sizeof(m) / sizeof(m[0]))) return JNI_ERR;

  return HOOKEE_JNI_VERSION;
}


