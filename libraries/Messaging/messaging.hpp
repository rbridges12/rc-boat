#include <stdint.h>

enum class StationState : uint8_t {
    Off = 0,
    Teleop = 1
};

enum class BoatState : uint8_t {
    Off = 0,
    Teleop = 1,
    Recovery = 2
};

enum class MessageType: uint8_t {
    Command = 0,
    Reply = 1
};

struct Header {
    uint32_t sequence_number;
    uint32_t time_sec;
    uint32_t time_nsec;
    MessageType type;
    uint8_t source_id;
    uint8_t dest_id;
    uint8_t padding;
};

struct CommandMessage {
    Header header;
    float throttle;
    float rudder_angle;
};

struct ReplyMessage {
    Header header;
    float current_throttle;
    float current_rudder_angle;
    BoatState state;
    uint8_t padding[3];
};

constexpr int test = sizeof(Header);