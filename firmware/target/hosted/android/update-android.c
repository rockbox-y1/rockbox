#include <jni.h>
#include <stddef.h>
#include <sys/stat.h>
#include "config.h"
#include "splash.h"
#include "lang.h"
#include "settings.h"

#ifdef INNIOASIS_Y1
extern JNIEnv *env_ptr;
extern jclass RockboxService_class;
extern jobject RockboxService_instance;

int android_update(void)
{
    // check if update.zip exists
    struct stat buffer;
    const char *updatePath = "/sdcard/.rockbox/update.zip";
    if (stat(updatePath, &buffer) == 0){
        if (env_ptr == NULL || RockboxService_instance == NULL) {
            return -1;
        }
        static jmethodID shutdown_method = NULL;
        if (shutdown_method == NULL) {
            shutdown_method = (*env_ptr)->GetMethodID(env_ptr, RockboxService_class, "updateRockbox", "()V");
            if (shutdown_method == NULL) {
                return -1;
            }
        }
        (*env_ptr)->CallVoidMethod(env_ptr, RockboxService_instance, shutdown_method);
        if ((*env_ptr)->ExceptionCheck(env_ptr)) {
            (*env_ptr)->ExceptionClear(env_ptr);
            return -1;
        }
        return 0;
    } else {
        splash(2*HZ, "ERROR: update.zip could not be found");
        return -1;
    }
}
#endif 