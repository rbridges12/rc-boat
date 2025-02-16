#! /usr/bin/env bash
arduino-cli compile -b arduino:avr:uno --library libraries/Messaging boat
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno boat