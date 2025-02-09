#include <SPI.h>
#include <RH_RF95.h>
#include <Arduino.h>
#include "messaging.hpp"

#define RFM95_CS 10
#define RFM95_RST 2
#define RFM95_INT 3

#define RF95_FREQ 915.0
constexpr uint8_t DEVICE_ID = 11;

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() 
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  while (!Serial);
  Serial.begin(9600);
  delay(100);

  Serial.println("Arduino LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}

uint32_t sequence_number = 0;
uint8_t send_buffer[RH_RF95_MAX_MESSAGE_LEN];
uint8_t receive_buffer[RH_RF95_MAX_MESSAGE_LEN];

void loop()
{
  Serial.println("Sending example command");
  
  uint32_t time_sec = millis() / 1000;
  uint32_t time_nsec = (millis() % 1000) * 1000000;
  CommandMessage msg = {
    .header = {
      .type = MessageType::Command,
      .source_id = DEVICE_ID,
      .dest_id = 22,
      .sequence_number = sequence_number++,
      .time_sec = time_sec,
      .time_nsec = time_nsec
    },
    .throttle = 0.5,
    .rudder_angle = 0.1
  };
  memcpy(send_buffer, &msg, sizeof(msg));
  RH_RF95::printBuffer("Sending: ", send_buffer, sizeof(msg));
  rf95.send(send_buffer, sizeof(msg));
  delay(10);
  rf95.waitPacketSent();

  // Now wait for a reply
  // Serial.println("Waiting for reply...");
  delay(10);
  if (rf95.waitAvailableTimeout(1000)) { 
    // Should be a reply message for us now   
    uint8_t len = sizeof(receive_buffer);
    if (rf95.recv(receive_buffer, &len)) {
      RH_RF95::printBuffer("Received: ", receive_buffer, len);
      if (len < sizeof(Header)) {
        Serial.println("Received message is too short for header");
        return;
      }
    //   Serial.println("size of header: ");
    //   Serial.println(sizeof(Header));
    //   Serial.println("size of Command: ");
    //   Serial.println(sizeof(CommandMessage));
    //   Serial.println("max message length: ");
    // Serial.println(RH_RF95_MAX_MESSAGE_LEN);
      Header header;
      memcpy(&header, receive_buffer, sizeof(header));
      Serial.println("Received a message:");
      Serial.print("Type: ");
      Serial.println(header.type == MessageType::Command ? "Command" : "Reply");
      Serial.print("Source ID: ");
      Serial.println(header.source_id);
      Serial.print("Destination ID: ");
      Serial.println(header.dest_id);
      Serial.print("Sequence number: ");
      Serial.println(header.sequence_number);
      Serial.print("Time: ");
      Serial.print(header.time_sec);
      Serial.print(".");
      Serial.println(header.time_nsec);
      if (header.type != MessageType::Reply)
      {
        Serial.println("Received message is not a response");
        return;
      }
      ReplyMessage reply;
      memcpy(&reply, receive_buffer, sizeof(reply));
      Serial.println("Received a reply:");
      Serial.print("Heading: ");
      Serial.println(reply.heading);
      Serial.print("Speed: ");
      Serial.println(reply.speed);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);    
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
  else
  {
    Serial.println("No reply, is there a listener around?");
  }
  Serial.println("");
  delay(1000);
}