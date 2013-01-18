COHTDOXY=/acc/src/dsc/drivers/coht/doxy
DOCHOST = cs-ccr-www3.cern.ch
DOCPATH = /var/www/html/private/coht/doxy
MODULE = cvorbn

all:
	echo Make documentation for libcvorb support library
	rm -f ,*.h
	rm -rf html latex man 
	cp $(COHTDOXY)/default.doxycfg .
	sh doxy.sh -n "CVORB user library API" -o "." ../lib/cvorb.c

clean:
	rm -rf html latex man default.doxycfg

doxy_install: all
	-ssh -t $(DOCHOST) mv $(DOCPATH)/$(MODULE) $(DOCPATH)/$(MODULE).old
	rsync -a html/ $(DOCHOST):$(DOCPATH)/$(MODULE)
	ssh -t $(DOCHOST) rm -rf $(DOCPATH)/$(MODULE).old
