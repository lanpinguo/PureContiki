

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


.PHONY:all clean PureSwitch PureCtrlPanel


all: PureSwitch PureCtrlPanel


PureSwitch:
	$(MAKE) -C PureSwitch/ 

PureCtrlPanel:
	$(MAKE) -C PureCtrlPanel/


install:
	$(INSTALL) -m 755 $(PROG) $(DESTDIR)/usr/local/bin/

clean:
	$(MAKE) -C PureSwitch clean
	$(MAKE) -C PureCtrlPanel clean



