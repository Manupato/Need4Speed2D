#include <arpa/inet.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../server/conection/server_protocol.h"
#include "../server/event.h"

#include "MockSocket.h"

using ::testing::_;
using ::testing::Return;

TEST(ServerProtocolTest, ParseJoinLobbyGetCmd) {
    MockSocket mock;
    ServerProtocol protocol;

    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = 0x17;  // JOIN_LOBBY
        return 1;
    });

    EXPECT_EQ(protocol.get_type_of_command(mock), CommandReceiverType::JoinLobby);
}

TEST(ServerProtocolTest, ParseCreateLobbyGetCmd) {
    MockSocket mock;
    ServerProtocol protocol;

    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = 0x16;  // CREATE_LOBBY
        return 1;
    });

    EXPECT_EQ(protocol.get_type_of_command(mock), CommandReceiverType::CreateLobby);
}

TEST(ServerProtocolTest, ParseExitLobbyGetCmd) {
    MockSocket mock;
    ServerProtocol protocol;

    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = 0x34;  // DISCONNECT opcode
        return 1;
    });

    EXPECT_EQ(protocol.get_type_of_command(mock), CommandReceiverType::Disconect);
}

TEST(ServerProtocolTest, ParseMoveGetCmd) {
    MockSocket mock;
    ServerProtocol protocol;

    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = 0x12;  // INPUT_KEY opcode
        return 1;
    });

    EXPECT_EQ(protocol.get_type_of_command(mock), CommandReceiverType::Move);
}

TEST(ServerProtocolTest, ParseStartLobbyGetCmd) {
    MockSocket mock;
    ServerProtocol protocol;

    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = 0x22;  // START_LOBBY opcode
        return 1;
    });

    EXPECT_EQ(protocol.get_type_of_command(mock), CommandReceiverType::StartLobby);
}

TEST(ServerProtocolTest, ParseCmdUpgradeGetCmd) {
    MockSocket mock;
    ServerProtocol protocol;

    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = 0x33;  // CMD_UPGRADE opcode
        return 1;
    });

    EXPECT_EQ(protocol.get_type_of_command(mock), CommandReceiverType::Upgrade);
}

TEST(ServerProtocolTest, ParseMoveCommand) {
    MockSocket mock;
    ServerProtocol protocol;

    using ::testing::_;
    using ::testing::InSequence;

    InSequence seq;

    // El opcode debe ser INPUT_KEY
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = INPUT_KEY;
        return 1;
    });

    auto type = protocol.get_type_of_command(mock);
    ASSERT_EQ(type, CommandReceiverType::Move);

    // La direccion del movimiento: 0x03
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = 0x03;
        return 1;
    });

    auto cmd = protocol.get_command_move(mock, 10);

    // El comando tendra: El id del jugador, su tipo de comando (move) y el parametro (direccion)
    EXPECT_EQ(cmd.client_id, 10);
    EXPECT_EQ(cmd.type, CommandReceiverType::Move);
    EXPECT_EQ(cmd.param, 0x03);
}

TEST(ServerProtocolTest, ParseJoinLobby) {
    MockSocket mock;
    ServerProtocol protocol;

    using ::testing::_;
    using ::testing::InSequence;

    InSequence seq;

    // OPCODE JOIN_LOBBY
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = JOIN_LOBBY;
        return 1;
    });

    // 4 bytes para el id del lobby. lobby_id = 9999
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned int) {
        // Internamente mi protocolo hace ntohl, entonces el socket lo devuelve como network long
        uint32_t v = htonl(9999);
        memcpy(b, &v, 4);
        return 4;
    });

    // model_car = 7
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = 7;
        return 1;
    });

    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned int) {
        // Tamaño del string "Jose"
        uint16_t len = htons(4);
        memcpy(b, &len, 2);
        return 2;
    });

    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned int) {
        memcpy(b, "Jose", 4);
        return 4;
    });

    auto type = protocol.get_type_of_command(mock);
    ASSERT_EQ(type, CommandReceiverType::JoinLobby);

    auto cmd = protocol.get_command_join_lobby(mock, 44);

    EXPECT_EQ(cmd.client_id, 44);
    EXPECT_EQ(cmd.type, CommandReceiverType::JoinLobby);
    EXPECT_EQ(cmd.id_lobby, 9999);
    EXPECT_EQ(cmd.model_car, 7);
    EXPECT_EQ(cmd.name, "Jose");
}

