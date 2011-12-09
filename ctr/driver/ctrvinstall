#!/bin/sh

DEVICE_NAME=CTRV
DRIVER_NAME=ctrv
DRIVER_PATH=../$DRIVER_NAME/$DRIVER_NAME-20111209.ko
DEVNAME=ctr
TRANSFER=/etc/transfer.ref
MAKEDEVS="y"

OUTPUT=":"
RUN=""
LUN=0

if [ x"$1" == x"-object" -a x"$2" == x"ctrvdrvr.o" ] ; then
	shift 2
fi

while getopts hvnbD:d:t:M:X:V:L: o
do	case $o in
	v)	OUTPUT="echo" ;;		# verbose
	n)	RUN=":" OUTPUT="echo" ;;	# dry run
	b)	MAKEDEVS="n" ;;			# omit devnode creation
	D)	DEVICE_NAME="$OPTARG" ;;
	d)	DRIVER_NAME="$OPTARG" ;;
	t)	TRANSFER="$OPTARG" ;;
	M)	LUN=`expr $LUN + 1`
		base_address1="$base_address1,$OPTARG"
		# transfer_lun=`awk '/^#\+#/ && $4 == "VME" && $6 == "CTRV" && "0x" $11 == "'$OPTARG'" { print $7 + 1 }' $TRANSFER`
		# if [ x"$transfer_lun" != x"$LUN" ] ; then
		# echo $transfer_lun $LUN
		# echo >&2 "ctrvinstall: WARNING: possible misassignment of LUN in CTRV #"$LUN
		# LUN=$transfer_lun
		# fi
		luns="$luns,$LUN" ;;
	X)	base_address2="$base_address2,$OPTARG" ;;
	V)	vector="$vector,$OPTARG";;
	L)	level="$level,$OPTARG";;
	[h?])	echo >&2 "usage: $0 [-?hvnb] [-D device] [-d driver] [-t transfer]"
		echo >&2 "	-h, -?		: help"
		echo >&2 "	-v		: verbose"
		echo >&2 "	-n		: dry-run, do not do anything (implies -v)"
		echo >&2 "	-b		: do not make device nodes"
		echo >&2 "	-D device	: hardware device name (e.g. VD80)"
		echo >&2 "	-d device	: driver name (e.g. vd80)"
		echo >&2 "	-t transfer	: transfer.ref file to use (default: /etc/transfer.ref)"
		exit ;;
	esac
done

# remove leading comma
luns=`echo $luns | sed 's!,!!'`
base_address1=`echo $base_address1 | sed 's!,!!'`
base_address2=`echo $base_address2 | sed 's!,!!'`
vector=`echo $vector | sed 's!,!!'`

$OUTPUT "Installing $DEVICE_NAME driver..."
INSMOD_ARGS="luns=$luns vme1=$base_address1 vme2=$base_address2 vecs=$vector"
if [ x"$luns" == x"" ] ; then
    echo >&2 "ctrvinstall: No $DEVICE_NAME declared in $TRANSFER, exiting"
    exit 1
fi

INSMOD_CMD="insmod $DRIVER_PATH $INSMOD_ARGS"
$OUTPUT installing $DRIVER_NAME by $INSMOD_CMD
sh -c "$RUN $INSMOD_CMD"

if [ x"$MAKEDEVS" == x"n" ] ; then
    exit 0
fi

MAJOR=`cat /proc/devices | awk '$2 == "'"$DRIVER_NAME"'" {print $1}'`
MINORS=`seq -s' ' 0 16`
$OUTPUT "creating device nodes /dev/$DEVNAME.* for driver $DRIVER_NAME, major $MAJOR, minors $MINORS"

if [ -z "$MAJOR" ]; then
	echo >&2 "ctrvinstall: driver $DRIVER_NAME not installed!"
	echo >&2 "ctrvinstall: command line [$INSMOD_CMD]"
	exit 1
fi
for MINOR in $MINORS; do
    sh -c "$RUN rm -f /dev/$DEVNAME.$MINOR"
    sh -c "$RUN mknod /dev/$DEVNAME.$MINOR c $MAJOR $MINOR"
done
