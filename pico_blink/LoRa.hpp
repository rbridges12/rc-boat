// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LORA_H
#define LORA_H

// #include <Arduino.h>
// #include <SPI.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

#define LORA_DEFAULT_SPI           spi0
#define LORA_DEFAULT_SPI_FREQUENCY 8E6 

#define PA_OUTPUT_RFO_PIN          0
#define PA_OUTPUT_PA_BOOST_PIN     1

class LoRa {
public:
  LoRa(int miso_pin, int mosi_pin, int sck_pin, int cs_pin, int reset_pin, int dio0_pin);

  int begin(long frequency);
  void end();

  int beginPacket(int implicitHeader = false);
  int endPacket(bool async = false);

  int parsePacket(int size = 0);
  int packetRssi();
  float packetSnr();
  long packetFrequencyError();

  int rssi();

  size_t write(uint8_t uint8_t);
  size_t write(const uint8_t *buffer, size_t size);

  int available();
  int read();
  int peek();
  void flush();

  void onReceive(void(*callback)(int));
  void onCadDone(void(*callback)(bool));
  void onTxDone(void(*callback)());

  void receive(int size = 0);
  void channelActivityDetection(void);
  void idle();
  void sleep();

  void setTxPower(int level, int outputPin = PA_OUTPUT_PA_BOOST_PIN);
  void setFrequency(long frequency);
  void setSpreadingFactor(int sf);
  void setSignalBandwidth(long sbw);
  void setCodingRate4(int denominator);
  void setPreambleLength(long length);
  void setSyncWord(int sw);
  void enableCrc();
  void disableCrc();
  void enableInvertIQ();
  void disableInvertIQ();
  void enableLowDataRateOptimize();
  void disableLowDataRateOptimize();
  
  void setOCP(uint8_t mA); // Over Current Protection control
  
  void setGain(uint8_t gain); // Set LNA gain

  uint8_t random();

  // void setPins(int ss = CS_PIN, int reset = LORA_DEFAULT_RESET_PIN, int dio0 = LORA_DEFAULT_DIO0_PIN);
  // void setSPI(SPIClass& spi);
  void setSPIFrequency(uint32_t frequency);

  // void dumpRegisters(Stream& out);

  static inline uint8_t bitWrite(uint8_t x, uint8_t n, bool b);

private:
  void explicitHeaderMode();
  void implicitHeaderMode();

  void handleDio0Rise();
  bool isTransmitting();

  int getSpreadingFactor();
  long getSignalBandwidth();

  void setLdoFlag();
  void setLdoFlagForced(const bool);

  uint8_t readRegister(uint8_t address);
  void writeRegister(uint8_t address, uint8_t value);
  uint8_t singleTransfer(uint8_t address, uint8_t value);

  static void onDio0Rise(uint gpio, uint32_t events);

private:
  int _miso_pin;
  int _mosi_pin;
  int _sck_pin;
  int _ss;
  int _reset;
  int _dio0;
  long _frequency;
  int _packetIndex;
  int _implicitHeaderMode;
  void (*_onReceive)(int);
  void (*_onCadDone)(bool);
  void (*_onTxDone)();
};

// extern LoRaClass LoRa;

#endif