TEST(ServerProtocolTest, ParseUpgradeCommand) {
    MockSocket mock;
    ServerProtocol protocol;

    using ::testing::_;
    using ::testing::InSequence;

    InSequence seq;

    // El opcode debe ser CMD_UPGRADE
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = CMD_UPGRADE;
        return 1;
    });

    auto type = protocol.get_type_of_command(mock);
    ASSERT_EQ(type, CommandReceiverType::Upgrade);

    // La mejora: 0x03
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = 0x03;
        return 1;
    });

    auto cmd = protocol.get_command_upgrade(mock, 33);

    // El comando tendra: El id del jugador, su tipo de comando (move) y el parametro (que mejora realiza)
    EXPECT_EQ(cmd.client_id, 33);
    EXPECT_EQ(cmd.type, CommandReceiverType::Upgrade);
    EXPECT_EQ(cmd.param, 0x03);
}

TEST(ServerProtocolTest, ParseCreateLobby) {
    MockSocket mock;
    ServerProtocol protocol;

    using ::testing::_;
    using ::testing::InSequence;

    InSequence seq;

    // OPCODE CREATE_LOBBY
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = CREATE_LOBBY;
        return 1;
    });

    // model_car = 5
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = 5;
        return 1;
    });

    // name len = 3
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned int) {
        uint16_t len = htons(3);
        memcpy(b, &len, 2);
        return 2;
    });

    // name = "Ana"
    EXPECT_CALL(mock, recvall(_, 3)).WillOnce([](void* b, unsigned int) {
        memcpy(b, "Ana", 3);
        return 3;
    });

    // maps_size = 2
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned int) {
        uint16_t v = htons(2);
        memcpy(b, &v, 2);
        return 2;
    });

    // mapa 1: "city"
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned int) {
        uint16_t len = htons(4);
        memcpy(b, &len, 2);
        return 2;
    });
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned int) {
        memcpy(b, "city", 4);
        return 4;
    });

    // mapa 2: "desert"
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned int) {
        uint16_t len = htons(6);
        memcpy(b, &len, 2);
        return 2;
    });
    EXPECT_CALL(mock, recvall(_, 6)).WillOnce([](void* b, unsigned int) {
        memcpy(b, "desert", 6);
        return 6;
    });

    auto type = protocol.get_type_of_command(mock);
    ASSERT_EQ(type, CommandReceiverType::CreateLobby);

    auto cmd = protocol.get_command_create_lobby(mock, 99);

    EXPECT_EQ(cmd.client_id, 99);
    EXPECT_EQ(cmd.type, CommandReceiverType::CreateLobby);

    EXPECT_EQ(cmd.model_car, 5);
    EXPECT_EQ(cmd.name, "Ana");

    ASSERT_EQ(cmd.maps.size(), 2);
    EXPECT_EQ(cmd.maps[0], "city");
    EXPECT_EQ(cmd.maps[1], "desert");
}


TEST(ServerProtocolTest, ParseStartLobby) {
    MockSocket mock;
    ServerProtocol protocol;

    using ::testing::InSequence;
    InSequence seq;

    // OPCODE START_LOBBY
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = START_LOBBY;
        return 1;
    });

    // lobby_id = 555
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned int) {
        uint32_t v = htonl(555);
        memcpy(b, &v, 4);
        return 4;
    });

    auto type = protocol.get_type_of_command(mock);
    ASSERT_EQ(type, CommandReceiverType::StartLobby);

    auto cmd = protocol.get_command_start_lobby(mock, 7);

    EXPECT_EQ(cmd.client_id, 7);
    EXPECT_EQ(cmd.type, CommandReceiverType::StartLobby);
    EXPECT_EQ(cmd.lobby_id, 555);
}

