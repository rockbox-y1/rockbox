echo "++++ Starting system update"

mount -o rw,remount /system
echo "++ Remounted /system"

cd /sdcard/.rockbox/
unzip update.zip
cd update
echo "++ Unpacked update"

cp /sdcard/.rockbox/update/libs/armeabi/librockbox.so /system/lib/
chmod 644 /system/lib/librockbox.so 
chown root:root /system/lib/librockbox.so
rm -rf /data/data/org.rockbox/lib
mkdir /data/data/org.rockbox/lib
for f in /sdcard/.rockbox/update/libs/armeabi/*; do
  cp "$f" /data/data/org.rockbox/lib/
done
chmod 777 /data/data/org.rockbox/lib/*
unzip libs/armeabi/libmisc.so
for f in .rockbox/*; do
  cp -rf "$f" /sdcard/.rockbox/
done
echo "++ Updated libraries"

rm /system/app/org.rockbox.apk
cp /sdcard/.rockbox/update/rockbox.apk /system/app/org.rockbox.apk
chmod 644 /system/app/org.rockbox.apk
chown root:root /system/app/org.rockbox.apk
echo "++ Updated rockbox apk"

#cp com.innioasis.y1_*.apk /data/app/com.innioasis.y1-1.apk
#echo "++ Updated stock apk"

cp /sdcard/.rockbox/update/install-recovery.sh /system/etc/install-recovery.sh
chmod 755 /system/etc/install-recovery.sh
mkdir -p /system/etc/init.d
cp /sdcard/.rockbox/update/99Y1ButtonScript /system/etc/init.d/
chmod 755 /system/etc/init.d/99Y1ButtonScript
cp /sdcard/.rockbox/update/switch-to-stock.sh /data/data/
cp /sdcard/.rockbox/update/gocurl /data/data/
chmod 755 /data/data/gocurl
cp /sdcard/.rockbox/update/poddl /data/data/
chmod 755 /data/data/poddl
cp /sdcard/.rockbox/update/update.sh /data/data/update/update.sh
echo "++ Updated scripts"

cp /sdcard/.rockbox/update/Rockbox.kl /system/usr/keylayout/Rockbox.kl
cp /sdcard/.rockbox/update/Stock.kl /system/usr/keylayout/Stock.kl
cp /sdcard/.rockbox/update/Rockbox.kl /system/usr/keylayout/Generic.kl
echo "++ Updated keylayouts"

cd /sdcard/.rockbox
rm -rf update/
echo "++ Cleaned up"

echo "++++ Finished update."
reboot
