#ifndef BRIGHTNESS_ANDROID_H
#define BRIGHTNESS_ANDROID_H

/* Set Android brightness to a specific percentage (0-100) */
//int android_brightness_set_percent(int percent);

/* Get current Android brightness as percentage (0-100) */
int android_brightness_get_percent(void);

bool backlight_hw_init(void);
void backlight_hw_on(void);
void backlight_hw_off(void);
void backlight_hw_brightness(int brightness);

#endif /* BRIGHTNESS_ANDROID_H */ 