#================================================================
# Makefile to produce libctr timing library
#
# LDVER determins the directory name where a libctr.a version is stored
# SOVER is the major shared object version. This is hard coded in the stub library libctr.a
# SUBVER is the minor version on the shared object library
#
# SOVER MUST be two decimal numbers seperated by a point, its a float eg: 1.224
# SUBVER MUST be one single decimal number, its an integer eg: 99
# N.B These constants are supplied to the libctr code via the -D option at compile time
#
# Let SOVER=1.2 and SUBVER=99, then when installing the shared objects this is what happens
# 1) libctr.$(CPU).so is copied to the target directory as libctr.so.1.2.99
# 2) A symbolic link is made in target libctr.so.1.2 -> libctr.so.1.2.99
# 3) A second symbolic link is made in target libctr.so -> libctr.so.1.2
# The stub library libctr.a always uses the link libctr.so.1.2 in this case
#
# Let LDVER=1.2, in this case a directory libctr.1.2 is created and libctr.a and includes
# are stored there
#
# SO_TARGET is where libctr(p/v).so gets stored
# AH_TARGET is where libctr.a and its includes gets stored
#
#================================================================

CPU=L866
ACC=lab

SOVER=1.1
LDVER=1.1
SUBVER=1

SO_TARGET=/acc/dsc/$(ACC)/$(CPU)/ctr
AH_TARGET=/acc/local/$(CPU)/tim

include /acc/dsc/src/co/Make.auto

DRVR = ../../driver

CFLAGS= -g -Wall -I. -I$(DRVR) -fPIC -DLDVER=$(LDVER) -DSOVER=$(SOVER) -DSUBVER=$(SUBVER)

SRCS=libctr.c libctr.h libctrP.h libctrp.c libctrv.c

INSTFILES=libctr.$(CPU).a libctr.h libctrp.$(CPU).so libctrv.$(CPU).so

all:libctr.$(CPU).a libctrp.$(CPU).so libctrv.$(CPU).so

libctr.$(CPU).o: $(SRCS)

libctrp.$(CPU).o: libctrp.c libctr_common.c

libctrv.$(CPU).o: libctrv.c libctr_common.c

libctr.$(CPU).a: libctr.$(CPU).o
	-$(RM) $@
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

libctrp.$(CPU).so: libctrp.$(CPU).o
	-$(RM) $@
	$(CC) $(CFLAGS) -o $@ -shared $^

libctrv.$(CPU).so: libctrv.$(CPU).o
	-$(RM) $@
	$(CC) $(CFLAGS) -o $@ -shared $^

clean:
	rm -f libctr.$(CPU).o
	rm -f libctr.$(CPU).a
	rm -f libctrp.$(CPU).o
	rm -f libctrp.$(CPU).so
	rm -f libctrv.$(CPU).o
	rm -f libctrv.$(CPU).so

install:
	@echo -e "\nUse: install-all, install-lib, install-lab, install-oper, install-oplhc\n"

install-all: install-lib install-lab install-oper install-oplhc

install-lib: $(INSTFILES)
	@echo -e "\nInstalling libctr.a in " $(AH_TARGET)
	dsc_install libctr.$(CPU).a $(AH_TARGET)
	rm -rf $(AH_TARGET)/libctr.$(LDVER)
	mkdir $(AH_TARGET)/libctr.$(LDVER)
	@echo "Installing libctr.a and libctr.h in " $(AH_TARGET) "/libctr." $(LDVER)
	dsc_install libctr.$(CPU).a $(AH_TARGET)/libctr.$(LDVER)
	dsc_install libctr.h $(AH_TARGET)/libctr.$(LDVER)
	dsc_install $(DRVR)/*.h $(AH_TARGET)/libctr.$(LDVER)
	@echo -e "Done install-lib\n"

install-lab: $(INSTFILES)
	make install-so ACC=lab

install-oper: $(INSTFILES)
	make install-so ACC=oper

install-oplhc: $(INSTFILES)
	make install-so ACC=oplhc

install-so: $(INSTFILES)
	echo -e "\nInstalling libctrp.so and libctrv.so in " $(ACC)
	rm -f $(SO_TARGET)/libctrp.so.$(SOVER).$(SUBVER)
	rm -f $(SO_TARGET)/libctrv.so.$(SOVER).$(SUBVER)
	rm -f $(SO_TARGET)/libctrp.so.$(SOVER)
	rm -f $(SO_TARGET)/libctrv.so.$(SOVER)
	rm -f $(SO_TARGET)/libctrp.so
	rm -f $(SO_TARGET)/libctrv.so
	cp libctrp.$(CPU).so $(SO_TARGET)/libctrp.so.$(SOVER).$(SUBVER)
	cp libctrv.$(CPU).so $(SO_TARGET)/libctrv.so.$(SOVER).$(SUBVER)
	cd $(SO_TARGET); ln -s libctrp.so.$(SOVER).$(SUBVER) libctrp.so.$(SOVER)
	cd $(SO_TARGET); ln -s libctrv.so.$(SOVER).$(SUBVER) libctrv.so.$(SOVER)
	cd $(SO_TARGET); ln -s libctrp.so.$(SOVER) libctrp.so
	cd $(SO_TARGET); ln -s libctrv.so.$(SOVER) libctrv.so
	echo -e "Done install in " $(ACC)
