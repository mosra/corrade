#!/bin/bash
set -e

if [ "$(adb get-state | tr -d '\r\n')" != "device" ]; then
    echo "ERROR: no device connected"
    exit 1
fi

adb push $1 /data/local/tmp
# No comment. https://code.google.com/p/android/issues/detail?id=3254
adb shell '/data/local/tmp/'$2'; echo -n ADB_IS_SHIT:$?' | tee /tmp/adb.retval | grep -v ADB_IS_SHIT
exit $(grep ADB_IS_SHIT /tmp/adb.retval | cut -d':' -f2)
