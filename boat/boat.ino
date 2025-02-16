
#include <Arduino.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <messaging.hpp>

constexpr int RFM95_CS = 10;
constexpr int RFM95_RST = 2;
constexpr int RFM95_INT = 3;
constexpr int LED = 13;
constexpr float RF95_FREQ = 915.0;
constexpr uint8_t DEVICE_ID = 1;
constexpr uint8_t BASESTATION_ID = 0;

// constexpr char TOGGLE_ENABLE_KEY = ' ';
// constexpr char THROTTLE_UP_KEY = 'w';
// constexpr char THROTTLE_DOWN_KEY = 's';
// constexpr char RUDDER_LEFT_KEY = 'a';
// constexpr char RUDDER_RIGHT_KEY = 'd';

// constexpr uint64_t CMD_TX_INTERVAL = 1000;  // ms
constexpr uint64_t CMD_TIMEOUT = 1200;    // ms

RH_RF95 rf95(RFM95_CS, RFM95_INT);
// StationState state = StationState::Off;
BoatState state = BoatState::Off;
float current_throttle = 0.0;
float current_rudder_angle = 0.0;

uint32_t sequence_number = 0;
uint8_t send_buffer[RH_RF95_MAX_MESSAGE_LEN];
uint8_t receive_buffer[RH_RF95_MAX_MESSAGE_LEN];
uint64_t last_send_time = 0;
uint64_t last_recv_time = 0;

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  while (!Serial)
    ;
  Serial.begin(9600);
  delay(100);

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  //   init and configure radio
  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1)
      ;
  }
  Serial.println("LoRa radio init OK!");
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1)
      ;
  }
  Serial.print("Set Freq to: ");
  Serial.println(RF95_FREQ);
  rf95.setTxPower(23, false);
}

void loop() {
  // receive message from radio if available
  bool received = false;
  CommandMessage cmd;
  if (rf95.available()) {
    uint8_t len = sizeof(receive_buffer);
    if (rf95.recv(receive_buffer, &len)) {
      Serial.println(sizeof(ReplyMessage));
      RH_RF95::printBuffer("Received: ", receive_buffer, len);
      if (len >= sizeof(Header)) {
        Header header;
        memcpy(&header, receive_buffer, sizeof(header));
        if (header.type == MessageType::Command) {
          if (len >= sizeof(CommandMessage)) {
            memcpy(&cmd, receive_buffer, sizeof(cmd));
            last_recv_time = millis();
            received = true;

            // TODO: make message printing function
            Serial.print("Received command from ");
            Serial.print(cmd.header.source_id);
            Serial.print(" at ");
            Serial.print(cmd.header.time_sec);
            Serial.print(".");
            Serial.println(cmd.header.time_nsec);
            Serial.print("Throttle: ");
            Serial.print(cmd.throttle);
            Serial.print(" Rudder: ");
            Serial.println(cmd.rudder_angle);
          } else {
            Serial.println("Received message is too short for CommandMessage");
          }
        } else {
          Serial.print("Received message with unknown type ");
          Serial.println((int)header.type);
        }
      }
    }
  }

  switch (state) {
  case BoatState::Off: {
    if (received) {
        Serial.println("Received first command, transitioning to teleop");
        state = BoatState::Teleop;
    }
    break;
  }
  case BoatState::Teleop: {
    if (received) {
        current_throttle = cmd.throttle;
        current_rudder_angle = cmd.rudder_angle;
        Serial.print("executing command: ");
        Serial.print(current_throttle);
        Serial.print(" ");
        Serial.println(current_rudder_angle);

        // send reply message
        uint32_t time_sec = millis() / 1000;
        uint32_t time_nsec = (millis() % 1000) * 1000000;
        ReplyMessage msg = {
            .header = {
                .sequence_number = sequence_number++,
                .time_sec = time_sec,
                .time_nsec = time_nsec,
                .type = MessageType::Reply,
                .source_id = DEVICE_ID,
                .dest_id = BASESTATION_ID
            },
            .current_throttle = current_throttle,
            .current_rudder_angle = current_rudder_angle,
            .state = state
        };
        memcpy(send_buffer, &msg, sizeof(msg));
        rf95.send(send_buffer, sizeof(msg));
        // last_send_time = millis();
    }
    else if (millis() - last_recv_time > CMD_TIMEOUT) {
        Serial.println("boat failed to receive command after timeout, transitioning to recovery");
        state = BoatState::Recovery;
        Serial.println("cancelling all commands");
    }
    break;
  }

  case BoatState::Recovery: {
    if (received) {
        Serial.println("Received command during recovery, transitioning to teleop");
        state = BoatState::Teleop;
    }
  }
  }
}