#! /usr/bin/env bash
arduino-cli compile -b teensy:avr:teensy41 --library libraries/Messaging basestation
arduino-cli upload -p usb1/1-5 -b teensy:avr:teensy41 basestation