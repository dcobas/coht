# specific delivery of waveform files
_deliver:
	@echo -en "\nDelivering waveform files in $(DDIR) -- "
	@if [ ! -e $(DDIR) ]; then mkdir $(DDIR); fi
	@if [ ! -e $(DDIR)/vtf ]; then mkdir $(DDIR)/vtf; fi
	@cp -r --preserve=mode,timestamps vtf/*.vtf $(DDIR)/vtf
	@echo -e "Done"
