CONTIKI = ../..

CONTIKI_PROJECT=bootloader

TARGET = pure-cc2538

all: $(CONTIKI_PROJECT) 


#linker optimizations
SMALL=1

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

BOOTLOADER_MODE_ENABLE=1
SOURCE_LDSCRIPT = ./cc2538_boot.lds

include $(CONTIKI)/Makefile.include


