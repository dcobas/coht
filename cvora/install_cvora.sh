#!/bin/sh

MODULE_NAME="CVORA-TMP"
DRIVER_NAME="cvora"

echo "Installing $MODULE_NAME driver..."

INSMOD_ARGS=`awk -f transfer2insmod.awk $MODULE_NAME /etc/transfer.ref`

if [ x"$INSMOD_ARGS" == x"" ] ; then
    echo "No $MODULE_NAME declared in /etc/transfer.ref, exiting"
    exit 1
fi

INSMOD_CMD="insmod $DRIVER_NAME.ko $INSMOD_ARGS"
echo "installing $DRIVER_NAME by insmod cvora $INSMOD_ARGS"
sh -c "$INSMOD_CMD"

MAJOR=`cat /proc/devices | awk '$2 == "'$DRIVER_NAME'" {print $1}'`
if [ -z "$MAJOR" ]; then
	echo "driver $DRIVER_NAME not installed!"
	exit 1
fi

MINORS=`awk '/^#\+#/ && $6 == "'$MODULE_NAME'" { printf("%s ", $7) }' /etc/transfer.ref`
echo "creating device nodes for driver $DRIVER_NAME, major $MAJOR, minors $MINORS"
for MINOR in $MINORS; do
    rm -f /dev/cvora.$MINOR
    mknod /dev/cvora.$MINOR c $MAJOR $MINOR
done
