#!/bin/bash
set -u -e
if [ $# -lt 2 ]; then
  echo "Usage: $0 serial_port address [register]" 1>&2
  echo "Sets slave address using broadcast Modbus request." 1>&2
  echo "e.g. $0 /dev/ttyNSC0 0x16" 1>&2
  echo "register denotes the target holding register address and defaults to 0x80." 1>&2
  echo "Make sure that only the target device is present on the RS485 bus." 1>&2
  exit 1
fi
serial_port="$1"
address="$2"
register="0x80"
if [ $# -gt 2 ]; then
  register="$3"
fi
echo "NOTE: an error below doesn't necessarily mean that the command didn't work!" 1>&2
modbus_client --debug -mrtu -a0x00 -r"$register" -t0x06 -d8 -s1 -pnone "$serial_port" "$address"
echo "Done. Please turn the device off, then on." 1>&2
