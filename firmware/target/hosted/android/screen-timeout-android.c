#include "config.h"
#include "system.h"
#include "debug.h"
#include "screen-timeout-android.h"
#include <jni.h>

#ifdef PLATFORM_INNIOASIS_Y1

/* External references to JNI environment and service */
extern JNIEnv *env_ptr;
extern jclass RockboxService_class;
extern jobject RockboxService_instance;

/* Method IDs for Java methods */
static jmethodID android_set_screen_timeout_method = NULL;
static jmethodID android_get_screen_timeout_method = NULL;

/* Set Android screen timeout to a specific value in seconds */
int android_screen_timeout_set(int timeoutSeconds)
{
    /* Check if JNI environment and service are available */
    if (env_ptr == NULL || RockboxService_instance == NULL) {
        return -1; /* Error if not ready */
    }
    
    /* Get method ID if not already cached */
    if (android_set_screen_timeout_method == NULL) {
        android_set_screen_timeout_method = (*env_ptr)->GetMethodID(env_ptr, RockboxService_class,
                                                     "setAndroidScreenTimeout", "(I)I");
        if (android_set_screen_timeout_method == NULL) {
            return -1; /* Error on method not found */
        }
    }
    
    /* Call the Java method */
    jint result = (*env_ptr)->CallIntMethod(env_ptr, RockboxService_instance, 
                                 android_set_screen_timeout_method, (jint)timeoutSeconds);
    
    /* Check for exceptions */
    if ((*env_ptr)->ExceptionCheck(env_ptr)) {
        (*env_ptr)->ExceptionClear(env_ptr);
        return -1; /* Error on exception */
    }
    
    return (int)result;
}

/* Get current Android screen timeout in seconds */
int android_screen_timeout_get(void)
{
    /* Check if JNI environment and service are available */
    if (env_ptr == NULL || RockboxService_instance == NULL) {
        return -1; /* Error if not ready */
    }
    
    /* Get method ID if not already cached */
    if (android_get_screen_timeout_method == NULL) {
        android_get_screen_timeout_method = (*env_ptr)->GetMethodID(env_ptr, RockboxService_class,
                                                     "getAndroidScreenTimeout", "()I");
        if (android_get_screen_timeout_method == NULL) {
            return -1; /* Error on method not found */
        }
    }
    
    /* Call the Java method */
    jint result = (*env_ptr)->CallIntMethod(env_ptr, RockboxService_instance, 
                                 android_get_screen_timeout_method);
    
    /* Check for exceptions */
    if ((*env_ptr)->ExceptionCheck(env_ptr)) {
        (*env_ptr)->ExceptionClear(env_ptr);
        return -1; /* Error on exception */
    }
    
    return (int)result;
}

#endif /* PLATFORM_INNIOASIS_Y1 */ 