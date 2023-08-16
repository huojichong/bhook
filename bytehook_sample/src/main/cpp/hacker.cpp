#include <android/log.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <inttypes.h>
#include <jni.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "bytehook.h"
#include "Vector3.h"

#define HACKER_JNI_VERSION    JNI_VERSION_1_6
#define HACKER_JNI_CLASS_NAME "com/bytedance/android/bytehook/sample/NativeHacker"
#define HACKER_TAG            "bytehook_tag"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#define LOG(fmt, ...) __android_log_print(ANDROID_LOG_INFO, HACKER_TAG, fmt, ##__VA_ARGS__)
#pragma clang diagnostic pop


typedef void (*VectSet_t)(Vector3* ,float,float,float);


#define OPEN_DEF(fn)                                                                                         \
  static fn##_t fn##_prev = NULL;                                                                            \
  static bytehook_stub_t fn##_stub = NULL;                                                                   \
  static void fn##_hooked_callback(bytehook_stub_t task_stub, int status_code, const char *caller_path_name, \
                                   const char *sym_name, void *new_func, void *prev_func, void *arg) {       \
    if (BYTEHOOK_STATUS_CODE_ORIG_ADDR == status_code) {                                                     \
      fn##_prev = (fn##_t)prev_func;                                                                         \
      LOG(">>>>> save original address: %" PRIxPTR, (uintptr_t)prev_func);                                   \
    } else {                                                                                                 \
      LOG(">>>>> hooked. stub: %" PRIxPTR                                                                    \
          ", status: %d, caller_path_name: %s, sym_name: %s, new_func: %" PRIxPTR ", prev_func: %" PRIxPTR   \
          ", arg: %" PRIxPTR,                                                                                \
          (uintptr_t)task_stub, status_code, caller_path_name, sym_name, (uintptr_t)new_func,                \
          (uintptr_t)prev_func, (uintptr_t)arg);                                                             \
    }                                                                                                        \
  }
OPEN_DEF(VectSet)

static void debug(const char *sym, const char *pathname, int flags, int fd, void *lr) {
  Dl_info info;
  memset(&info, 0, sizeof(info));
  dladdr(lr, &info);

  LOG("proxy %s(\"%s\", %d), return FD: %d, called from: %s (%s)", sym, pathname, flags, fd, info.dli_fname,
      info.dli_sname);
}

static int open_proxy_auto(const char *pathname, int flags, mode_t modes) {
  // In automatic mode, if you need to call the original function,
  // please always use the BYTEHOOK_CALL_PREV() macro.
  BYTEHOOK_STACK_SCOPE();
  int fd = BYTEHOOK_CALL_PREV(open_proxy_auto, pathname, flags, modes);
  debug("open", pathname, flags, fd, BYTEHOOK_RETURN_ADDRESS());

  return fd;
}

static int open_real_proxy_auto(const char *pathname, int flags, mode_t modes) {
    BYTEHOOK_STACK_SCOPE();
  int fd = BYTEHOOK_CALL_PREV(open_real_proxy_auto, pathname, flags, modes);
  debug("__open_real", pathname, flags, fd, BYTEHOOK_RETURN_ADDRESS());

  return fd;
}

static int open2_proxy_auto(const char *pathname, int flags) {
    BYTEHOOK_STACK_SCOPE();
    int fd = BYTEHOOK_CALL_PREV(open2_proxy_auto, pathname, flags);
  debug("__open_2", pathname, flags, fd, BYTEHOOK_RETURN_ADDRESS());

  return fd;
}


static void VectSet_Proxy(Vector3* vect, float x, float y, float z)
{
    BYTEHOOK_STACK_SCOPE();
    LOG("vect_proxy %f,%f,%f",x,y,z);
    BYTEHOOK_CALL_PREV(VectSet_Proxy,vect,20,20,20);
}


static bool allow_filter(const char *caller_path_name, void *arg) {
  (void)arg;

//  if (NULL != strstr(caller_path_name, "libc.so")) return false;
//  if (NULL != strstr(caller_path_name, "libbase.so")) return false;
//  if (NULL != strstr(caller_path_name, "liblog.so")) return false;
//  if (NULL != strstr(caller_path_name, "libunwindstack.so")) return false;
//  if (NULL != strstr(caller_path_name, "libutils.so")) return false;
  if (NULL != strstr(caller_path_name, "libunity.so")) return false;
  // ......

  return true;
}

static bool allow_filter_for_hook_all(const char *caller_path_name, void *arg) {
  (void)arg;

  if (NULL != strstr(caller_path_name, "liblog.so")) return false;

  return true;
}

static int hacker_hook(JNIEnv *env, jobject thiz, jint type) {
  (void)env, (void)thiz;


  VectSet_stub = bytehook_hook_partial(allow_filter, NULL, NULL, "_ZN7Vector33SetEfff",
                                       (void*)VectSet_Proxy, VectSet_hooked_callback, NULL);
  return 0;
}

static int hacker_unhook(JNIEnv *env, jobject thiz) {
  (void)env, (void)thiz;

  if(VectSet_stub != nullptr)
  {
      bytehook_unhook(VectSet_stub);
      VectSet_stub = nullptr;
  }
  return 0;
}

static void hacker_dump_records(JNIEnv *env, jobject thiz, jstring pathname) {
  (void)thiz;

  const char *c_pathname = (env)->GetStringUTFChars( pathname, 0);
  if (NULL == c_pathname) return;

  int fd = open(c_pathname, O_CREAT | O_WRONLY | O_CLOEXEC | O_TRUNC | O_APPEND, S_IRUSR | S_IWUSR);
  if (fd >= 0) {
    bytehook_dump_records(fd, BYTEHOOK_RECORD_ITEM_ALL);
    close(fd);
  }

  (env)->ReleaseStringUTFChars( pathname, c_pathname);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
  (void)reserved;

  if (NULL == vm) return JNI_ERR;

  JNIEnv *env;
  if (JNI_OK != (vm)->GetEnv( (void **)&env, HACKER_JNI_VERSION)) return JNI_ERR;
  if (NULL == env ) return JNI_ERR;

  jclass cls;
  if (NULL == (cls = (env)->FindClass( HACKER_JNI_CLASS_NAME))) return JNI_ERR;

  JNINativeMethod m[] = {{"nativeHook", "(I)I", (void *)hacker_hook},
                         {"nativeUnhook", "()I", (void *)hacker_unhook},
                         {"nativeDumpRecords", "(Ljava/lang/String;)V", (void *)hacker_dump_records}};
  if (0 != (env)->RegisterNatives( cls, m, sizeof(m) / sizeof(m[0]))) return JNI_ERR;

  return HACKER_JNI_VERSION;
}
