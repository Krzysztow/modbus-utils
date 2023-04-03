modbus-utils
============

Modbus client and server command line tools based on libmodbus.

NOTE:
Both apps are linked with libmodbus library. After repository is pulled do the following:

compilation
===========

```sh
make
```

running
=======

If modbus libraries are not in a default location (either it's needed to move libraries to app location or set
appropriate environment variable):
- on linux it would be:
  1) LD_LIBRARY_PATH=./libmodbus/src/.libs/ ./mbClient OR
  2) export LD_LIBRARY_PATH=./libmodbus/src/.libs/ and then run ./mbClient

usage
=====

Run apps with no arguments, descriptive help information will be provided.
