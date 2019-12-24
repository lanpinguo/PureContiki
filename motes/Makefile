

PWD=$(realpath ./)

ifeq ($(RM),)
export RM=rm
endif

ifeq ($(SED),)
export SED=sed
endif

ifeq ($(MAKE),)
export MAKE=make
endif


.PHONY:all clean PureSwitch
all: PureSwitch 



PureSwitch:
	$(MAKE) -C PureSwitch/ 

PureCtrlPanel:
	$(MAKE) -C libs/I2Cdev


install:
	$(INSTALL) -m 755 $(PROG) $(DESTDIR)/usr/local/bin/

clean:
	$(MAKE) -C PureSwitch clean



