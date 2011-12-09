#!	/bin/sh

CPU=L865
KVER=2.6.24.7-rt27

INSTALL_SCRIPTS=ctr[pv]install
MODULES=${CPU}/${KVER}/ctr[pv].ko 
SYMLINKS=n
DEST=./ctr-`date "+%Y%m%d%H%M%S"`

OUTPUT=":"
RUN=""

while getopts sd:hvn o
do	case $o in
	v)	OUTPUT="echo" ;;		# verbose
	n)	RUN=":" OUTPUT="echo" ;;	# dry run
	s)	SYMLINKS=y ;;
	d)	DEST=$OPTARG ;;
	[h?])	echo >&2 "usage: $0 [-?hvnb] [-D device] [-d driver] [-t transfer]"
		echo >&2 "	-h, -?		: help"
		echo >&2 "	-v		: verbose"
		echo >&2 "	-n		: dry-run, do not do anything (implies -v)"
		echo >&2 "	-s 		: create symlinks"
		echo >&2 "	-d dest		: delivery destination (default: ./ctr-<timestamp>)"
		exit ;;
	esac
done

mkdir -p $DEST
chmod +x $INSTALL_SCRIPTS
dsc_install $INSTALL_SCRIPTS $MODULES $DEST

if [ x"$SYMLINKS" == x"y" ] ; then 
    ( cd $DEST 
    ln -sf ctrpinstall ctrinstall 
    ln -sf ../../ctr/Vhdl.versions . 
    ln -sf ../../ctr/ctrtest.config . )
fi
