include ../Makefile.base

H_INST_DIR = Cvorb
INSTDIR    = $(UTILINSTDIR)/include

_deliver:
	@ umask 0002; \
	if [ -d $(INSTDIR)/$(H_INST_DIR) ]; then \
		if [ -d $(INSTDIR)/$(H_INST_DIR)/previousV ]; then \
			rm -r $(INSTDIR)/$(H_INST_DIR)/previousV; \
		fi; \
		mkdir -p $(INSTDIR)/$(H_INST_DIR)/previousV; \
		echo "$(H_INST_DIR) include directory already exist."; \
		echo "Saving previous header files..."; \
		$(CP) --preserve=mode,timestamps $(INSTDIR)/$(H_INST_DIR)/*.h $(INSTDIR)/$(H_INST_DIR)/previousV; \
		$(CP) -r --preserve=mode,timestamps $(INSTDIR)/$(H_INST_DIR)/user $(INSTDIR)/$(H_INST_DIR)/previousV; \
		rm $(INSTDIR)/$(H_INST_DIR)/*.h; \
		rm -r $(INSTDIR)/$(H_INST_DIR)/user; \
	else \
		echo "Creating $(H_INST_DIR) include directory..."; \
		mkdir -p $(INSTDIR)/$(H_INST_DIR); \
	fi; \
	echo "Copying new header files to '$(INSTDIR)/$(H_INST_DIR)' directory..."; \
	$(CP) --preserve=mode,timestamps *.h $(INSTDIR)/$(H_INST_DIR); \
	mkdir -p $(INSTDIR)/$(H_INST_DIR)/user; \
	$(CP) --preserve=mode,timestamps user/*.h $(INSTDIR)/$(H_INST_DIR)/user

deliver:
	$(Q)$(MAKE) _deliver CPU=L865
	$(Q)$(MAKE) _deliver CPU=ppc4

# quet down
linux lynx all:
	@: