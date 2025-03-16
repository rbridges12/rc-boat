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

## 2/16/25
- wrote initial boat code
- fixed basestation not receiving reply messages by reducing Tx rate (need to investigate this)
- working basic state machines for both basestation and boat code communicating with each other

## 2/17/25 - 2/27/25
- designed and 3D printed test case for radio
- initial radio range communication test
- picked and bought brushless motor, ESC, LiPo battery + charger, servos, and propeller shaft
- designed motor and prop shaft mount in CAD
- tested servo and brushless motor control using arduino
- started boat hull design in CAD

## 2/28/25
- wrote code to initialize and control brushless ESC and servo
- wired up and tested boat testbed with radio servo and ESC, all powered by ESC BEC, tested teleop motor and servo control

## 2/29/25 - 3/13/25
- printed test motor mount
- printed very simple hull shape for scale
- finished boat hull design v1 in CAD
- printed boat hull v1

## 3/14/25
- removed support material and cleaned up boat hull
- rough test assembly of boat halves, motor mount, and rudder assembly with hot glue
- attached motor and servo, aligned prop shaft

## 3/15/25
- attached servo pushrod
- connected electronics and tested remote control with hardware attached to boat
- tuned control limits
- submerged back of boat in the sink and tested propeller and rudder functionality in water