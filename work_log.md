# Work Log

## 1/11/25
- tested out pico and teensy, several use cables were bad, and pico was dead :(
- setup teensy dev env, both arduino 2 IDE and arduino CLI

## 1/12/25
- got pico working (it wasn't dead)
- set up pico dev environment, including building picotool for ubuntu 20.04

## 1/26/25
- tried to get much of SPI devices working on teensy and arduino pro mini, nothing was working. got stuff working on arduino uno, and radio module was able to initialize
- SPI still not working on teensy, probably because pin headers are not actually soldered on

## 2/1/25
- got access to mit makerspace and soldered all pin headers on
- finally got LoRa working on teensy (power rail on breadboard was broken), got successful LoRa communication between the two boards using example code

## 2/9/25
- implemented messaging protocol
- figured out manual library compilation
- laid out basic state machine control flow and data transmission between boat and basestation

## 2/12/25
- wrote initial basestation code
- attached SMA antennas (went from ~-70 to ~-20 RSSI)
- tried testing basestation code, messages aren't parsing correctly between arduino uno and teensy 4.1, probably some kind of struct packing issue between 8bit and 32bit processors
- fixed struct packing issue by adding a dummy byte
- basestation not receiving telemetry messages for some reason