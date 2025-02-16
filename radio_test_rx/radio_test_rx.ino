// Arduino9x_RX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Arduino9x_TX

#include <Arduino.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <messaging.hpp>

#define RFM95_CS 10
#define RFM95_RST 2
#define RFM95_INT 3

#define RF95_FREQ 915.0
constexpr uint8_t DEVICE_ID = 22;

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Blinky on receipt
#define LED 13

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  while (!Serial)
    ;
  Serial.begin(9600);
  delay(100);

  Serial.println("Arduino LoRa RX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1)
      ;
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1)
      ;
  }
  Serial.print("Set Freq to: ");
  Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf =
  // 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter
  // pin, then you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}

uint32_t sequence_number = 0;
uint8_t send_buffer[RH_RF95_MAX_MESSAGE_LEN];
uint8_t receive_buffer[RH_RF95_MAX_MESSAGE_LEN];

void loop() {
  if (rf95.available()) {
    // Should be a message for us now
    uint8_t len = sizeof(receive_buffer);

    if (rf95.recv(receive_buffer, &len)) {
      digitalWrite(LED, HIGH);
      RH_RF95::printBuffer("Received: ", receive_buffer, len);
      if (len < sizeof(Header)) {
        Serial.println("Received message is too short for header");
        return;
      }
      Header header;
      memcpy(&header, receive_buffer, sizeof(header));
      if (header.type == MessageType::Command) {
        if (len < sizeof(CommandMessage)) {
          Serial.println("Received message is too short for CommandMessage");
          return;
        }
        CommandMessage msg;
        memcpy(&msg, receive_buffer, sizeof(msg));
        Serial.print("Received command from ");
        Serial.print(msg.header.source_id);
        Serial.print(" at ");
        Serial.print(msg.header.time_sec);
        Serial.print(".");
        Serial.print(msg.header.time_nsec);
        Serial.print(" with throttle ");
        Serial.print(msg.throttle);
        Serial.print(" and rudder angle ");
        Serial.println(msg.rudder_angle);
      }
    //   else if (header.type == MessageType::Reply) {
    //     if (len < sizeof(ReplyMessage)) {
    //       Serial.println("Received message is too short for ReplyMessage");
    //       return;
    //     }
    //     ReplyMessage msg;
    //     memcpy(&msg, receive_buffer, sizeof(msg));
    //     Serial.print("Received reply from ");
    //     Serial.print(msg.header.source_id);
    //     Serial.print(" at ");
    //     Serial.print(msg.header.time_sec);
    //     Serial.print(".");
    //     Serial.print(msg.header.time_nsec);
    //     Serial.print(" with heading ");
    //     Serial.print(msg.heading);
    //     Serial.print(" and speed ");
    //     Serial.println(msg.speed);
    //   } else {
    //     Serial.print("Received message with unknown type ");
    //     Serial.println(header.type);
    //   }
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
    //   Serial.println(sizeof(CommandMessage));

      // Send a reply
      ReplyMessage reply = {
        .header = {
            .sequence_number = sequence_number++,
            .time_sec = millis() / 1000,
            .time_nsec = millis() % 1000,
            .type = MessageType::Reply,
            .source_id = DEVICE_ID,
            .dest_id = header.source_id,
            .padding = 0
        },
        .current_throttle = 0.5,
        .current_rudder_angle = 0.1,
        .state = BoatState::Teleop
      };
      memcpy(send_buffer, &reply, sizeof(reply));
      rf95.send(send_buffer, sizeof(reply));
      rf95.waitPacketSent();
      Serial.println("Sent a reply");
    //   RH_RF95::printBuffer("Sent: ", send_buffer, sizeof(reply));
      digitalWrite(LED, LOW);
    } else {
      Serial.println("Receive failed");
    }
    Serial.println("");
  }
}