TEST(ServerProtocolTest, SendIDToClient) {
    MockSocket mock;
    ServerProtocol protocol;

    using ::testing::_;
    using ::testing::ElementsAreArray;

    int id = 12345;
    uint32_t id_be = htonl(id);

    // Esperamos que sendall reciba exactamente 5 bytes: [opcode, id_be]
    EXPECT_CALL(mock, sendall(_, 5)).WillOnce([&](const void* data, unsigned int size) {
        const uint8_t* buf = reinterpret_cast<const uint8_t*>(data);

        EXPECT_EQ(buf[0], EVENT_SEND_ID);

        uint32_t extracted;
        memcpy(&extracted, buf + 1, 4);
        // Lo que se mando por el socket, tiene que estar en network long 
        // (por lo que lo convierto a host long para comprobarlo con mi id local)
        EXPECT_EQ(ntohl(extracted), (uint32_t)id);

        return 5;
    });

    protocol.send_id_to_client(mock, id);
}

TEST(ServerProtocolTest, SendLobbySnapshot) {
    MockSocket mock;
    ServerProtocol protocol;

    LobbySnapshotData lobby;
    lobby.lobby_id = 777;
    lobby.lobby_players = {{"Juan", 3}, {"Ana", 5}};

    // Calculamos el tamaño esperado del buffer:
    // 1 opcode + 4 id + 2 count + jugador1(2+4+1) + jugador2(2+3+1)
    size_t expected_size = 1 + 4 + 2 + (2 + 4 + 1) + (2 + 3 + 1);

    EXPECT_CALL(mock, sendall(_, expected_size)).WillOnce([&](const void* data, unsigned int size) {
        const uint8_t* buf = reinterpret_cast<const uint8_t*>(data);
        int pos = 0;

        EXPECT_EQ(buf[pos++], EVENT_LOBBY_SNAPSHOT);

        uint32_t id;
        memcpy(&id, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(id), 777);

        uint16_t count;
        memcpy(&count, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(count), 2);

        // Player 1
        uint16_t len1;
        memcpy(&len1, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(len1), 4);
        EXPECT_EQ(std::string((char*)buf + pos, 4), "Juan");
        pos += 4;
        EXPECT_EQ(buf[pos++], 3);

        // Player 2
        uint16_t len2;
        memcpy(&len2, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(len2), 3);
        EXPECT_EQ(std::string((char*)buf + pos, 3), "Ana");
        pos += 3;
        EXPECT_EQ(buf[pos++], 5);

        return expected_size;
    });


    auto* q = new Queue<std::shared_ptr<IEvent>>();
    auto ev = std::make_shared<LobbySnapshotEvent>(std::move(lobby));
    q->push(ev);
    EXPECT_TRUE(protocol.send_event_to_client(mock, *q));
}

TEST(ServerProtocolTest, SendJoinError) {
    MockSocket mock;
    ServerProtocol protocol;

    EXPECT_CALL(mock, sendall(_, 1)).WillOnce([](const void* data, unsigned int) {
        EXPECT_EQ(reinterpret_cast<const uint8_t*>(data)[0], EVENT_LOBBY_JOIN_ERROR);
        return 1;
    });

    auto* q = new Queue<std::shared_ptr<IEvent>>();
    auto ev = std::make_shared<JoinErrorEvent>();
    q->push(ev);

    EXPECT_TRUE(protocol.send_event_to_client(mock, *q));
}

