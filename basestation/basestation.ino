
#include <Arduino.h>
// #include <RH_RF95.h>
#include <LoRa.h>
#include <SPI.h>
#include <messaging.hpp>

constexpr int RFM95_CS = 10;
constexpr int RFM95_RST = 2;
constexpr int RFM95_INT = 3;
constexpr int LED = 13;
constexpr float RF95_FREQ = 915.0;
constexpr uint8_t DEVICE_ID = 0;
constexpr uint8_t BOAT_ID = 1;

constexpr char TOGGLE_ENABLE_KEY = ' ';
constexpr char THROTTLE_UP_KEY = 'w';
constexpr char THROTTLE_DOWN_KEY = 's';
constexpr char RUDDER_LEFT_KEY = 'a';
constexpr char RUDDER_RIGHT_KEY = 'd';

constexpr uint64_t CMD_TX_INTERVAL = 1000;  // ms
constexpr uint64_t REPLY_TIMEOUT = 1000;    // ms
constexpr float MIN_THROTTLE = 5.0;
constexpr float MAX_THROTTLE = 20.0;
constexpr float MIN_RUDDER_ANGLE = 50.0;
constexpr float MAX_RUDDER_ANGLE = 130.0;

// RH_RF95 rf95(RFM95_CS, RFM95_INT);
StationState state = StationState::Off;
BoatState boat_state = BoatState::Off;
float current_throttle = 5.0;
float current_rudder_angle = 90.0;

uint32_t sequence_number = 0;
uint8_t send_buffer[255];
uint8_t receive_buffer[255];
uint64_t last_send_time = 0;
uint64_t last_recv_time = 0;
bool led_state = false;

void setup() {
  pinMode(LED, OUTPUT);
  // pinMode(RFM95_RST, OUTPUT);
  // digitalWrite(RFM95_RST, HIGH);

  Serial.begin(9600);
  while (!Serial);
  delay(100);

  // // manual reset
  // digitalWrite(RFM95_RST, LOW);
  // delay(10);
  // digitalWrite(RFM95_RST, HIGH);
  // delay(10);
  LoRa.setPins(RFM95_CS, RFM95_RST, RFM95_INT);

  //   init and configure radio
  // while (!rf95.init()) {
  //   Serial.println("LoRa radio init failed");
  //   while (1)
  //     ;
  // }
  if (!LoRa.begin(915E6)) {
      Serial.println("LoRa radio init failed");
    while (true);
  }
  
  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("LoRa init succeeded.");

  // Serial.println("LoRa radio init OK!");
  // if (!rf95.setFrequency(RF95_FREQ)) {
  //   Serial.println("setFrequency failed");
  //   while (1)
  //     ;
  // }
  // Serial.print("Set Freq to: ");
  // Serial.println(RF95_FREQ);
  // rf95.setTxPower(23, false);
}

void loop() {
  switch (state) {
  case StationState::Off: {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == TOGGLE_ENABLE_KEY) {
        state = StationState::Teleop;
        delay(100);
        Serial.println("Switching to teleop mode");
      }
    }
    break;
  }
  case StationState::Teleop: {
    // check for command key input
    bool command_changed = false;
    if (Serial.available()) {
      char c = Serial.read();
      switch (c) {
      case TOGGLE_ENABLE_KEY:
        state = StationState::Off;
        break;
      case THROTTLE_UP_KEY:
        if (current_throttle < MAX_THROTTLE)
          current_throttle += 1;
        command_changed = true;
        break;
      case THROTTLE_DOWN_KEY:
        if (current_throttle > MIN_THROTTLE)
          current_throttle -= 1;
        command_changed = true;
        break;
      case RUDDER_LEFT_KEY:
        if (current_rudder_angle > MIN_RUDDER_ANGLE)
          current_rudder_angle -= 1;
        command_changed = true;
        break;
      case RUDDER_RIGHT_KEY:
        if (current_rudder_angle < MAX_RUDDER_ANGLE)
          current_rudder_angle += 1;
        command_changed = true;
        break;
      }
    }

    // send command message
    if (command_changed || (millis() - last_send_time) > CMD_TX_INTERVAL) {
      uint32_t time_sec = millis() / 1000;
      uint32_t time_nsec = (millis() % 1000) * 1000000;
      CommandMessage msg = {
          .header = {
            .sequence_number = sequence_number++,
            .time_sec = time_sec,
            .time_nsec = time_nsec,
            .type = MessageType::Command,
            .source_id = DEVICE_ID,
            .dest_id = BOAT_ID
          },
          .throttle = current_throttle,
          .rudder_angle = current_rudder_angle};
      memcpy(send_buffer, &msg, sizeof(msg));
      // Serial.println(sizeof(CommandMessage));
      // RH_RF95::printBuffer("Sending: ", send_buffer, sizeof(msg));
      // rf95.send(send_buffer, sizeof(msg));
      // delay(10);
      // rf95.waitPacketSent();
      LoRa.beginPacket();
      LoRa.write(send_buffer, sizeof(msg));
      LoRa.endPacket();
      last_send_time = millis();
    }

    // check for reply timeout
    if (millis() - last_recv_time > REPLY_TIMEOUT) {
      boat_state = BoatState::Recovery;
      digitalWrite(LED, LOW);
      // Serial.println("Warning: boat failed to reply after timeout");
    }
    break;
  }
  }
}


void onReceive(int packet_size) {
  ReplyMessage reply;
  Serial.print("Received message, size=");
  Serial.println(packet_size);
  if (packet_size == 0) return;
  for (int i = 0; i < packet_size; i++) {
    receive_buffer[i] = (uint8_t) LoRa.read();
  }
  if (packet_size >= sizeof(Header)) {
    Header header;
    memcpy(&header, receive_buffer, sizeof(header));
    if (header.type == MessageType::Reply) {
      if (packet_size >= sizeof(ReplyMessage)) {
        memcpy(&reply, receive_buffer, sizeof(reply));
        // received = true;
        led_state = !led_state;
        digitalWrite(LED, led_state);
        last_recv_time = millis();
        boat_state = reply.state;
        Serial.print("Received reply from ");
        Serial.print(reply.header.source_id);
        Serial.print(" at ");
        Serial.print(reply.header.time_sec);
        Serial.print(".");
        Serial.print(reply.header.time_nsec);
        Serial.print(" with state ");
        Serial.print(static_cast<int>(reply.state));
        Serial.print(", throttle ");
        Serial.print(reply.current_throttle);
        Serial.print(", and rudder angle ");
        Serial.println(reply.current_rudder_angle);
      } else {
        Serial.println("Received message is too short for ReplyMessage");
      }
    } else {
      Serial.print("Received message with unknown type ");
      Serial.println((int)header.type);
    }
  }
    Serial.print("RSSI: ");
    // Serial.println(rf95.lastRssi(), DEC);
    Serial.println(LoRa.rssi(), DEC);
}