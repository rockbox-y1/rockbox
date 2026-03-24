#include <jni.h>
#include <stddef.h>
#include "config.h"

#ifdef PLATFORM_INNIOASIS_Y1
extern JNIEnv *env_ptr;
extern jclass RockboxService_class;
extern jobject RockboxService_instance;

int android_reset_bluetooth(void)
{
    if (env_ptr == NULL || RockboxService_instance == NULL) {
        return -1;
    }
    static jmethodID reset_method = NULL;
    if (reset_method == NULL) {
        reset_method = (*env_ptr)->GetMethodID(env_ptr, RockboxService_class, "resetBluetooth", "()V");
        if (reset_method == NULL) {
            return -1;
        }
    }
    (*env_ptr)->CallVoidMethod(env_ptr, RockboxService_instance, reset_method);
    if ((*env_ptr)->ExceptionCheck(env_ptr)) {
        (*env_ptr)->ExceptionClear(env_ptr);
        return -1;
    }
    return 0;
}
#endif 