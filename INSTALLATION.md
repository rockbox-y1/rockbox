# Installation and Updating
## Update Rockbox
1. Download the latest Rockbox `update_[360p|240p].zip` here: https://github.com/rockbox-y1/rockbox/releases
2. Connect your Y1 and copy `update_[360p|240p].zip` to `.rockbox/update.zip` (make sure it is renamed to `update.zip`)
3. Safely disconnect your Y1
4. Go to `Main Menu > System` and click `Firmware Update`
5. The update process will now run in the background and automatically restart the device once it is done

**Note:** If this is the first time you install Rockbox on your Y1, if you don't see the `Firmware Update` option in `Main Menu > System`, or if just want to have a fresh install follow the steps for `Full Installation` below.

## Full Installation
### MTKClient

1. Install MTKClient: https://github.com/bkerler/mtkclient
2. Download the latest Rockbox included firmware (rom_\[360p|240p\].zip) here: https://github.com/rockbox-y1/rockbox/releases/latest 
3. Unpack the archive:
```
mkdir rom && cd rom
unzip ../rom_[360p|240p].zip
```
4. Turn of the device, disconnect from the PC
5. Start the flashing process:
```
cd rom
python ../mtk.py w logo,uboot,bootimg,recovery,android,usrdata logo.bin,lk.bin,boot.img,recovery.img,system.img,userdata.img
```
6. Connect the device via USB
7. Unplug the device when the process has finished
8. Power on the device

### SPFlash Tool

1. Download the latest Rockbox included firmware (rom_\[360p|240p\].zip) here: https://github.com/rockbox-y1/rockbox/releases/latest
2. Unpack the archive
3. Install SP Flash Tool v5.1904: https://spflashtool.com/download/
4. Start SP Flash Tool
5. Follow the [official firmware flashing instructions](https://support.innioasis.com/download/flashing_tutorial/Flashing_tutorial-Y1_EN%20v2.0.7-20241021.pdf) but use the MT6572_Android_scatter.txt from the rom_\[360p|240p\].zip file

## Manual installation (for developers)

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
adb remount
adb push android/scripts/Rockbox.kl /system/usr/keylayout/Generic.kl Generic.kl
adb shell chmod 644 /system/usr/keylayout/Generic.kl
adb reboot
```
- Download the latest Rockbox release APK from the sidebar
- Download the latest Rockbox `update_\[360p|240p\].zip` and unpack it such that there is a folder called `update/libs/armeabi`
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
adb push update/libs/armeabi/librockbox.so /system/lib/
adb shell chmod 644 /system/lib/librockbox.so
adb shell chown root:root /system/lib/librockbox.so
adb shell rm -rf /data/data/org.rockbox/lib
adb shell mkdir /data/data/org.rockbox/lib
adb push update/libs/armeabi/* /data/data/org.rockbox/lib/.
```
- Restart the device, choose Rockbox as launcher when asked
```
adb reboot
```
