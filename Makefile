DESTDIR=/
prefix=usr

ifeq ($(DEB_BUILD_GNU_TYPE),$(DEB_HOST_GNU_TYPE))
       CC=gcc
else
       CC=$(DEB_HOST_GNU_TYPE)-gcc
endif

CC_FLAGS=-I../common -I/usr/include/modbus -L../common -lmodbus

all: client server

client:
	$(MAKE) -C modbus_client

server:
	$(MAKE) -C modbus_server

install: $(BIN_NAME)
	$(MAKE) -C modbus_client install
	$(MAKE) -C modbus_server install
	install -m 0755 set-rd-address.sh $(DESTDIR)/$(prefix)/bin/set-rd-address.sh

clean:
	$(MAKE) -C modbus_client clean
	$(MAKE) -C modbus_server clean

.PHONY: install clean all
