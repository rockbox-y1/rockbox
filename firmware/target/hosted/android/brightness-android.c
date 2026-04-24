#include "config.h"
#include "system.h"
#include "debug.h"
#include "brightness-android.h"
#include <jni.h>

#ifdef INNIOASIS_Y1

/* External references to JNI environment and service */
extern JNIEnv *env_ptr;
extern jclass RockboxService_class;
extern jobject RockboxService_instance;

/* Method IDs for Java methods */
static jmethodID android_set_brightness_method = NULL;
static jmethodID android_get_brightness_percent_method = NULL;

/* Set Android brightness to a specific percentage (0-100) */
void backlight_hw_brightness(int percent)
{    
    /* Get method ID if not already cached */
    if (android_set_brightness_method == NULL) {
        android_set_brightness_method = (*env_ptr)->GetMethodID(env_ptr, RockboxService_class,
                                                     "setAndroidBrightnessPercent", "(I)I");
    }
    
    /* Call the Java method */
    (*env_ptr)->CallIntMethod(env_ptr, RockboxService_instance, 
                              android_set_brightness_method, (jint)percent);
        
    return;
}

/* Get current Android brightness as percentage (0-100) */
int android_brightness_get_percent(void)
{
    /* Check if JNI environment and service are available */
    if (env_ptr == NULL || RockboxService_instance == NULL) {
        return 100; /* Default to 100% if not ready */
    }
    
    /* Get method ID if not already cached */
    if (android_get_brightness_percent_method == NULL) {
        android_get_brightness_percent_method = (*env_ptr)->GetMethodID(env_ptr, RockboxService_class,
                                                         "getAndroidBrightnessPercent", "()I");
        if (android_get_brightness_percent_method == NULL) {
            return 100; /* Default to 100% on error */
        }
    }
    
    /* Call the Java method */
    jint result = (*env_ptr)->CallIntMethod(env_ptr, RockboxService_instance, 
                                 android_get_brightness_percent_method);
    
    /* Check for exceptions */
    if ((*env_ptr)->ExceptionCheck(env_ptr)) {
        (*env_ptr)->ExceptionClear(env_ptr);
        return 100; /* Default to 100% on error */
    }
    
    return (int)result;
}

/* placeholders, android does this system-wide */
bool backlight_hw_init(void){
    return true;
}
void backlight_hw_on(void){
    return;
}
void backlight_hw_off(void){
    return;
}

#endif /* INNIOASIS_Y1 */ 