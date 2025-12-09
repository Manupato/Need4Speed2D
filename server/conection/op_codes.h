#ifndef OP_CODES_H
#define OP_CODES_H

#include <cstdint>

static constexpr uint8_t INPUT_KEY = 0x12;
static constexpr uint8_t CREATE_LOBBY = 0x16;
static constexpr uint8_t JOIN_LOBBY = 0x17;
static constexpr uint8_t START_LOBBY = 0x22;
static constexpr uint8_t CMD_UPGRADE = 0x33;
static constexpr uint8_t CMD_DISCONNECT = 0x34;
static constexpr uint8_t EVENT_SEND_SNAPSHOT = 0x01;
static constexpr uint8_t EVENT_SEND_ID = 0x15;
static constexpr uint8_t EVENT_LOBBY_JOIN_ERROR = 0x20;
static constexpr uint8_t EVENT_LOBBY_SNAPSHOT = 0x21;
static constexpr uint8_t EVENT_START_LOBBY = 0x22;
static constexpr uint8_t EVENT_PRE_GAME_SNAPSHOT = 0x23;
static constexpr uint8_t EVENT_RACE_RESULTS = 0x24;
static constexpr uint8_t EVENT_EXIT_JOIN = 0x30;
static constexpr uint8_t EVENT_PHASE_CHANGE = 0x32;


#endif  // OP_CODES_H
