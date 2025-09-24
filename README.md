# Rockbox Android Fork for Innioasis Y1

| <img src="./img/240p_menu.png" alt="Rockbox-240p Menu" width="75%"/> | <img src="./img/240p_wps.png" alt="Rockbox-240p WPS" width="75%"/> |
|:--:|:--:|
| Rockbox Menu | Rockbox WPS |

## General Information

This is an experimental build of Rockbox that is not in any way associated with or developed by Innioasis/Timmkoo.

Most of the work was already done by the original Rockbox team - all credits to them.

I mostly added quick hacks to make this usable on a device without any touch inputs.

Do NOT run this if you don't know what you are doing. You might brick your device in the process of installing this app. You have been warned.

Installation instructions can be found here: https://innioasis.app/

## Controls

- Scroll Wheel (Most Screens): Up / Down
- Scroll Wheel (Now Playing screen): Volume
- Center (Short): Accept / Enter
- Center (Long): Turn Off Screen
- Menu/Back (Short): Cancel / Back
- Menu/Back (Long): Open Context Menu
- Play/Pause (Long) (Most Screens): Open While Playing Screen
- Play/Pause (Long) (Now Playing screen): WPS Hotkey
- Play/Pause (Short) (Text Input): Open Keyboard
- Media Buttons: Play/Pause/Next/Previous/Seek
- Holding Next + Previous (>5 seconds): Restart Rockbox (Thanks, u/thinkVHS!)
- Holding Menu/Back + Play/Pause (>10 seconds): Switch between Rockbox and Stock

## Themes

### Installation

