DESTDIR=/
prefix=usr
BIN_NAME=modbus_client

ifeq ($(DEB_BUILD_GNU_TYPE),$(DEB_HOST_GNU_TYPE))
       CC=gcc
else
       CC=$(DEB_HOST_GNU_TYPE)-gcc
endif

CC_FLAGS=-I../common `pkg-config --libs --cflags libmodbus`

$(BIN_NAME): modbus_client.c
	$(CC)  modbus_client.c  $(CC_FLAGS)  -o $(BIN_NAME)


install: $(BIN_NAME)
	install -m 0755 $(BIN_NAME) $(DESTDIR)/$(prefix)/bin/$(BIN_NAME)

clean:
	-@rm $(BIN_NAME)
.PHONY: install clean all