TEST(ServerProtocolTest, SendStartLobby) {
    MockSocket mock;
    ServerProtocol protocol;

    EXPECT_CALL(mock, sendall(_, 1)).WillOnce([](const void* data, unsigned int) {
        EXPECT_EQ(reinterpret_cast<const uint8_t*>(data)[0], EVENT_START_LOBBY);
        return 1;
    });

    auto* q = new Queue<std::shared_ptr<IEvent>>();
    auto ev = std::make_shared<StartLobbyEvent>();
    q->push(ev);

    EXPECT_TRUE(protocol.send_event_to_client(mock, *q));
}

TEST(ServerProtocolTest, SendExitJoin) {
    MockSocket mock;
    ServerProtocol protocol;

    EXPECT_CALL(mock, sendall(_, 1)).WillOnce([](const void* data, unsigned int) {
        EXPECT_EQ(reinterpret_cast<const uint8_t*>(data)[0], EVENT_EXIT_JOIN);
        return 1;
    });

    auto* q = new Queue<std::shared_ptr<IEvent>>();
    auto ev = std::make_shared<ExitJoinEvent>();
    q->push(ev);


    EXPECT_TRUE(protocol.send_event_to_client(mock, *q));
}

TEST(ServerProtocolTest, SendPhaseChange) {
    MockSocket mock;
    ServerProtocol protocol;

    EXPECT_CALL(mock, sendall(_, 1)).WillOnce([](const void* data, unsigned int) {
        EXPECT_EQ(reinterpret_cast<const uint8_t*>(data)[0], EVENT_PHASE_CHANGE);
        return 1;
    });

    auto* q = new Queue<std::shared_ptr<IEvent>>();
    auto ev = std::make_shared<PhaseChangeEvent>();
    q->push(ev);

    EXPECT_TRUE(protocol.send_event_to_client(mock, *q));
}

