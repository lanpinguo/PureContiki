CONTIKI = ../..
CONTIKI_PROJECT = PureSwitch

TARGET = pure-cc2538

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

REST_RESOURCES_DIR = ./resources ../common_resources

REST_RESOURCES_FILES += res-defines.c res-device.c res-leds.c res-relay-switch.c res-hcho.c

PROJECTDIRS += $(REST_RESOURCES_DIR)
PROJECT_SOURCEFILES += $(REST_RESOURCES_FILES)

PROJECT_SOURCEFILES += coap-server.c coap-client.c test-coffee.c 

PLATFORM_HAS_RELAY_SWITCH=1
PLATFORM_HAS_LEDS=1
PLATFORM_HAS_BUTTON = 0
PLATFORM_HAS_HCHO_SENSOR = 1

APPS = serial-shell 
APPS += telnetd
CFLAGS += -DTELNET_ENABLE=1
#APPS += ping6
APPS += er-coap
APPS += rest-engine

#TSCH CONFIG
MAKE_WITH_ORCHESTRA ?= 0 # force Orchestra from command line
MAKE_WITH_SECURITY ?= 0 # force Security from command line
USING_TSCH ?= 0 # force TSCH Selection from command line

ifeq ($(USING_TSCH),1)

PROJECT_SOURCEFILES +=  tsch_node.c

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



COFFEE_FILES = 10

#CONTIKI_WITH_RIME = 1
CONTIKI_WITH_IPV6 = 1
#CONTIKI_WITH_IPV4 =1
#USB_CDC_ETH_CLASS = 1

ifeq ($(OTA_ENABLE),1)
  CFLAGS += -DOTA_VERSION_NO_CCA=1
  SOURCE_LDSCRIPT = ./cc2538_no_cca.lds
else
  SOURCE_LDSCRIPT = ./cc2538_boot.lds
endif


all: $(CONTIKI_PROJECT) hexImage

hexImage:
	arm-none-eabi-objcopy -O ihex $(CONTIKI_PROJECT).$(TARGET) $(CONTIKI_PROJECT).hex

include $(CONTIKI)/Makefile.include
