# Patching com.innioasis.y1 for Innnioasis ATA ROMs 

## Problem

The Innioasis Y1 stock app (`com.innioasis.y1`) crashes on start because it tries to write shared preferences to `/proc/tpd_keys_enable`, a kernel proc entry that only exists on the Stock ROM. The ATA ROM's kernel doesn't include this entry.

The crash happens in `SharedPreferencesUtils.kt` during `Y1Application.onCreate()`:
```
java.io.FileNotFoundException: /proc/tpd_keys_enable: open failed: ENOENT
```

## Solution

Redirect the app's writes from `/proc/tpd_keys_enable` to a writable file path (e.g., `/data/local/tmp/tpd_keys_enable`).

## Prerequisites

- `apktool`
- `baksmali` (for disassembling DEX files)
- `smali` (for assembling DEX files)
- `apksigner` (for signing APKs)
- Platform signing keys: `platform.pk8` and `platform.x509.pem`

## Step-by-Step

### 1. Extract DEX files from the new APK

```bash
cd /tmp
rm -rf y1_patch && mkdir y1_patch && cd y1_patch

# Extract the APK
unzip -o /path/to/new/com.innioasis.y1_X.X.X.apk -d original_apk

# Extract DEX files
mkdir -p dex_out
cp original_apk/classes.dex dex_out/
cp original_apk/classes2.dex dex_out/
```

### 2. Find the problematic class

The app uses `SharedPreferencesUtils` to access `/proc/tpd_keys_enable`.
Find which DEX file contains it:

```bash
# Check classes.dex
baksmali list dex_out/classes.dex 2>/dev/null | grep SharedPreferencesUtils
# or check classes2.dex
baksmali list dex_out/classes2.dex 2>/dev/null | grep SharedPreferencesUtils
```

### 3. Disassemble the class

Replace `classesX.dex` with whichever file contains `SharedPreferencesUtils`:

```bash
baksmali disassemble -o smali_out/ dex_out/classesX.dex
```

### 4. Edit the smali file

Open `smali_out/com/innioasis/y1/utils/SharedPreferencesUtils.smali`.

Find all occurrences of:
```
const-string v1, "/proc/tpd_keys_enable"
```

Replace with:
```
const-string v1, "/data/local/tmp/tpd_keys_enable"
```

**Note:** All occurrences must be replaced (currently one in `setKeyLock()`, one in `isKeyLock()`).

### 5. Reassemble the DEX

```bash
smali assemble smali_out/ -o classes_patched.dex
```

### 6. Rebuild the APK

```bash
# Replace the original DEX with the patched one
cp original_apk/classes.dex classes_backup.dex
cp classes_patched.dex original_apk/classes.dex

# Rebuild the APK (adjust if the DEX was in classes2.dex instead)
cd original_apk
rm -f ../classes_patched.apk
zip -r ../classes_patched.apk *

cd ..
```

### 7. Sign the APK

```bash
apksigner sign \
    --key /path/to/platform.pk8 \
    --cert /path/to/platform.x509.pem \
    classes_patched.apk
```

### 8. Verify

```bash
apksigner verify --verbose classes_patched.apk
# Should show: Verified using v1/v2/v3 scheme: true
```

### 9. Install

```bash
adb install classes_patched.apk
```

## Notes

- The path `/data/local/tmp/tpd_keys_enable` is writable on the Rockbox ROM
- If a newer version moves `SharedPreferencesUtils` to a different DEX file,
  adjust step 6 accordingly (replace `classes2.dex` instead of `classes.dex`)
- If the new version changes the file path string, search for the new path in the
  smali output and replace it similarly

## Troubleshooting

### "DexOpt failed" / "unable to open DEX file"
- Make sure you're replacing the correct DEX file (classes.dex vs classes2.dex)
- Verify the smali assembled without errors
- Check that the APK structure is intact (zip is valid)

### App still crashes
- Search for all occurrences of the old path: `grep -r "proc/tpd_keys_enable" smali_out/`
- There may be additional references in other classes
