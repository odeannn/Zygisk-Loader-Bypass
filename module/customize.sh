SKIPUNZIP=1

ui_print "- Extracting module files"
unzip -o "$ZIPFILE" 'module.prop' -d $MODPATH >&2
unzip -o "$ZIPFILE" 'zygisk/*' -d $MODPATH >&2

# Clean up
rm -rf $MODPATH/zygisk/arm64-v8a
rm -rf $MODPATH/zygisk/armeabi-v7a
rm -rf $MODPATH/zygisk/x86
rm -rf $MODPATH/zygisk/x86_64

# Rename to correct architecture
ui_print "- Installing for architecture: $ARCH"
if [ "$ARCH" = "arm" ]; then
    mv $MODPATH/zygisk/armeabi-v7a $MODPATH/zygisk/$ARCH
elif [ "$ARCH" = "arm64" ]; then
    mv $MODPATH/zygisk/arm64-v8a $MODPATH/zygisk/$ARCH
elif [ "$ARCH" = "x86" ]; then
    mv $MODPATH/zygisk/x86 $MODPATH/zygisk/$ARCH
elif [ "$ARCH" = "x64" ]; then
    mv $MODPATH/zygisk/x86_64 $MODPATH/zygisk/x86_64
fi

set_perm_recursive $MODPATH 0 0 0755 0644
