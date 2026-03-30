#!/system/bin/sh
# Innioasis Y1 script
# Functionality: Switch to Stock Firmware/App

keymap_stock="/system/usr/keylayout/Stock.kl"
keymap="/system/usr/keylayout/Generic.kl"

echo "Setting up Stock Keymap."
mount -o remount,rw /system
cp "$keymap_stock" "$keymap"
echo "Disabling Rockbox"
pm disable org.rockbox
echo "Enabling Stock"
pm enable com.innioasis.y1
echo "Rebooting"
reboot