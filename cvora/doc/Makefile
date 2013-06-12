COHTDOXY=/acc/src/dsc/drivers/coht/doxy

all:
	echo Make documentation for libcvora support library
	rm -f ,*.h
	rm -rf html latex man 
	cp $(COHTDOXY)/default.doxycfg .
	sh doxy.sh -n "CVORA user library API" -o "." ../libcvora.h

clean:
	rm -rf html latex man default.doxycfg

doxy_install: all
	mkdir -p $(DOXY_INSTPATH)
	cp -r doc/html/* $(DOXY_INSTPATH)
