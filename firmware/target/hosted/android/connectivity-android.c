#include <jni.h>
#include "plugin.h"
#include <stddef.h>
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include "powermgmt-android.h"

#ifdef PLATFORM_INNIOASIS_Y1
/* global fields for use with various JNI calls */
extern JNIEnv *env_ptr;
extern jobject RockboxService_instance;
extern jclass  RockboxService_class;

extern bool upload_scrobble(const char *artist, const char *track, const char *album, int timestamp, long length);

bool upload_scrobble(const char *artist, const char *track, const char *album, int timestamp, long length)
{
    if (env_ptr == NULL || RockboxService_instance == NULL) {
        return false;
    }
    static jmethodID scrobbler_method = NULL;
    if (scrobbler_method == NULL) {
        scrobbler_method = (*env_ptr)->GetMethodID(env_ptr, RockboxService_class, 
                                            "lastfmScrobbler", 
                                            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IJ)Z");
        if (scrobbler_method == NULL) {
            return false;
        }
    }

    jstring artist_jstring = (*env_ptr)->NewStringUTF(env_ptr, artist);
    jstring track_jstring = (*env_ptr)->NewStringUTF(env_ptr, track);
    jstring album_jstring = (*env_ptr)->NewStringUTF(env_ptr, album);

    (*env_ptr)->CallBooleanMethod(env_ptr, RockboxService_instance, scrobbler_method, 
                                    artist_jstring, 
                                    track_jstring, 
                                    album_jstring, 
                                    (jint) timestamp,
                                    (jlong) length);
    if ((*env_ptr)->ExceptionCheck(env_ptr)) {
        (*env_ptr)->ExceptionClear(env_ptr);
        return false;
    }
    return true;
}

extern int android_podcast_download_episode(int podcast_num, int num);
int android_podcast_download_episode(int podcast_num, int num)
{
    if (env_ptr == NULL || RockboxService_instance == NULL) {
        return -1;
    }
    static jmethodID podcast_method = NULL;
    if (podcast_method == NULL) {
        podcast_method = (*env_ptr)->GetMethodID(env_ptr, RockboxService_class, "startPodcastDownload", "(II)V");
        if (podcast_method == NULL) {
    return -1;
        }
    }
    (*env_ptr)->CallVoidMethod(env_ptr, RockboxService_instance, podcast_method, podcast_num, num);
    if ((*env_ptr)->ExceptionCheck(env_ptr)) {
        (*env_ptr)->ExceptionClear(env_ptr);
        return -1;
    }
    return 0;
}

extern int android_podcast_delete_episode(int podcast_num, int num);
int android_podcast_delete_episode(int podcast_num, int num)
{
    
    if (env_ptr == NULL || RockboxService_instance == NULL) {
        return -1;

    }
    static jmethodID podcast_method = NULL;
    if (podcast_method == NULL) {
        podcast_method = (*env_ptr)->GetMethodID(env_ptr, RockboxService_class, "deleteEpisode", "(II)V");
        if (podcast_method == NULL) {
            return -1;
        }
    }
    (*env_ptr)->CallVoidMethod(env_ptr, RockboxService_instance, podcast_method, podcast_num, num);
    if ((*env_ptr)->ExceptionCheck(env_ptr)) {
        (*env_ptr)->ExceptionClear(env_ptr);
        return -1;
    }
    return 0;
}

char** split_string_newline(const char* str, int* count) {
    if (!str) {
        *count = 0;
        return NULL;
    }
    
    // count number of lines
    *count = 1;
    const char* ptr = str;
    while (*ptr) {
        if (*ptr == '\n') {
            (*count)++;
        }
        ptr++;
    }
    
    char** result = malloc((*count + 1) * sizeof(char*)); // +1 for terminator
    if (!result) {
        *count = 0;
        return NULL;
    }
    
    // actually split string
    char* copy = strdup(str);
    if (!copy) {
        free(result);
        *count = 0;
        return NULL;
    }
    
    int index = 0;
    char* token = strtok(copy, "\n");
    while (token && index < *count) {
        result[index] = strdup(token);
        if (result[index]) {
            index++;
        }
        token = strtok(NULL, "\n");
    }
    
    result[index] = NULL;
    free(copy);
    
    return result;
}

extern void free_array(char** array);
void free_array(char** array) {
    if (array) {
        int i = 0;
        while (array[i]) {
            free(array[i]);
            i++;
        }
        free(array);
    }
}

extern char** android_podcast_get_podcast_names(void);
char** android_podcast_get_podcast_names(void)
{
    if (env_ptr == NULL || RockboxService_instance == NULL) {
        return NULL;
    }
    
    static jmethodID podcast_method = NULL;
    if (podcast_method == NULL) {
        podcast_method = (*env_ptr)->GetMethodID(env_ptr, RockboxService_class, "getPodcastNames", "()Ljava/lang/String;");
        if (podcast_method == NULL) {
            return NULL;
        }
    }
    
    jstring jstr = (*env_ptr)->CallObjectMethod(env_ptr, RockboxService_instance, podcast_method);
    
    if ((*env_ptr)->ExceptionCheck(env_ptr)) {
                (*env_ptr)->ExceptionClear(env_ptr);
        return NULL;
    }
    
    if (jstr == NULL) {
        return NULL;
    }
    
    const char* cstr = (*env_ptr)->GetStringUTFChars(env_ptr, jstr, NULL);
    if (cstr == NULL) {
        (*env_ptr)->DeleteLocalRef(env_ptr, jstr);
        return NULL;
    }
    
    // Split the string into array
    int count;
    char** result = split_string_newline(cstr, &count);
    
    // Clean up
    (*env_ptr)->ReleaseStringUTFChars(env_ptr, jstr, cstr);
    (*env_ptr)->DeleteLocalRef(env_ptr, jstr);
    
    return result;
}

