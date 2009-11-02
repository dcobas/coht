###############################################################################
# @file deliver.mk
#
# @brief CERN delivery
#
# @author Copyright (C) 2009 CERN. Yury GEORGIEVSKIY <ygeorgie@cern.ch>
#
# @date Created on 25/08/2009
###############################################################################

wrong-dest := $(filter-out $(strip $(ACCS)) _deliver,$(MAKECMDGOALS))
__destination = $(filter $(strip $(ACCS)),$(MAKECMDGOALS))

# Don't allow make deliver (without explicit destination $(ACCS))
#destination := $(if $(__destination), $(__destination), $(ACCS))

destination := $(if $(__destination), $(__destination), $(ACCS))

# quiet down $(ACCS)
$(destination):
	@:

ifeq ($(notdir $(shell pwd)), ins_rm_mod)
_deliver:
	$(if $(wrong-dest), \
		$(error wrong delivery place(s) "$(wrong-dest)"),)
	$(if $(destination),, \
		$(error you should explicitly specify destination "$(ACCS)"))
	@for a in $(destination); do \
		tmp_dir="/acc/dsc/$$a/$(CPU)/bin"; \
		if [ -w $$tmp_dir ]; then \
			echo -en "\nDelivering [dgmodinst] && [dgmoduninst] in $$tmp_dir -- "; \
			dsc_install dgmodinst.$(CPU) $$tmp_dir; \
			sudo chown root.root $$tmp_dir/dgmodinst; \
			sudo chmod 6555 $$tmp_dir/dgmodinst; \
			\
			dsc_install dgmoduninst.$(CPU) $$tmp_dir; \
			sudo chown root.root $$tmp_dir/dgmoduninst; \
			sudo chmod 6555 $$tmp_dir/dgmoduninst; \
			echo -e "Done"; \
		fi; \
	done
	@echo ""
else ifeq ($(notdir $(shell pwd)), driver)
include ./makefiles/drvr-paramount.mk
_deliver:
	$(if $(wrong-dest), \
		$(error wrong delivery place(s) "$(wrong-dest)"),)
	$(if $(destination),, \
		$(error you should explicitly specify destination "$(ACCS)"))
	@for a in $(destination); do \
		tmp_dir="/acc/dsc/$$a/$(CPU)/$(KVER)/$(DRIVER_NAME)"; \
		if [ -w $$tmp_dir ]; then \
			echo -e "\nDelivering $(notdir $(DRIVER)) in $$tmp_dir "; \
			dsc_install $(DRIVER) $$tmp_dir; \
			\
			echo -e "\nDelivering $(notdir $(SIMULATOR)) in $$tmp_dir "; \
			dsc_install $(SIMULATOR) $$tmp_dir; \
			\
			echo -e "\nDelivering $(DRIVER_NAME).info in $$tmp_dir "; \
			dsc_install ../$(FINAL_DEST)/$(DRIVER_NAME).info $$tmp_dir; \
		elif [ ! -e $$tmp_dir ]; then \
			echo -e "\nCreating $$tmp_dir directory... "; \
			mkdir $$tmp_dir; \
			echo -e "Delivering $(notdir $(DRIVER)) in $$tmp_dir "; \
			dsc_install $(DRIVER) $$tmp_dir; \
			\
			echo -e "\nDelivering $(notdir $(SIMULATOR)) in $$tmp_dir "; \
			dsc_install $(SIMULATOR) $$tmp_dir; \
			\
			echo -e "\nDelivering $(DRIVER_NAME).info in $$tmp_dir "; \
			dsc_install ../$(FINAL_DEST)/$(DRIVER_NAME).info $$tmp_dir; \
		elif [ -e $$tmp_dir ]; then \
			echo -e "\nCan't deliver $(notdir $(DRIVER_NAME)) in $$tmp_dir"; \
			echo -e "You don't have write permissions!"; \
		fi; \
	done
	@echo ""
else ifeq ($(notdir $(shell pwd)), test)
_deliver:
	$(if $(wrong-dest), \
		$(error wrong delivery place(s) "$(wrong-dest)"),)
	$(if $(destination),, \
		$(error you should explicitly specify destination "$(ACCS)"))
	@for a in $(destination); do \
		tmp_dir="/acc/dsc/$$a/$(CPU)/bin"; \
		if [ -w $$tmp_dir ]; then \
			echo -en "\nDelivering $(EXEC_OBJS) in $$tmp_dir -- "; \
			dsc_install $(FINAL_DEST)/$(EXEC_OBJS) $$tmp_dir; \
			echo -e "Done"; \
		fi; \
	done
	@echo ""
else ifeq ($(notdir $(shell pwd)), lib)
_deliver:
	@ umask 0002; \
	if [ -d $(INSTDIR)/$(DRVR) ]; then \
		if [ -d $(INSTDIR)/$(DRVR)/previousV ]; then \
			rm -rf $(INSTDIR)/$(DRVR)/previousV; \
		fi; \
		mkdir -p $(INSTDIR)/$(DRVR)/previousV; \
		echo "$(INSTDIR)/$(DRVR) include directory already exist."; \
		echo "Saving current libraries..."; \
		$(CP) --preserve=mode,timestamps $(INSTDIR)/$(DRVR)/*.a $(INSTDIR)/$(DRVR)/previousV; \
		rm -f $(INSTDIR)/$(DRVR)/*.a; \
	else \
		echo "Creating $(INSTDIR)/$(DRVR) lib directory..."; \
		mkdir -p $(INSTDIR)/$(DRVR); \
	fi; \
	echo "Copying new $(LIBRARIES) libraries to '$(INSTDIR)/$(DRVR)' directory..."; \
	dsc_install $(addprefix $(FINAL_DEST)/, $(LIBRARIES)) $(INSTDIR)/$(DRVR)
endif