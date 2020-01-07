CONTIKI = ../..
CONTIKI_PROJECT = PureCtrlPanel

TARGET = pure-cc2538

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

REST_RESOURCES_DIR = ./resources

REST_RESOURCES_FILES += res-leds.c  

PROJECTDIRS += $(REST_RESOURCES_DIR)
PROJECT_SOURCEFILES += $(REST_RESOURCES_FILES)

PROJECT_SOURCEFILES += coap-server.c coap-client.c test-coffee.c

PLATFORM_HAS_RELAY_SWITCH=0
PLATFORM_HAS_BUTTON = 1
PLATFORM_HAS_LEDS = 1

APPS = serial-shell 
#APPS += ping6
APPS += er-coap
APPS += rest-engine


COFFEE_FILES = 4

#CONTIKI_WITH_RIME = 1
CONTIKI_WITH_IPV6 = 1
#CONTIKI_WITH_IPV4 =1
SOURCE_LDSCRIPT = ./cc2538_boot.lds
#USB_CDC_ETH_CLASS = 1

all: $(CONTIKI_PROJECT) hexImage

hexImage:
	arm-none-eabi-objcopy -O ihex $(CONTIKI_PROJECT).$(TARGET) $(CONTIKI_PROJECT).hex

include $(CONTIKI)/Makefile.include
