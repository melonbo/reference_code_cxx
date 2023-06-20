CONFIG_UCLIBC_BUILD=y

CROSS_COMPILE ?= mips-linux-gnu-

CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar cr
STRIP = $(CROSS_COMPILE)strip

CFLAGS = $(INCLUDES) -O2 -Wall -march=mips32r2

ifeq ($(CONFIG_UCLIBC_BUILD), y)
CFLAGS += -muclibc
LDFLAG += -muclibc
endif

ifeq ($(CONFIG_UCLIBC_BUILD), y)
SDK_LIB_DIR = ../../lib/uclibc
else
SDK_LIB_DIR = ../../lib/glibc
endif

SDK_INC_DIR = ./

INCLUDES = -I$(SDK_INC_DIR) -I../../include

LIBS =	$(SDK_LIB_DIR)/libimp.a $(SDK_LIB_DIR)/libalog.a

LDFLAG += -Wl,-gc-sections

SAMPLES = rtsp_server

all: 	$(SAMPLES)

rtsp_server: $(SDK_LIB_DIR)/libsysutils.a $(SDK_LIB_DIR)/libalog.a rtscp-server.c sample-common.c
	$(CC) $(INCLUDES)  $(LDFLAG) -o $@ $^ $(LIBS) -lpthread -lm -lrt
	$(STRIP) $@

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o *~
	rm -f $(SAMPLES)

distclean: clean
	rm -f $(SAMPLES)
