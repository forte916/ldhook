CROSS_COMPILE	=
#CROSS_COMPILE	= arm-linux-gnueabi-
CC				= $(CROSS_COMPILE)gcc
ARCH=$(shell echo $(CROSS_COMPILE) | cut -d- -f1 )

DEBUG_FLAGS		=	-DDEBUG -DDEBUG_LOG ##-H
CFLAGS			=	-Wall -D_GNU_SOURCE ##-DFOPEN_FCLOSE
#CFLAGS			+=	-D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
# #ifdef __USE_LARGEFILE64
# #ifndef __USE_FILE_OFFSET64
CFLAGS			+=	$(DEBUG_FLAGS)
INCLUDES		=	
LDFLAGS			=	-L./
LIBS			=	-ldl -lpthread -lsqlite3


ALL_SRCS=$(wildcard *.c)
STORED_SRCS=./stored.c \
	./storeSqlite.c \
	./storeThread.c \
	./storeMessage.c \
	./storeControl.c \

LDHOOK_SRCS=./ldhook.c \
	./storeMessage.c \
	./storeControl.c

STORED_OBJS=$(STORED_SRCS:.c=.o)
LDHOOK_OBJS=$(LDHOOK_SRCS:.c=.o)
BINS=$(ALL_SRCS:.c=)

OUTPUT		=	output
PKG_NAME	=	ldhook.tgz

.PHONY: all bins clean run ldhook open_close
all: ldhook stored open_close

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -fPIC -c -o $@ $<
#	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean: package_clean
	$(RM) *.o $(BINS) ldhook.so

run:
	./test.sh
#	LD_LIBRARY_PATH=./ LD_PRELOAD=./ldhook.so ./open_close

stored: $(STORED_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(LIBS) -o $@ $^

ldhook: $(LDHOOK_OBJS)
	$(CC) $(LDFLAGS) $(LIBS) -fPIC -shared -o $@.so $^
#	$(CC) $(LDFLAGS) $(LIBS) -shared $^ -o $@.so

open_close: open_close.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

.PHONY: package package_clean
package: all
	mkdir -p $(OUTPUT)
	cp -fp ldhook.so $(OUTPUT)
	cp -fp stored $(OUTPUT)
	cp -fp open_close $(OUTPUT)
	cp -fp test.sh  $(OUTPUT)
	-cp -fp hello.txt $(OUTPUT)
	tar -zcvf $(PKG_NAME) $(OUTPUT)

package_clean:
	$(RM) -r $(OUTPUT) $(PKG_NAME)

