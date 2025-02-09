#include <stdint.h>

enum MessageType: uint8_t {
    Command = 0,
    Reply = 1
};

struct Header {
    MessageType type;
    uint8_t source_id;
    uint8_t dest_id;
    uint8_t sequence_number;
    uint32_t time_sec;
    uint32_t time_nsec;
};

struct CommandMessage {
    Header header;
    float throttle;
    float rudder_angle;
};

struct ReplyMessage {
    Header header;
    float heading;
    float speed;
};