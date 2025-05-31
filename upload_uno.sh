#! /usr/bin/env bash
arduino-cli compile -b arduino:avr:uno --library libraries/Messaging basestation
# arduino-cli compile -b arduino:avr:uno --library libraries/Messaging boat
arduino-cli upload -p /dev/ttyACM2 --fqbn arduino:avr:uno basestation
# arduino-cli upload -p /dev/ttyACM2 --fqbn arduino:avr:uno boat