1. (optional) Download the fontpack, extract it, drag the .rockbox folder onto your device https://www.rockbox.org/dl.cgi?bin=fonts
2. Download a theme from (360p see below, 240p: https://themes.rockbox.org/index.php?target=ipod6g)
3. extract it
4. drag the .rockbox folder onto your device

### List of 360p Themes

- Theme Pack with included Fonts: https://github.com/rockbox-y1/rockbox/releases/tag/360p-theme-pack
  - BONES
  - iClassic_v1.2 (based on https://old.reddit.com/r/innioasis/comments/1n9envr/iclassic_theme_for_rockbox_now_in_240p_and_360p/)
  - iMMXX
  - InfoMatrix
  - MacClassic
  - PodOne
  - PodTwo
  - SNAZZ
  - SNAZZ2
  - SNAZZ3
  - SPAZZ
- ipodmod3blk-y1: https://github.com/AkikoKumagara/ipodmod3blk-y1

### List of working Themes (240p)

There are likely more but these are tested.

- CenterArt
- FreshOSInstall (needs manual steps)
- Horizon
- iLike
- OneBit_OLED
- OneBit_Mono
- OneBit_VFD_ALT
- OP_1
- Orbit
- SKIDMARK (artefacting in menus)
- SNAZZ2 (artefacting in menus)
- SNAZZ3 (artefacting in menus)
- InfoMatrix
- naranjada
- PodOne
- Redux
- Themify
- Win95
- xplorr

## Installation
### Innioasis Updater (recommended)

This is a multi-platform community-built tool for updating official firmware and installing custom firmware (which often includes this Rockbox port). 

The Innioasis Updater was developed independently by TeamSlide without input from Innioasis/Timmkoo.

You can find it here: https://innioasis.app/

The firmware image included in the releases is based on multiwirth's firmware: https://github.com/multiwirth/Innioasis_Y1_system_dumps/releases/tag/custom_wifi_type_a_v0.2

### SPFlash Tool

1. Download the latest Rockbox included firmware here: https://innioasis.app/firmware.html
2. Unpack the archive
3. Install SP Flash Tool v5.1904:
  - Windows: included in the archive, alternatively: https://spflashtools.com/windows/sp-flash-tool-v5-1904
  - Linux: https://spflashtools.com/linux/sp-flash-tool-v5-1904-for-linux
4. Start SP Flash Tool
5. Follow the official firmware flashing instructions but use the MT6572_Android_scatter.txt from the Project Gallagher archive

### Manual installation (for developers)

If, despite all warnings, you still want to try installing it manually you need to do the following:

- have an Innioasis Y1 with ADB enabled
- root the device (see https://xdaforums.com/t/root-framaroot-a-one-click-apk-to-root-some-devices.2130276/):
```
wget https://supersuroot.org/downloads/supersu-2-82.apk
adb install supersu-2-82.apk
adb install Framaroot-1.9.3.apk
adb shell monkey -p com.alephzain.framaroot -c android.intent.category.LAUNCHER 1
# wait for it to start
adb shell input keyevent DPAD_DOWN
adb shell input keyevent DPAD_CENTER
# wait for the success message
adb shell monkey -p eu.chainfire.supersu -c android.intent.category.LAUNCHER 1
# follow instructions, once done
adb reboot
```
- Reboot device
```
adb reboot
```
- Update keymap file to be able to navigate systems menus
```
adb shell mount -o rw,remount /system
adb pull /system/usr/keylayout/Generic.kl Generic.kl
cp Generic.kl Generic.kl_backup
sed -i '/key 103   DPAD_UP/c\key 105   DPAD_UP' Generic.kl
sed -i '/key 108   DPAD_DOWN/c\key 106   DPAD_DOWN' Generic.kl
sed -i '/key 105   DPAD_LEFT/c\key 103   MEDIA_PREVIOUS' Generic.kl
sed -i '/key 106   DPAD_RIGHT/c\key 108   MEDIA_NEXT' Generic.kl
sed -i '/key 163   MEDIA_NEXT/c\key 163   DPAD_RIGHT' Generic.kl
sed -i '/key 165   MEDIA_PREVIOUS/c\key 165   DPAD_LEFT' Generic.kl
adb push Generic.kl /system/usr/keylayout/Generic.kl
adb shell chmod 644 /system/usr/keylayout/Generic.kl
adb reboot
```
- Download the latest Rockbox release APK from the sidebar
- Download the latest Rockbox libs.tar.gz and unpack it (`tar -xzf libs.tar.gz`) such that there is a folder called `libs/armeabi`
- Either use one of the preinstalled themes or supply your own in the .rockbox folder on the SD card
- Uninstall any apps you do not want
```
# list packages
adb shell pm list packages
# uninstall package
adb uninstall <package>
# or if that fails
adb shell pm disable-user <package>
```
- Uninstall old Rockbox version (can be skipped if v0.1 was never installed)
```
adb uninstall org.rockbox
```
- Install rockbox as system app:
```
adb remount
adb push rockbox-[release].apk /system/app/org.rockbox.apk
adb shell chmod 644 /system/app/org.rockbox.apk
adb shell chown root:root /system/app/org.rockbox.apk
adb push libs/armeabi/librockbox.so /system/lib/
adb shell chmod 644 /system/lib/librockbox.so
adb shell chown root:root /system/lib/librockbox.so
adb shell rm -rf /data/data/org.rockbox/lib
adb shell mkdir /data/data/org.rockbox/lib
adb push libs/armeabi/* /data/data/org.rockbox/lib/.
```
- Restart the device, choose Rockbox as launcher when asked
```
adb reboot
```
- (optional) Download the voice pack from the releases, extract it, drag the .rockbox folder onto your device
- **If rockbox gets stuck at the Rockbox logo and doesn't load your theme please delete .rockbox/config.cfg on your SD card**

## How to restart the app

When you initialize the database Rockbox will ask you to restart. You can do this via `Main Menu > System > Restart Rockbox (last option in list)`.

## Known issues

- Setting different theme might need a restart of Rockbox (Main Menu > System > Restart Rockbox) or clearing the backdrop (Theme Settings)
- Rockbox might randomly crash (usually recovers on its own now) - restart your device or Rockbox via:
```
adb shell monkey -p org.rockbox -c android.intent.category.LAUNCHER 1
```

## Planned

Ordered by priority

### Unknown/Long-term
#### Connectivity

- Fetch podcasts via rss
