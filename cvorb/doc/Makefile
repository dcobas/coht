DOCHOST = cs-ccr-www3.cern.ch
DOCPATH = /var/www/html/private/coht/doxy
DOXYCFG = default.doxycfg
MODULE = cvorbn

all: clean $(DOXYCFG)
	sh doxy.sh -n "CVORB user library API" -o "." ../include/cvorb.h ../lib/cvorb.c

$(DOXYCFG):
	doxygen -s -g > /dev/null
	mv Doxyfile $@
clean:
	rm -rf html latex man $(DOXYCFG)

doxy_install: all
	-ssh -t $(DOCHOST) mv $(DOCPATH)/$(MODULE) $(DOCPATH)/$(MODULE).old
	rsync -a html/ $(DOCHOST):$(DOCPATH)/$(MODULE)
	ssh -t $(DOCHOST) rm -rf $(DOCPATH)/$(MODULE).old
