CONTIKI_PROJECT=border-router

TARGET = pure-cc2538
PLATFORM_HAS_BUTTON = 1
PLATFORM_HAS_LEDS = 1

all: $(CONTIKI_PROJECT) 

CONTIKI=../..

#linker optimizations
SMALL=1

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"


REST_RESOURCES_DIR = ./resources ../common_resources 

REST_RESOURCES_FILES += res-defines.c res-device.c res-nbr.c

PROJECTDIRS += $(REST_RESOURCES_DIR)

PROJECT_SOURCEFILES += $(REST_RESOURCES_FILES)

PROJECT_SOURCEFILES += coap-server.c coap-client.c

PROJECT_SOURCEFILES += slip-bridge.c

#Simple built-in webserver is the default.
#Override with make WITH_WEBSERVER=0 for no webserver.
#WITH_WEBSERVER=webserver-name will use /apps/webserver-name if it can be
#found in the /apps, /platform/$(TARGET)/apps/, or current directory (in that order).
# WITH_WEBSERVER=webserver for /apps/webserver
# WITH_WEBSERVER=raven-webserver for /platform/avr-raven/apps/raven-webserver/
#make clean before changing webservers!

#Note /apps/webserver contains a 2500 byte style sheet which is a severe test
#of the slip connection. Large MSS together with low baud rates without flow
#control will overrun the transmit buffer when the style sheet is requested.

ifeq ($(MAKE_WITH_NON_STORING),1)
CFLAGS += -DWITH_NON_STORING=1
endif

WITH_WEBSERVER=0
ifeq ($(WITH_WEBSERVER),1)
CFLAGS += -DUIP_CONF_TCP=1
CFLAGS += -DWEBSERVER=1
PROJECT_SOURCEFILES += httpd-simple.c
else ifneq ($(WITH_WEBSERVER), 0)
APPS += $(WITH_WEBSERVER)
CFLAGS += -DUIP_CONF_TCP=1
CFLAGS += -DWEBSERVER=2
endif

APPS += serial-shell 
APPS += er-coap
APPS += rest-engine
APPS += telnetd
CFLAGS += -DTELNET_ENABLE=1

#TSCH CONFIG
MAKE_WITH_ORCHESTRA ?= 0 # force Orchestra from command line
MAKE_WITH_SECURITY ?= 0 # force Security from command line
USING_TSCH ?= 0 # force TSCH Selection from command line

ifeq ($(USING_TSCH),1)

#PROJECT_SOURCEFILES +=  tsch_node.c

APPS += orchestra
MODULES += core/net/mac/tsch
CFLAGS += -DMAC_USING_TSCH

ifeq ($(MAKE_WITH_ORCHESTRA),1)
CFLAGS += -DWITH_ORCHESTRA=1
endif

ifeq ($(MAKE_WITH_SECURITY),1)
CFLAGS += -DWITH_SECURITY=1
endif

endif


ifeq ($(PREFIX),)
 PREFIX = fd00::1/64
endif

ifeq ($(OTA_ENABLE),1)
  CFLAGS += -DOTA_VERSION_NO_CCA=1
  SOURCE_LDSCRIPT = ./cc2538_no_cca.lds
else
  SOURCE_LDSCRIPT = ./cc2538_boot.lds
endif

CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include

$(CONTIKI)/tools/tunslip6:	$(CONTIKI)/tools/tunslip6.c
	(cd $(CONTIKI)/tools && $(MAKE) tunslip6)

connect-router:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 $(PREFIX)

connect-router-cooja:	$(CONTIKI)/tools/tunslip6
	sudo $(CONTIKI)/tools/tunslip6 -a 127.0.0.1 $(PREFIX)
