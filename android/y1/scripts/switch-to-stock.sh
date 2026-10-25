#!/system/bin/sh
# Innioasis Y1 script
# Functionality: Switch to Stock Firmware/App

echo "Disabling Rockbox"
pm disable org.rockbox
echo "Enabling Stock"
pm enable com.innioasis.y1