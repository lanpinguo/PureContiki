

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


.PHONY:all clean PureSwitch PureCtrlPanel border-router PureSensor


all: PureSwitch PureCtrlPanel border-router PureSensor


PureSwitch:
	$(MAKE) -C PureSwitch/ 

PureCtrlPanel:
	$(MAKE) -C PureCtrlPanel/

PureBdrAgent:
	$(MAKE) -C PureBdrAgent/

border-router:
	$(MAKE) -C rpl-border-router/

PureSensor:
	$(MAKE) -C PureSensor/

install:
	$(INSTALL) -m 755 $(PROG) $(DESTDIR)/usr/local/bin/

clean:
	$(MAKE) -C PureSwitch clean
	$(MAKE) -C PureCtrlPanel clean
	#$(MAKE) -C PureBdrAgent clean
	$(MAKE) -C rpl-border-router clean
	$(MAKE) -C PureSensor clean



