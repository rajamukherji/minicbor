.PHONY: clean all install

PLATFORM = $(shell uname)
MACHINE = $(shell uname -m)

all: libminicbor.a

*.o: *.h

CFLAGS += -std=gnu99 -fstrict-aliasing -Wstrict-aliasing -Wall \
	-I. -pthread -DGC_THREADS -D_GNU_SOURCE
LDFLAGS += -lm

ifdef DEBUG
	CFLAGS += -g -DGC_DEBUG -DDEBUG
	LDFLAGS += -g
else
	CFLAGS += -O3
	LDFLAGS +=
endif

common_objects = \
	minicbor_reader.o \
	minicbor_writer.o

platform_objects =

ifeq ($(MACHINE), i686)
	CFLAGS += -fno-pic
endif

ifeq ($(PLATFORM), Linux)
endif

ifeq ($(PLATFORM), FreeBSD)
endif

ifeq ($(PLATFORM), Darwin)
endif

libminicbor.a: $(common_objects) $(platform_objects)
	ar rcs $@ $(common_objects) $(platform_objects)

clean:
	rm -f *.o
	rm -f libminicbor.a

PREFIX = /usr
install_include = $(DESTDIR)$(PREFIX)/include/minicbor
install_lib = $(DESTDIR)$(PREFIX)/lib

install_h = \
	$(install_include)/minicbor.h

install_a = $(install_lib)/libminicbor.a

$(install_h): $(install_include)/%: %
	mkdir -p $(install_include)
	cp $< $@

$(install_a): $(install_lib)/%: %
	mkdir -p $(install_lib)
	cp $< $@

install: $(install_h) $(install_a)
