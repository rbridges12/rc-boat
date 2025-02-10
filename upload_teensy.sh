#! /usr/bin/env bash
arduino-cli compile -b teensy:avr:teensy41 --library libraries/Messaging radio_test_tx
arduino-cli upload -p usb1/1-5 -b teensy:avr:teensy41 radio_test_tx