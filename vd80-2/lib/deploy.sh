#!	/bin/sh

if [ x"$CPU" = x"" ] ; then
	echo "please specify CPU=<arch>"
fi

dsc_install	\
	../driver/vd80.h \
	libvd80.h \
	libvd80.$(CPU).a \
	/acc/local/$(CPU)/drv/vd80-2.0
