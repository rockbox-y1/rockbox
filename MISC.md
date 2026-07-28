# Collection of technical information and quirks
## Replacing vibration motor with Taptic Engine
An iPhone 12 Mini taptic engine fits the housing when the internal speaker is removed. You can roughly follow [this video](https://www.youtube.com/watch?v=zDgUktwXOa0).

The haptic feedback is a lot better since it responds quicker to the vibration signal compared to the original vibration motor. It is recommended to enable Settings > General Settings > Keyclick > Taptic Engine Mode.

## Keylock exemptions are weird
Sometimes keylock exemptions take some time to work or need a second press. It seems like this is a limitation that will be hard to fully solve without the touch wheel driver sources or access to the AOSP source used for this ROM. Once the device is in sleep mode for a while the touch wheel simply stops sending button presses to the kernel. This is likely a power-saving setting that an app has no control over. Feel free to contribute a fix for this if you have one.

## Overwriting files usually found in .rockbox (language files and others)
To for example edit `.lang` files you will want to access the respective files, usually found .rockbox. Currently these files are not populated in the .rockbox folder on the SD card of the device but instead on the internal storage (`/data/data/org.rockbox/app_rockbox/rockbox/langs`) . If you place these files in the .rockbox folder however Rockbox will use these instead of the files in the internal storage.

**So where can you find and edit these files easily?**
- download `update.zip` of the Rockbox release you are using
- extract `update.zip`
- rename `/data/data/org.rockbox/app_rockbox/rockbox/langs/libmisc.so` to libmisc.zip and extract it
- modify the files in the .rockbox folder you just extracted from `libmisc` and place them in the .rockbox folder on the SD card
- restart Rockbox

## Time can't be adjusted when connected to Wi-Fi
The time should be set to the correct time and time zone automatically if you are connected to the internet. If you still want to adjust the time manually, deactivate the automatic time setting in the Android System Menu.