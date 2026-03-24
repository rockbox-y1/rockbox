#ifndef SCREEN_TIMEOUT_ANDROID_H
#define SCREEN_TIMEOUT_ANDROID_H

/* Set Android screen timeout to a specific value in seconds */
int android_screen_timeout_set(int timeoutSeconds);

/* Get current Android screen timeout in seconds */
int android_screen_timeout_get(void);

#endif /* SCREEN_TIMEOUT_ANDROID_H */ 