DOXY_INSTPATH = /acc/doc/html/private/coht/doxy/vd80-2.0

all:
	echo Make documentation for linux VD80 library
	rm -f ,*.h
	rm -rf doc
	doxygen Doxyfile

doxy_install: all
	mkdir -p $(DOXY_INSTPATH)
	cp -r doc/html/* $(DOXY_INSTPATH)/
