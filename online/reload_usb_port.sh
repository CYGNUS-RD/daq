#!/bin/bash

port="usb3-3:2.0" # replace numbers with actual bus number as shown by lsusb -t: {bus}-{port}(.{subport})

bind_usb() {
  echo "$1" >/sys/bus/usb/drivers/kvaser_usb/bind
}

unbind_usb() {
  echo "$1" >/sys/bus/usb/drivers/kvaser_usb/unbind
}

unbind_usb "$port"
sleep 1 # enable delay here
bind_usb "$port"
