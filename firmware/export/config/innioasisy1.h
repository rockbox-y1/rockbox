/*
 * This config file is for Rockbox as an application on Android (Innioasis Y1 specific)
 */

/* We don't run on hardware directly */
#define CONFIG_PLATFORM (PLATFORM_HOSTED|PLATFORM_INNIOASIS_Y1)
#define HAVE_FPU

/* For Rolo and boot loader */
#define MODEL_NUMBER 100

#define MODEL_NAME   "Rockbox"

#define USB_NONE

/* define this if you have a colour LCD */
#define HAVE_LCD_COLOR

/* Define this for LCD backlight available */
#define HAVE_BACKLIGHT
/* Enable LCD brightness control */
#define HAVE_BACKLIGHT_BRIGHTNESS
/* Main LCD backlight brightness range and defaults */
#define MIN_BRIGHTNESS_SETTING      1
#define MAX_BRIGHTNESS_SETTING      100
#define DEFAULT_BRIGHTNESS_SETTING  100

/* define this if you want album art for this target */
#define HAVE_ALBUMART

/* define this to enable bitmap scaling */
#define HAVE_BMP_SCALING

/* define this to enable JPEG decoding */
#define HAVE_JPEG

/* define this if you have access to the quickscreen */
#define HAVE_QUICKSCREEN

/* define this if you would like tagcache to build on this target */
#define HAVE_TAGCACHE

/* LCD dimensions
 *
 * overriden by configure for application builds */
#ifndef LCD_WIDTH
#define LCD_WIDTH  360
#endif

#ifndef LCD_HEIGHT
#define LCD_HEIGHT 480
#endif

#define LCD_DEPTH  16
#define LCD_PIXELFORMAT RGB565

#define HAVE_LCD_ENABLE

/* define this to indicate your device's keypad */
//#define HAVE_TOUCHSCREEN
#define HAVE_BUTTON_DATA
/* define this if you have a real-time clock */
#define CONFIG_RTC APPLICATION

/* The number of bytes reserved for loadable codecs */
#define CODEC_SIZE 0x100000

/* The number of bytes reserved for loadable plugins */
#define PLUGIN_BUFFER_SIZE 0x200000

#define AB_REPEAT_ENABLE
#define ACTION_WPSAB_SINGLE ACTION_WPS_BROWSE

#define AUDIOHW_HAVE_BALANCE

#define HAVE_MULTIMEDIA_KEYS
#define CONFIG_KEYPAD INNIOASIS_Y1_PAD

/* define this if the target has volume keys which can be used in the lists */
#define HAVE_VOLUME_IN_LIST

/* define this if the host platform can change volume outside of rockbox */
#define PLATFORM_HAS_VOLUME_CHANGE

#define HAVE_SW_TONE_CONTROLS 

#define HAVE_HEADPHONE_DETECTION

#define HAVE_TIME_ESTIMATION
#define CONFIG_BATTERY_MEASURE PERCENTAGE_MEASURE
#define CONFIG_CHARGING CHARGING_MONITOR

#define NO_LOW_BATTERY_SHUTDOWN
/* Define this to the CPU frequency */
/*
#define CPU_FREQ 48000000
*/

#define CONFIG_LCD LCD_COWOND2

/* Define this if a programmable hotkey is mapped */
#define HAVE_HOTKEY

#define BOOTDIR "/.rockbox"

/* No special storage */
#define CONFIG_STORAGE STORAGE_HOSTFS
#define HAVE_STORAGE_FLUSH
