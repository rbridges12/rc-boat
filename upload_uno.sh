#! /usr/bin/env bash
arduino-cli compile -b arduino:avr:uno --library libraries/Messaging radio_test_rx
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno radio_test_rx