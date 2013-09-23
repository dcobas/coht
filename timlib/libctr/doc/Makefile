DOXY_INSTPATH = /acc/doc/html/private/coht/doxy/libctr

all:
	echo Make documentation for libctr shared object library
	./doxy.sh -n"libctr Timing Library" -o "doc" libctr.doxygen

doxy_install: all
	mkdir -p $(DOXY_INSTPATH)
	cp -r doc/html/* $(DOXY_INSTPATH)