TEST(ServerProtocolTest, PreGameSnapshot) {
    MockSocket mock;
    ServerProtocol protocol;

    PoleCoordsAndDirec pole;
    pole.coord_up_left = Coord{10, 20};
    pole.coord_down_right = Coord{100, 200};
    pole.direc = 0;

    PreGameSnapshotData pre_game;
    pre_game.remaining_races = 2;
    pre_game.map_id = MapId::LibertyCity;
    pre_game.pole = pole;
    pre_game.race_total_time_seconds = 120;
    pre_game.race_move_enabled_time_seconds = 110;

    size_t expected_size = 1 + 4 + 4 + 4 + 4 + 1 + 2 + 1 + 4 + 4;

    EXPECT_CALL(mock, sendall(_, expected_size)).WillOnce([&](const void* data, unsigned int size) {

        const uint8_t* buf = reinterpret_cast<const uint8_t*>(data);
        int pos = 0;

        EXPECT_EQ(buf[pos++], EVENT_PRE_GAME_SNAPSHOT);

        uint32_t coord_up_left_x_px;
        memcpy(&coord_up_left_x_px, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(coord_up_left_x_px), 10);

        uint32_t coord_up_left_y_px;
        memcpy(&coord_up_left_y_px, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(coord_up_left_y_px), 20);

        uint32_t coord_down_right_x_px;
        memcpy(&coord_down_right_x_px, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(coord_down_right_x_px), 100);

        uint32_t coord_down_right_y_px;
        memcpy(&coord_down_right_y_px, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(coord_down_right_y_px), 200);

        uint8_t direc;
        memcpy(&direc, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(direc, 0);

        uint16_t remaining_races;
        memcpy(&remaining_races, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(remaining_races), 2);

        uint8_t map_id;
        memcpy(&map_id, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(map_id, 0);

        uint32_t race_total_time_seconds;
        memcpy(&race_total_time_seconds, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(race_total_time_seconds), 120);

        uint32_t race_move_enabled_time_seconds;
        memcpy(&race_move_enabled_time_seconds, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(race_move_enabled_time_seconds), 110);

        return expected_size;
    });

    auto* q = new Queue<std::shared_ptr<IEvent>>();
    auto ev = std::make_shared<PreGameSnapshotEvent>(std::move(pre_game));
    q->push(ev);

    EXPECT_TRUE(protocol.send_event_to_client(mock, *q));
}

TEST(ServerProtocolTest, SendGameSnapshot) {
    MockSocket mock;
    ServerProtocol protocol;

    GameSnapshotData game;

    game.time_seconds_remained = 42;

    PlayerSnapshot p;
    p.id = 10;
    p.ghost = 0;
    p.car_life = 100;
    p.model = 2;
    p.animation = 1;
    p.sound_code = 3;
    p.x_px = 1000;
    p.y_px = 2000;
    p.z = 1;
    p.angle = 90;
    p.next_checkpoint = {Coord{3000, 4000}};
    p.there_is_second_checkpoint = 1;
    p.next_next_checkpoint = {Coord{5000, 7000}};
    p.goal = 0;
    p.next_next_goal = 0;

    game.players.clear();
    game.players.push_back(p);

    NpcSnapshot npc;
    npc.model = 0;
    npc.animation = 1;
    npc.x_px = 2;
    npc.y_px = 3;
    npc.z = 4;
    npc.angle = 5;

    game.npcs.clear();
    game.npcs.push_back(npc);

    size_t expected_size = 1 + 4 + 2;
    // El jugador en el test va a sumar: 
    expected_size += (4 + 1 + 2 + 2 + 1 + 1 + 4 + 4 + 1 + 4 + 2 + 4 + 4 + 1 + 1 + 2 + 4 + 4 + 1);
    // 2 bytes para indicar que hay 1 npc
    expected_size += 2;
    // + Lo que suma el npc
    expected_size += (2 + 1 + 4 + 4 + 1 + 4);

    EXPECT_CALL(mock, sendall(_, expected_size)).WillOnce([&](const void* data, unsigned int size) {

        const uint8_t* buf = reinterpret_cast<const uint8_t*>(data);
        int pos = 0;

        EXPECT_EQ(buf[pos++], EVENT_SEND_SNAPSHOT);

        uint32_t time_seconds_remained;
        memcpy(&time_seconds_remained, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(time_seconds_remained), 42);

        uint16_t size_players;
        memcpy(&size_players, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(size_players), 1);

        uint32_t id;
        memcpy(&id, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(id), 10);

        uint8_t ghost;
        memcpy(&ghost, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(ghost, 0);

        uint16_t car_life;
        memcpy(&car_life, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(car_life), 100);

        uint16_t model;
        memcpy(&model, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(model), 2);

        uint8_t animation;
        memcpy(&animation, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(animation, 1);

        uint8_t sound_code;
        memcpy(&sound_code, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(sound_code, 3);

        uint32_t x_px;
        memcpy(&x_px, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(x_px), 1000);

        uint32_t y_px;
        memcpy(&y_px, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(y_px), 2000);

        uint8_t z;
        memcpy(&z, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(z, 1);

        uint32_t angle;
        memcpy(&angle, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(angle), 90);

        uint16_t next_checkpoint;
        memcpy(&next_checkpoint, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(next_checkpoint), 1);

        uint32_t x_px_checkpoint;
        memcpy(&x_px_checkpoint, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(x_px_checkpoint), 3000);

        uint32_t y_px_checkpoint;
        memcpy(&y_px_checkpoint, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(y_px_checkpoint), 4000);

        uint8_t goal;
        memcpy(&goal, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(goal, 0);

        uint8_t there_is_second_checkpoint;
        memcpy(&there_is_second_checkpoint, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(there_is_second_checkpoint, 1);

        uint16_t next_next_checkpoint;
        memcpy(&next_next_checkpoint, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(next_next_checkpoint), 1);

        uint32_t x_px_next_next_checkpoint;
        memcpy(&x_px_next_next_checkpoint, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(x_px_next_next_checkpoint), 5000);

        uint32_t y_px_next_next_checkpoint;
        memcpy(&y_px_next_next_checkpoint, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(y_px_next_next_checkpoint), 7000);

        uint8_t goal_next_next;
        memcpy(&goal_next_next, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(goal_next_next, 0);

        uint16_t size_npcs;
        memcpy(&size_npcs, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(size_npcs), 1);

        uint16_t npc_model;
        memcpy(&npc_model, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(npc_model), 0);

        uint8_t animation_npc;
        memcpy(&animation_npc, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(animation_npc, 1);

        uint32_t x_npc;
        memcpy(&x_npc, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(x_npc), 2);

        uint32_t y_npc;
        memcpy(&y_npc, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(y_npc), 3);

        uint8_t z_npc;
        memcpy(&z_npc, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(z_npc, 4);

        uint32_t angle_npc;
        memcpy(&angle_npc, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(angle_npc), 5);

        return expected_size;
    });

    auto* q = new Queue<std::shared_ptr<IEvent>>();
    auto ev = std::make_shared<GameSnapshotEvent>(std::move(game));
    q->push(ev);

    EXPECT_TRUE(protocol.send_event_to_client(mock, *q));
}

TEST(ServerProtocolTest, SendRaceResults) {
    MockSocket mock;
    ServerProtocol protocol;

    RaceResultsData rr;
    rr.last_race = 0;

    PlayerRaceResult r1;
    r1.id = 10;
    r1.name = "Juan";
    r1.race_time_seconds = 100;
    r1.total_time_seconds = 300;
    r1.status = 1;

    PlayerRaceResult r2;
    r2.id = 20;
    r2.name = "Ana";
    r2.race_time_seconds = 110;
    r2.total_time_seconds = 320;
    r2.status = 0;

    rr.race_results.clear();
    rr.race_results.push_back(r1);
    rr.race_results.push_back(r2);

    // El tamaño esperado sera:
    size_t expected_size = (1 + 1 + 2);
    // Le sumamos lo de Juan:
    expected_size += (2 + 4 + 4 + 4 + 1);
    // Le sumamos lo de Ana:
    expected_size += (2 + 3 + 4 + 4 + 1);


    EXPECT_CALL(mock, sendall(_, expected_size)).WillOnce([&](const void* data, unsigned int size) {

        const uint8_t* buf = reinterpret_cast<const uint8_t*>(data);
        int pos = 0;

        EXPECT_EQ(buf[pos++], EVENT_RACE_RESULTS);

        uint8_t last_race;
        memcpy(&last_race, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(last_race, 0);

        uint16_t count;
        memcpy(&count, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(count), 2);

        // Player 1
        uint16_t len1;
        memcpy(&len1, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(len1), 4);
        EXPECT_EQ(std::string((char*)buf + pos, 4), "Juan");
        pos += 4;

        uint32_t race_time_seconds;
        memcpy(&race_time_seconds, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(race_time_seconds), 100);

        uint32_t total_time_seconds;
        memcpy(&total_time_seconds, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(total_time_seconds), 300);

        uint8_t status_p1;
        memcpy(&status_p1, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(status_p1, 1);

        // Player 2
        uint16_t len2;
        memcpy(&len2, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(len2), 3);
        EXPECT_EQ(std::string((char*)buf + pos, 3), "Ana");
        pos += 3;

        uint32_t race_time_seconds2;
        memcpy(&race_time_seconds2, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(race_time_seconds2), 110);

        uint32_t total_time_seconds2;
        memcpy(&total_time_seconds2, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(total_time_seconds2), 320);

        uint8_t status_p2;
        memcpy(&status_p2, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(status_p2, 0);

        return expected_size;
    });

    auto* q = new Queue<std::shared_ptr<IEvent>>();
    auto ev = std::make_shared<RaceResultsEvent>(std::move(rr));
    q->push(ev);

    EXPECT_TRUE(protocol.send_event_to_client(mock, *q));
}

TEST(ServerProtocolTest, SendRaceResultsLast) {
    MockSocket mock;
    ServerProtocol protocol;

    RaceResultsData rr;
    rr.last_race = 1;
    rr.podium_count = 2;

    PlayerRaceResult r1;
    r1.id = 10;
    r1.name = "Juan";
    r1.race_time_seconds = 100;
    r1.total_time_seconds = 300;
    r1.status = 1;

    PlayerRaceResult r2;
    r2.id = 20;
    r2.name = "Ana";
    r2.race_time_seconds = 110;
    r2.total_time_seconds = 320;
    r2.status = 0;

    PlayerRaceResult r3;
    r3.id = 30;
    r3.name = "Marcos";
    r3.race_time_seconds = 300;
    r3.total_time_seconds = 400;
    r3.status = 1;

    PlayerRaceResult r4;
    r4.id = 40;
    r4.name = "Polo";
    r4.race_time_seconds = 500;
    r4.total_time_seconds = 600;
    r4.status = 0;

    rr.race_results.clear();
    rr.race_results.push_back(r1);
    rr.race_results.push_back(r2);
    rr.race_results.push_back(r3);
    rr.race_results.push_back(r4);

    // El tamaño esperado sera:
    size_t expected_size = (1 + 1 + 2);
    // Le sumamos lo Polo (No podio)
    expected_size += (2 + 4 + 4 + 4 + 1);
    // Ahora vamos con el podio
    expected_size += 1; // Flag del podio para que el cliente sepa cuantos mostrar
    // Le sumamos lo de Juan:
    expected_size += (1 + 2 + 4 + 4 + 4 + 1);
    // Le sumamos lo de Ana:
    expected_size += (1 + 2 + 3 + 4 + 4 + 1);
    // Le sumamos lo de Marcos:
    expected_size += (1 + 2 + 6 + 4 + 4 + 1);

    EXPECT_CALL(mock, sendall(_, expected_size)).WillOnce([&](const void* data, unsigned int size) {

        const uint8_t* buf = reinterpret_cast<const uint8_t*>(data);
        int pos = 0;

        EXPECT_EQ(buf[pos++], EVENT_RACE_RESULTS);
        EXPECT_EQ(buf[pos++], 0X01);    // lastRace flag

        uint16_t count_no_podium;
        memcpy(&count_no_podium, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(count_no_podium), 1);

        // Player Polo
        uint16_t len1;
        memcpy(&len1, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(len1), 4);
        EXPECT_EQ(std::string((char*)buf + pos, 4), "Polo");
        pos += 4;

        uint32_t race_time_seconds;
        memcpy(&race_time_seconds, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(race_time_seconds), 500);

        uint32_t total_time_seconds;
        memcpy(&total_time_seconds, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(total_time_seconds), 600);

        uint8_t status_p1;
        memcpy(&status_p1, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(status_p1, 0);

        // Aca le digo al cliente "Ey yo te voy a mandar los TRES players del podio pero vos solo mostra
        // DOS". Esto lo hacemos para la animacion de la ultima carrera, yo (sv) le mando 4 snapshots distintas
        // Sin podio, solo mostra el tercer pj -> solo el segundo y tercero -> mostrar todos.
        EXPECT_EQ(buf[pos++], 2);

        // Player Juan
        EXPECT_EQ(buf[pos++], 1);   // Juan EXISTE

        uint16_t len2;
        memcpy(&len2, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(len2), 4);
        EXPECT_EQ(std::string((char*)buf + pos, 4), "Juan");
        pos += 4;

        uint32_t race_time_seconds2;
        memcpy(&race_time_seconds2, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(race_time_seconds2), 100);

        uint32_t total_time_seconds2;
        memcpy(&total_time_seconds2, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(total_time_seconds2), 300);

        uint8_t status_p2;
        memcpy(&status_p2, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(status_p2, 1);

        // Player Ana
        EXPECT_EQ(buf[pos++], 1);   // Ana EXISTE

        uint16_t len3;
        memcpy(&len3, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(len3), 3);
        EXPECT_EQ(std::string((char*)buf + pos, 3), "Ana");
        pos += 3;

        uint32_t race_time_seconds3;
        memcpy(&race_time_seconds3, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(race_time_seconds3), 110);

        uint32_t total_time_seconds3;
        memcpy(&total_time_seconds3, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(total_time_seconds3), 320);

        uint8_t status_p3;
        memcpy(&status_p3, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(status_p3, 0);

        // Player Marcos
        EXPECT_EQ(buf[pos++], 1);   // Marcos EXISTE

        uint16_t len4;
        memcpy(&len4, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(len4), 6);
        EXPECT_EQ(std::string((char*)buf + pos, 6), "Marcos");
        pos += 6;

        uint32_t race_time_seconds4;
        memcpy(&race_time_seconds4, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(race_time_seconds4), 300);

        uint32_t total_time_seconds4;
        memcpy(&total_time_seconds4, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(total_time_seconds4), 400);

        uint8_t status_p4;
        memcpy(&status_p4, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(status_p4, 1);

        return expected_size;
    });

    auto* q = new Queue<std::shared_ptr<IEvent>>();
    auto ev = std::make_shared<RaceResultsLastEvent>(std::move(rr));
    q->push(ev);

    EXPECT_TRUE(protocol.send_event_to_client(mock, *q));
}

TEST(ServerProtocolTest, SendRaceResultsLastOnlyOnePlayer) {
    MockSocket mock;
    ServerProtocol protocol;

    RaceResultsData rr;
    rr.last_race = 1;
    rr.podium_count = 3;

    PlayerRaceResult r1;
    r1.id = 10;
    r1.name = "Juan";
    r1.race_time_seconds = 100;
    r1.total_time_seconds = 300;
    r1.status = 1;

    rr.race_results.clear();
    rr.race_results.push_back(r1);

    // El tamaño esperado sera:
    size_t expected_size = (1 + 1 + 2);
    // Ahora vamos con el podio
    expected_size += 1; // Flag del podio para que el cliente sepa cuantos mostrar
    // Le sumamos lo de Juan:
    expected_size += (1 + 2 + 4 + 4 + 4 + 1);
    // Player 2 y 3 NO existen:
    expected_size += (1 + 1);

    EXPECT_CALL(mock, sendall(_, expected_size)).WillOnce([&](const void* data, unsigned int size) {

        const uint8_t* buf = reinterpret_cast<const uint8_t*>(data);
        int pos = 0;

        EXPECT_EQ(buf[pos++], EVENT_RACE_RESULTS);
        EXPECT_EQ(buf[pos++], 0X01);    // lastRace flag

        uint16_t count_no_podium;
        memcpy(&count_no_podium, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(count_no_podium), 0);

        EXPECT_EQ(buf[pos++], 3); // flag de cuantos mostrar en el podio

        // Player Juan
        EXPECT_EQ(buf[pos++], 1);   // Juan EXISTE

        uint16_t len2;
        memcpy(&len2, buf + pos, 2);
        pos += 2;
        EXPECT_EQ(ntohs(len2), 4);
        EXPECT_EQ(std::string((char*)buf + pos, 4), "Juan");
        pos += 4;

        uint32_t race_time_seconds2;
        memcpy(&race_time_seconds2, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(race_time_seconds2), 100);

        uint32_t total_time_seconds2;
        memcpy(&total_time_seconds2, buf + pos, 4);
        pos += 4;
        EXPECT_EQ(ntohl(total_time_seconds2), 300);

        uint8_t status_p2;
        memcpy(&status_p2, buf + pos, 1);
        pos += 1;
        EXPECT_EQ(status_p2, 1);

        // Player 2
        EXPECT_EQ(buf[pos++], 0);   // 2 NO EXISTE

        // Player 3
        EXPECT_EQ(buf[pos++], 0);   // 3 NO EXISTE

        return expected_size;
    });

    auto* q = new Queue<std::shared_ptr<IEvent>>();
    auto ev = std::make_shared<RaceResultsLastEvent>(std::move(rr));
    q->push(ev);

    EXPECT_TRUE(protocol.send_event_to_client(mock, *q));
}
