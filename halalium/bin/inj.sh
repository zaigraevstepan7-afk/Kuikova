#!/system/bin/sh

PACKAGE="com.axlebolt.standoff2"

# Путь к файлам
DIR="/data/local/tmp"
INJECTOR="$DIR/AndKittyInjector"
LIB="$DIR/libhalalium.so"


chmod 777 $INJECTOR
chmod 777 $LIB

$INJECTOR --package $PACKAGE --libs $LIB --memfd  --delay 2000000