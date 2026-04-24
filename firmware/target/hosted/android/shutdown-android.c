#include <jni.h>
#include <stddef.h>
#include "config.h"

#ifdef INNIOASIS_Y1
extern JNIEnv *env_ptr;
extern jclass RockboxService_class;
extern jobject RockboxService_instance;

int android_shutdown_device(int show_dialog)
{
    if (env_ptr == NULL || RockboxService_instance == NULL) {
        return -1;
    }
    static jmethodID shutdown_method = NULL;
    if (shutdown_method == NULL) {
        shutdown_method = (*env_ptr)->GetMethodID(env_ptr, RockboxService_class, "shutdownDevice", "(I)V");
        if (shutdown_method == NULL) {
            return -1;
        }
    }
    (*env_ptr)->CallVoidMethod(env_ptr, RockboxService_instance, shutdown_method, show_dialog);
    if ((*env_ptr)->ExceptionCheck(env_ptr)) {
        (*env_ptr)->ExceptionClear(env_ptr);
        return -1;
    }
    return 0;
}
int android_switch_firmware(void)
{
    if (env_ptr == NULL || RockboxService_instance == NULL) {
        return -1;
    }
    static jmethodID firmware_switch_method = NULL;
    if (firmware_switch_method == NULL) {
        firmware_switch_method = (*env_ptr)->GetMethodID(env_ptr, RockboxService_class, "switchFirmware", "()V");
        if (firmware_switch_method == NULL) {
            return -1;
        }
    }
    (*env_ptr)->CallVoidMethod(env_ptr, RockboxService_instance, firmware_switch_method);
    if ((*env_ptr)->ExceptionCheck(env_ptr)) {
        (*env_ptr)->ExceptionClear(env_ptr);
        return -1;
    }
    return 0;
}

#endif 