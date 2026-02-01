CC     ?= cc
TARGET := windows

WAYLAND_SCANNER   := $(shell pkg-config --variable=wayland_scanner wayland-scanner)
WAYLAND_PROTOCOLS := $(shell pkg-config --variable=pkgdatadir wayland-protocols)
WAYLAND_HEADER    := ext-foreign-toplevel-list-v1-protocol.h
WAYLAND_INTERFACE := ext-foreign-toplevel-list-v1-protocol.c

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
DEPS := $(SRCS:.c=.d)

CFLAGS += $(shell pkg-config --cflags wayland-client)
LDLIBS += $(shell pkg-config --libs wayland-client)

ext-foreign-toplevel-list-v1-protocol.h: \
	$(WAYLAND_PROTOCOLS)/staging/ext-foreign-toplevel-list/ext-foreign-toplevel-list-v1.xml
	$(WAYLAND_SCANNER) client-header $< $@

ext-foreign-toplevel-list-v1-protocol.c: \
	$(WAYLAND_PROTOCOLS)/staging/ext-foreign-toplevel-list/ext-foreign-toplevel-list-v1.xml
	$(WAYLAND_SCANNER) private-code $< $@

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.d: %.c $(WAYLAND_HEADER) $(WAYLAND_INTERFACE)
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(DEPS)

.PHONY: clean
clean:
	rm -f $(OBJS) $(DEPS) $(TARGET) $(WAYLAND_HEADER) $(WAYLAND_INTERFACE)

.DEFAULT_GOAL := $(TARGET)
