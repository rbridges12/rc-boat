
#include <Arduino.h>
// #include <RH_RF95.h>
#include <LoRa.h>
#include <SPI.h>
// #include <Servo.h>
#include <PWMServo.h>
#include <messaging.hpp>

// pins
constexpr int RFM95_CS = 8;
constexpr int RFM95_RST = 2;
constexpr int RFM95_INT = 3;
constexpr int LED = 7;
constexpr int SERVO_PIN = 10;
constexpr int ESC_THROTTLE_PIN =9;

constexpr float RF95_FREQ = 915.0;
constexpr uint8_t DEVICE_ID = 1;
constexpr uint8_t BASESTATION_ID = 0;
constexpr uint64_t CMD_TIMEOUT = 1200;    // ms

// RH_RF95 rf95(RFM95_CS, RFM95_INT);
PWMServo rudder_servo;
PWMServo esc;
BoatState state = BoatState::Off;
float current_throttle = 0.0;
float current_rudder_angle = 90.0;

uint32_t sequence_number = 0;
uint8_t send_buffer[255];
uint8_t receive_buffer[255];
uint64_t last_send_time = 0;
uint64_t last_recv_time = 0;

void initialize_esc() {
  esc.attach(ESC_THROTTLE_PIN, 1000, 2000);
  Serial.println("Initializing ESC");
  esc.write(0);
  delay(5000);
  Serial.println("ESC initialized");
}

void setup() {
  pinMode(LED, OUTPUT);
  // pinMode(RFM95_RST, OUTPUT);
  // digitalWrite(RFM95_RST, HIGH);

  Serial.begin(9600);
  while (!Serial);
  delay(100);

  initialize_esc();
  rudder_servo.attach(SERVO_PIN);

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  //   init and configure radio
  if (!LoRa.begin(915E6)) {
      Serial.println("LoRa radio init failed");
    while (true);
  }
  
  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("LoRa init succeeded.");
  // while (!rf95.init()) {
  //   Serial.println("LoRa radio init failed");
  //   while (1)
  //     ;
  // }
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
  // receive message from radio if available
  bool received = false;

  switch (state) {
  case BoatState::Teleop: {
    if (millis() - last_recv_time > CMD_TIMEOUT) {
      Serial.println("boat failed to receive command after timeout, transitioning to recovery");
      state = BoatState::Recovery;
      Serial.println("cancelling all commands");
      esc.write(0);
      rudder_servo.write(90);
    }
    break;
  }
  }
}

void onReceive(int packet_size) {
  CommandMessage cmd;
  Serial.print("Received message, size=");
  Serial.println(packet_size);
  if (packet_size == 0) return;
  for (int i = 0; i < packet_size; i++) {
    receive_buffer[i] = (uint8_t) LoRa.read();
  }
  if (packet_size >= sizeof(Header)) {
    Header header;
    memcpy(&header, receive_buffer, sizeof(header));
    if (header.type == MessageType::Command) {
      if (packet_size >= sizeof(CommandMessage)) {
        memcpy(&cmd, receive_buffer, sizeof(cmd));
        last_recv_time = millis();

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

  switch (state) {
  case BoatState::Off: {
    Serial.println("Received first command, transitioning to teleop");
    state = BoatState::Teleop;
    break;
  }
  case BoatState::Teleop: {
    digitalWrite(LED, HIGH);
    delay(10);
    digitalWrite(LED, LOW);
    current_throttle = cmd.throttle;
    current_rudder_angle = cmd.rudder_angle;
    Serial.print("executing command: ");
    Serial.print(current_throttle);
    Serial.print(" ");
    Serial.println(current_rudder_angle);
    int angle = map((int)current_throttle, 0, 100, 0, 180);
    Serial.print("esc angle: ");
    Serial.println(angle);
    esc.write(angle);
    rudder_servo.write((int)current_rudder_angle);

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
    // rf95.send(send_buffer, sizeof(msg));
    // last_send_time = millis();
    LoRa.beginPacket();
    LoRa.write(send_buffer, sizeof(msg));
    LoRa.endPacket();
    break;
  }
  case BoatState::Recovery: {
    Serial.println("Received command during recovery, transitioning to teleop");
    state = BoatState::Teleop;
  }
  }
}