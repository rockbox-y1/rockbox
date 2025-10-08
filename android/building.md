
# Building on Windows

Your best options are to use either <u>WSL</u>, <u>Docker</u>, or a <u>Linux VM</u>

# Building on Linux

## Prerequisites

- Android Platform Tools
- GCC
- Make
- openjdk-17-jdk

1. With a copy of https://github.com/rockbox-y1/rockbox.git navigate to `android/`

2. Run `./installToolchain.sh`

3. Export env vars

   ```
   export ANDROID_SDK_PATH=$HOME/android-sdk
   export ANDROID_NDK_PATH=$HOME/android-ndk-r10e
   ```

4. Create the build directory and enter it `mkdir build && cd build`

5. Run the configure tool for android:

   ```
   ../../tools/configure --target=201 --lcdwidth=480 --lcdheight=360 --type=n
   ```

6. Run `make`

7. Run `make classes`

8. Run `make zip`

9. Run `make unsigned-apk`

10. CD to the root directory, and run signer:

    ```bash
    APKSIGNER=""
    if command -v apksigner >/dev/null 2>&1; then
        APKSIGNER="apksigner"
    elif [ -n "$ANDROID_SDK_PATH" ] && [ -f "$ANDROID_SDK_PATH/build-tools/$(ls -1 "$ANDROID_SDK_PATH/build-tools" | tail -1)/apksigner" ]; then
        APKSIGNER="$ANDROID_SDK_PATH/build-tools/$(ls -1 "$ANDROID_SDK_PATH/build-tools" | tail -1)/apksigner"
    fi
    cd android/build
    $APKSIGNER sign --key ../platform.pk8 --cert ../platform.x509.pem rockbox_unsigned.apk
    cp rockbox_unsigned.apk rockbox.apk
    ```

11. Install it the apk with `adb install -r rockbox.apk`

---

# Building on Mac (Arm)

## Prerequisites

- Android Platform Tools: (`brew install --cask android-platform-tools`)
- GCC
- Make
- openjdk-17-jdk (`brew install openjdk@17`)

1. With a copy of https://github.com/rockbox-y1/rockbox.git navigate to `android/`

2. Run `./installToolchain.sh`

3. Export env vars

   ```
   export ANDROID_SDK_PATH=$HOME/android-sdk
   export ANDROID_NDK_PATH=$HOME/android-ndk-r10e
   ```

4. Create the build directory and enter it `mkdir build && cd build`

5. Run the configure tool for android:

   ```
   ../../tools/configure --target=201 --lcdwidth=480 --lcdheight=360 --type=n
   ```

6. Install Android build tools 28.0.3 for android-17

   This is the last supported version of build tools that support make based builds, it also has support for Mac Silicon

   ```
   sdkmanager "build-tools;28.0.3" "platforms;android-17"
   ```

7. Sym link 19.1.0 build tools to the 28.0.3 version

   ```
   mv $ANDROID_SDK_PATH/build-tools/19.1.0/aapt $ANDROID_SDK_PATH/build-tools/19.1.0/aapt.bak
   ln -s $ANDROID_SDK_PATH/build-tools/28.0.3/aapt $ANDROID_SDK_PATH/build-tools/19.1.0/aapt
   ```

8. Run `make`

9. Switch java versions to 17 `export JAVA_HOME="/opt/homebrew/opt/openjdk@17/libexec/openjdk.jdk/Contents/Home"`

10. Run `make classes`

11. Run `make zip`

12. Run `make unsigned-apk`

13. CD to the root directory, and run signer:

    ```bash
    APKSIGNER=""
    if command -v apksigner >/dev/null 2>&1; then
        APKSIGNER="apksigner"
    elif [ -n "$ANDROID_SDK_PATH" ] && [ -f "$ANDROID_SDK_PATH/build-tools/$(ls -1 "$ANDROID_SDK_PATH/build-tools" | tail -1)/apksigner" ]; then
        APKSIGNER="$ANDROID_SDK_PATH/build-tools/$(ls -1 "$ANDROID_SDK_PATH/build-tools" | tail -1)/apksigner"
    fi
    cd android/build
    $APKSIGNER sign --key ../platform.pk8 --cert ../platform.x509.pem rockbox_unsigned.apk
    cp rockbox_unsigned.apk rockbox.apk
    ```

14. Install it the apk with `adb install -r rockbox.apk`
