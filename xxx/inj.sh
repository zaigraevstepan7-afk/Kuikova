#!/system/bin/sh
# xxx — Standoff 2 0.39.2 internal (AndKittyInjector)
# Push to /data/local/tmp/ together with AndKittyInjector + libxxx.so

PACKAGE=com.axlebolt.standoff2
DIR=/data/local/tmp

cd "$DIR" || exit 1
chmod 755 AndKittyInjector 2>/dev/null
chmod 644 libxxx.so 2>/dev/null

./AndKittyInjector --package "$PACKAGE" --libs libxxx.so --memfd --delay 2000000
