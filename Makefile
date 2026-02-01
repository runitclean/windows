WAYLAND_SCANNER  :=$(shell pkg-config --variable=wayland_scanner wayland-scanner)
WAYLAND_PROTOCOLS:=$(shell pkg-config --variable=pkgdatadir wayland-protocols)

ext-foreign-toplevel-list-v1-protocol.h:
	@$(WAYLAND_SCANNER) client-header \
	$(WAYLAND_PROTOCOLS)/staging/ext-foreign-toplevel-list/ext-foreign-toplevel-list-v1.xml \
	$@

ext-foreign-toplevel-list-v1-protocol.c:
	@$(WAYLAND_SCANNER) private-code \
	$(WAYLAND_PROTOCOLS)/staging/ext-foreign-toplevel-list/ext-foreign-toplevel-list-v1.xml \
	$@

windows: ext-foreign-toplevel-list-v1-protocol.h ext-foreign-toplevel-list-v1-protocol.c

clean:
	@rm -f ext-foreign-toplevel-list-v1-protocol.h ext-foreign-toplevel-list-v1-protocol.c

.DEFAULT_GOAL = windows