extern char** android_podcast_get_episode_list(int podcast_num);
char** android_podcast_get_episode_list(int podcast_num)
{
    if (env_ptr == NULL || RockboxService_instance == NULL) {
        return NULL;
    }
    
    static jmethodID podcast_method = NULL;
    if (podcast_method == NULL) {
        podcast_method = (*env_ptr)->GetMethodID(env_ptr, RockboxService_class, "getEpisodeList", "(I)Ljava/lang/String;");
        if (podcast_method == NULL) {
            return NULL;
        }
    }
    
    jstring jstr = (*env_ptr)->CallObjectMethod(env_ptr, RockboxService_instance, podcast_method, podcast_num);
    
    if ((*env_ptr)->ExceptionCheck(env_ptr)) {
        (*env_ptr)->ExceptionClear(env_ptr);
        return NULL;
    }
    
    if (jstr == NULL) {
        return NULL;
    }
    
    const char* cstr = (*env_ptr)->GetStringUTFChars(env_ptr, jstr, NULL);
    if (cstr == NULL) {
        (*env_ptr)->DeleteLocalRef(env_ptr, jstr);
        return NULL;
    }
    
    // Split the string into array
    int count;
    char** result = split_string_newline(cstr, &count);
    
    // Clean up
    (*env_ptr)->ReleaseStringUTFChars(env_ptr, jstr, cstr);
    (*env_ptr)->DeleteLocalRef(env_ptr, jstr);
    
    return result;
}

extern const char* android_podcast_get_episode_path(int podcast_num, int num);
const const char* android_podcast_get_episode_path(int podcast_num, int num)
{
    if (env_ptr == NULL || RockboxService_instance == NULL) {
        return NULL;
    }
    
    static jmethodID podcast_method = NULL;
    if (podcast_method == NULL) {
        podcast_method = (*env_ptr)->GetMethodID(env_ptr, RockboxService_class, "getEpisodePath", "(II)Ljava/lang/String;");
        if (podcast_method == NULL) {
            return NULL;
        }
    }
    
    jstring jstr = (*env_ptr)->CallObjectMethod(env_ptr, RockboxService_instance, podcast_method, podcast_num, num);
    
    if ((*env_ptr)->ExceptionCheck(env_ptr)) {
        (*env_ptr)->ExceptionClear(env_ptr);
        return NULL;
    }
    
    if (jstr == NULL) {
        return NULL;
    }
    
    const char* cstr = (*env_ptr)->GetStringUTFChars(env_ptr, jstr, NULL);
    if (cstr == NULL) {
        (*env_ptr)->DeleteLocalRef(env_ptr, jstr);
        return NULL;
    }
    
    return cstr;
}

extern int android_podcast_get_list_count(char** array);
int android_podcast_get_list_count(char** array) {
    if (!array) return 0;
    int count = 0;
    while (array[count]) {
        count++;
    }
    return count;
}

extern const char* android_podcast_connect_wifi(void);
const char* android_podcast_connect_wifi(void)
{
    android_acquire_wakelock();
    if (env_ptr == NULL || RockboxService_instance == NULL) {
        return NULL;
    }
    
    static jmethodID podcast_method = NULL;
    if (podcast_method == NULL) {
        podcast_method = (*env_ptr)->GetMethodID(env_ptr, RockboxService_class, "connectWifi", "()Ljava/lang/String;");
        if (podcast_method == NULL) {
            return NULL;
        }
    }
    
    jstring jstr = (*env_ptr)->CallObjectMethod(env_ptr, RockboxService_instance, podcast_method);
    
    if ((*env_ptr)->ExceptionCheck(env_ptr)) {
        (*env_ptr)->ExceptionClear(env_ptr);
        return NULL;
    }
    
    if (jstr == NULL) {
        return NULL;
    }
    
    const char* cstr = (*env_ptr)->GetStringUTFChars(env_ptr, jstr, NULL);
    if (cstr == NULL) {
        (*env_ptr)->DeleteLocalRef(env_ptr, jstr);
        return NULL;
    }
    
    return cstr;
}

extern int android_podcast_disconnect_wifi(void);
int android_podcast_disconnect_wifi(void)
{
    android_release_wakelock();
    if (env_ptr == NULL || RockboxService_instance == NULL) {
        return -1;
    }
    static jmethodID podcast_method = NULL;
    if (podcast_method == NULL) {
        podcast_method = (*env_ptr)->GetMethodID(env_ptr, RockboxService_class, "disconnectWifi", "()V");
        if (podcast_method == NULL) {
    return -1;
        }
    }
    (*env_ptr)->CallVoidMethod(env_ptr, RockboxService_instance, podcast_method);
    if ((*env_ptr)->ExceptionCheck(env_ptr)) {
        (*env_ptr)->ExceptionClear(env_ptr);
        return -1;
    }
    return 0;
}
#endif