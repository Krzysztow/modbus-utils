prefix=/usr
BIN_NAME=modbus_server

ifeq ($(DEB_BUILD_GNU_TYPE),$(DEB_HOST_GNU_TYPE))
       CC=gcc
else
       CC=$(DEB_HOST_GNU_TYPE)-gcc
endif

CC_FLAGS=-I../common `PKG_CONFIG_PATH=/usr/lib/$(DEB_HOST_GNU_TYPE)/pkgconfig pkg-config --libs --cflags libmodbus`

$(BIN_NAME):
	$(CC) modbus_server.c $(CC_FLAGS) -o $(BIN_NAME)


install: $(BIN_NAME)
	install -Dm0755 $(BIN_NAME) -t $(DESTDIR)$(prefix)/bin

clean:
	-@rm $(BIN_NAME)
.PHONY: install clean all



