#ifndef PROTOCOL_CLIENT_H
#define PROTOCOL_CLIENT_H

#include <string>
#include <vector>

#include "../common/ISocket.h"
#include "../common/operations_bytes.h"

#include "ServerEvent.h"

const int8_t SEND_CREATE_LOBBY = 0X16;
const int8_t SEND_JOIN_LOBBY = 0X17;
const int8_t SEND_START_GAME = 0X22;
const uint8_t SEND_CAR_UPGRADE = 0X33;
const uint8_t SEND_LEAVE = 0X34;

const uint8_t INPUT_KEY = 0x12;
// Keys luego de input_key
const uint8_t UP_PRESSED = 0x00;
const uint8_t LEFT_PRESSED = 0x01;
const uint8_t DOWN_PRESSED = 0x02;
const uint8_t RIGHT_PRESSED = 0x03;
const uint8_t UP_UNPRESSED = 0x04;
const uint8_t LEFT_UNPRESSED = 0x05;
const uint8_t DOWN_UNPRESSED = 0x06;
const uint8_t RIGHT_UNPRESSED = 0x07;
const uint8_t COMMAND_WIN = 0x08;
const uint8_t COMMAND_LOSE = 0x09;
const uint8_t COMMAND_INFINITE_LIFE = 0x10;
const uint8_t COMMAND_GHOST = 0x11;


const uint8_t RECEIVE_SNAPSHOT = 0x01;
const uint8_t RECEIVE_ID = 0x15;
const uint8_t RECEIVE_JOIN_ERROR = 0x20;
const uint8_t RECEIVE_SNAPSHOT_LOBBY = 0x21;
const uint8_t RECEIVE_START_GAME = 0x22;

const uint8_t RECEIVE_PREGAME_SNAPSHOT = 0x23;
const uint8_t RIGHT = 0X01;
const uint8_t LEFT = 0X02;
const uint8_t UP = 0X03;
const uint8_t DOWN = 0X04;

const uint8_t RECEIVE_RACE_RESULTS = 0x24;
const uint8_t RECEIVE_SUCESS = 0x30;
const uint8_t RECEIVE_CHANGE_FASE = 0x32;

class ProtocolClient {

private:
    ISocket& skt;

    OperationsBytes operation;

    std::vector<uint8_t> send_key(SendKey send_key);

    std::vector<uint8_t> send_create_lobby(CreateToLobby snapshot);

    std::vector<uint8_t> send_join_lobby(JoinToLobby snapshot);

    std::vector<uint8_t> send_start_game(StartGame start_game);

    std::vector<uint8_t> send_upgrade_car(CarUpgrades car_upgrade);


    ServerEventReceiver receive_snapshot_lobby();

    ServerEventReceiver receive_snapshot();

    ServerEventReceiver receive_id();

    ServerEventReceiver receive_pre_game_snapshot();

    ServerEventReceiver receive_race_results();

public:
    explicit ProtocolClient(ISocket& skt);

    void send_event(const ServerEventSender& key_ingresada);

    ServerEventReceiver receive_event(bool& server_event);
};

#endif
