#! /usr/bin/env bash
arduino-cli compile -b teensy:avr:teensy41 --library libraries/Messaging basestation
arduino-cli upload -p usb3/3-1.1/3-1.1.1 -b teensy:avr:teensy41 basestation