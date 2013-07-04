#!/bin/sh

DEVICE_NAME=VD80
TRANSFER=/etc/transfer.ref
DRIVER_NAME=vd80
DEVNODE_NAME=$DRIVER_NAME

# Install the vd80 driver specific script

OUTPUT=":"
RUN=""
while getopts hvn2D:d:t: o
do	case $o in
	v)	OUTPUT="echo" ;;		# verbose
	n)	RUN=":" ;;			# dry run
	2)	DRIVER_NAME=vd80-2 ;;
	D)	DEVICE_NAME="$OPTARG" ;;
	d)	DRIVER_NAME="$OPTARG" ;;
	t)	TRANSFER="$OPTARG" ;;
	[h?])	echo >&2 "usage: $0 [-?hvnb36] [-D device] [-d driver] [-t transfer]"
		exit ;;
	esac
done

$OUTPUT "Installing $DEVICE_NAME driver..."
PARSER=$DRIVER_NAME.awk
INSMOD_ARGS=`awk -f $PARSER $DEVICE_NAME $TRANSFER`
if [ x"$INSMOD_ARGS" == x"" ] ; then
    echo "No $DEVICE_NAME declared in $TRANSFER, exiting"
    exit 1
fi

INSMOD_CMD="/sbin/insmod $DRIVER_NAME.ko $INSMOD_ARGS"
$OUTPUT installing $DRIVER_NAME by $INSMOD_CMD
sh -c "$RUN $INSMOD_CMD"


MAJOR=`cat /proc/devices | awk '$2 == "'"$DRIVER_NAME"'" {print $1}'`
if [ -z "$MAJOR" ]; then
	echo "driver $DRIVER_NAME not installed!"
	exit 1
fi
MINORS=`awk '/^#\+#/ && $6 == "'"$DEVICE_NAME"'" { printf("%s ", $7) }' $TRANSFER`
$OUTPUT "creating device nodes for driver $DRIVER_NAME, major $MAJOR, minors $MINORS"
for MINOR in $MINORS; do
    sh -c "$RUN rm -f /dev/$DEVNODE_NAME.$MINOR"
    sh -c "$RUN mknod -m 777 /dev/$DEVNODE_NAME.$MINOR c $MAJOR $MINOR"
done
