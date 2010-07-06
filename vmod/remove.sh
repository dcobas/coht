#!	/bin/sh

for mod in vmod12e16 mod_pci modulbus_register ; do
    if cat /proc/modules | grep -q $mod  ; then
    	rmmod $mod
    fi
done

