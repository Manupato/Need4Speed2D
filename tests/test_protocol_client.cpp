#include <arpa/inet.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../client/ProtocolClient.h"

#include "MockSocket.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;

TEST(ProtocolClientTest, ReceiveSnapshotLobby) {
    MockSocket mock;
    ProtocolClient protocol(mock);
    bool closed = false;

    using ::testing::_;
    using ::testing::InSequence;

    InSequence seq;

    // 1) opcode = 0x21
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = 0x21;
        return 1;
    });

    // 2) lobby_code = 1234
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned int) {
        uint32_t v = htonl(1234);
        memcpy(b, &v, 4);
        return 4;
    });

    // 3) amount_players = 1
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned int) {
        uint16_t c = htons(1);
        memcpy(b, &c, 2);
        return 2;
    });

    // 4) name length = 4
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned int) {
        uint16_t len = htons(4);
        memcpy(b, &len, 2);
        return 2;
    });

    // 5) name “Juan” (una sola llamada!)
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned int) {
        memcpy(b, "Juan", 4);
        return 4;
    });

    // 6) car_model = 3
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = 3;
        return 1;
    });

    auto ev = protocol.receive_event(closed);

    ASSERT_EQ(ev.type, ServerEventReceiverType::SNAPSHOT_LOBBY);
    ASSERT_EQ(ev.snapshot_lobby.lobby_code, 1234);
    ASSERT_EQ(ev.snapshot_lobby.players_id.size(), 1);
    EXPECT_EQ(ev.snapshot_lobby.players_id[0].player_name, "Juan");
    EXPECT_EQ(ev.snapshot_lobby.players_id[0].car_model, 3);
}

TEST(ProtocolClientTest, ReceiveJoinError) {
    MockSocket mock;
    ProtocolClient protocol(mock);
    bool closed = false;

    using ::testing::InSequence;
    InSequence seq;

    // opcode = 0x20
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = 0x20;
        return 1;
    });

    auto ev = protocol.receive_event(closed);

    ASSERT_EQ(ev.type, ServerEventReceiverType::ERROR);
}

TEST(ProtocolClientTest, ReceiveID) {
    MockSocket mock;
    ProtocolClient protocol(mock);
    bool closed = false;

    using ::testing::InSequence;
    InSequence seq;

    // opcode SEND_ID = 0x10 (o tu valor real SEND_ID)
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = RECEIVE_ID;
        return 1;
    });

    // id = 999
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned int) {
        uint32_t v = htonl(999);
        memcpy(b, &v, 4);
        return 4;
    });

    auto ev = protocol.receive_event(closed);

    ASSERT_EQ(ev.type, ServerEventReceiverType::RECEIVE_ID);
    ASSERT_EQ(ev.id_jugador, 999);
}


TEST(ProtocolClientTest, ReceiveSnapshotBasic) {
    MockSocket mock;
    ProtocolClient protocol(mock);
    bool closed = false;

    using ::testing::InSequence;
    InSequence seq;

    // 1) opcode: SNAPSHOT
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = RECEIVE_SNAPSHOT;
        return 1;
    });

    // 2) actual_time = 555
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned int) {
        uint32_t v = htonl(555);
        memcpy(b, &v, 4);
        return 4;
    });

    // 3) players count = 1
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned int) {
        uint16_t c = htons(1);
        memcpy(b, &c, 2);
        return 2;
    });

    // ------ PLAYER ------
    EXPECT_CALL(mock, recvall(_, 4))  // id = 42
            .WillOnce([](void* b, unsigned int) {
                uint32_t v = htonl(42);
                memcpy(b, &v, 4);
                return 4;
            });

    EXPECT_CALL(mock, recvall(_, 1))  // ghost
            .WillOnce([](void* b, unsigned int) {
                reinterpret_cast<uint8_t*>(b)[0] = 1;  // true
                return 1;
            });

    EXPECT_CALL(mock, recvall(_, 2))  // car_life = 100
            .WillOnce([](void* b, unsigned int) {
                uint16_t v = htons(100);
                memcpy(b, &v, 2);
                return 2;
            });

    EXPECT_CALL(mock, recvall(_, 2))  // model = 5
            .WillOnce([](void* b, unsigned int) {
                uint16_t v = htons(5);
                memcpy(b, &v, 2);
                return 2;
            });

    EXPECT_CALL(mock, recvall(_, 1))  // animation
            .WillOnce([](void* b, unsigned int) {
                reinterpret_cast<uint8_t*>(b)[0] = 7;
                return 1;
            });

    EXPECT_CALL(mock, recvall(_, 1))  // sound STOP (=1)
            .WillOnce([](void* b, unsigned int) {
                reinterpret_cast<uint8_t*>(b)[0] = 1;
                return 1;
            });

    // coords x
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned int) {
        uint32_t v = htonl(1000);
        memcpy(b, &v, 4);
        return 4;
    });

    // coords y
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned int) {
        uint32_t v = htonl(2000);
        memcpy(b, &v, 4);
        return 4;
    });

    // z
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = 2;
        return 1;
    });

    // angle
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned int) {
        uint32_t v = htonl(90);
        memcpy(b, &v, 4);
        return 4;
    });

    // checkpoints count = 0
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned int) {
        uint16_t v = htons(0);
        memcpy(b, &v, 2);
        return 2;
    });

    // is_finishline
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = 1;
        return 1;
    });

    // is_secondary_check = 0 (no hay segundo checkpoint)
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned int) {
        reinterpret_cast<uint8_t*>(b)[0] = 0;
        return 1;
    });

    // NPC count = 0
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned int) {
        uint16_t v = htons(0);
        memcpy(b, &v, 2);
        return 2;
    });

    auto ev = protocol.receive_event(closed);

    ASSERT_EQ(ev.type, ServerEventReceiverType::SNAPSHOT);
    ASSERT_EQ(ev.snapshot.actual_time, 555);
    ASSERT_EQ(ev.snapshot.players.size(), 1);

    auto& p = ev.snapshot.players[0];
    EXPECT_EQ(p.user_id, 42);
    EXPECT_TRUE(p.is_car_ghost);
    EXPECT_EQ(p.car_life, 100);
    EXPECT_EQ(p.car_model, 5);
    EXPECT_EQ(p.car_animation, 7);
    EXPECT_EQ(p.type_sound, TypeSound::STOP);
    EXPECT_EQ(p.player_position.coord_x, 1000);
    EXPECT_EQ(p.player_position.coord_y, 2000);
    EXPECT_EQ(p.car_coord_z, 2);
    EXPECT_EQ(p.rotation, 90);
    EXPECT_EQ(p.next_checkpoint.size(), 0);
    EXPECT_TRUE(p.is_checkpoint_finishline);
    EXPECT_EQ(ev.snapshot.npcs.size(), 0);
}

TEST(ProtocolClientTest, ReceiveStartGame) {
    MockSocket mock;
    ProtocolClient protocol(mock);
    bool closed = false;

    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        reinterpret_cast<uint8_t*>(b)[0] = RECEIVE_START_GAME;
        return 1;
    });

    auto ev = protocol.receive_event(closed);

    ASSERT_EQ(ev.type, ServerEventReceiverType::START_GAME);
}

TEST(ProtocolClientTest, ReceiveSuccess) {
    MockSocket mock;
    ProtocolClient protocol(mock);
    bool closed = false;

    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        reinterpret_cast<uint8_t*>(b)[0] = RECEIVE_SUCESS;
        return 1;
    });

    auto ev = protocol.receive_event(closed);

    ASSERT_EQ(ev.type, ServerEventReceiverType::SUCESS);
}

TEST(ProtocolClientTest, ReceiveChangeFase) {
    MockSocket mock;
    ProtocolClient protocol(mock);
    bool closed = false;

    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        reinterpret_cast<uint8_t*>(b)[0] = RECEIVE_CHANGE_FASE;
        return 1;
    });

    auto ev = protocol.receive_event(closed);

    ASSERT_EQ(ev.type, ServerEventReceiverType::CHANGE_FASE);
}

TEST(ProtocolClientTest, ReceivePreGameSnapshot) {
    MockSocket mock;
    ProtocolClient protocol(mock);
    bool closed = false;

    InSequence seq;

    // opcode
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        reinterpret_cast<uint8_t*>(b)[0] = RECEIVE_PREGAME_SNAPSHOT;
        return 1;
    });

    // top_left.x = 100
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned) {
        uint32_t v = htonl(100);
        memcpy(b, &v, 4);
        return 4;
    });

    // top_left.y = 200
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned) {
        uint32_t v = htonl(200);
        memcpy(b, &v, 4);
        return 4;
    });

    // bottom_right.x = 300
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned) {
        uint32_t v = htonl(300);
        memcpy(b, &v, 4);
        return 4;
    });

    // bottom_right.y = 400
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned) {
        uint32_t v = htonl(400);
        memcpy(b, &v, 4);
        return 4;
    });

    // direction = UP (3)
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        reinterpret_cast<uint8_t*>(b)[0] = 3;
        return 1;
    });

    // pending_matches = 2 → no es último mapa
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned) {
        uint16_t v = htons(2);
        memcpy(b, &v, 2);
        return 2;
    });

    // map_selected = 1 → SanAndreas
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        reinterpret_cast<uint8_t*>(b)[0] = 1;
        return 1;
    });

    // game_total_time = 5000
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned) {
        uint32_t v = htonl(5000);
        memcpy(b, &v, 4);
        return 4;
    });

    // game_start_time = 123
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned) {
        uint32_t v = htonl(123);
        memcpy(b, &v, 4);
        return 4;
    });

    auto ev = protocol.receive_event(closed);

    ASSERT_EQ(ev.type, ServerEventReceiverType::PREGAME);

    auto& pg = ev.pre_snapshot;

    EXPECT_EQ(pg.start_line.top_left.coord_x, 100);
    EXPECT_EQ(pg.start_line.top_left.coord_y, 200);
    EXPECT_EQ(pg.start_line.bottom_right.coord_x, 300);
    EXPECT_EQ(pg.start_line.bottom_right.coord_y, 400);

    EXPECT_EQ(pg.start_line.direction, Direction::UP);
    EXPECT_FALSE(pg.is_last_map);

    EXPECT_EQ(pg.map_selected, Map::SanAndreas);
    EXPECT_EQ(pg.game_total_time, 5000);
    EXPECT_EQ(pg.game_start_time, 123);
}

TEST(ProtocolClientTest, ReceiveRaceResults) {
    MockSocket mock;
    ProtocolClient protocol(mock);
    bool closed = false;

    InSequence seq;

    // opcode
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        reinterpret_cast<uint8_t*>(b)[0] = RECEIVE_RACE_RESULTS;
        return 1;
    });

    // is_last_race = false
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        reinterpret_cast<uint8_t*>(b)[0] = 0x00;
        return 1;
    });

    // amount_players = 2
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned) {
        uint16_t v = htons(2);
        memcpy(b, &v, 2);
        return 2;
    });

    // --- PLAYER 1 ---
    // name length = 4 ("Juan")
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned) {
        uint16_t v = htons(4);
        memcpy(b, &v, 2);
        return 2;
    });

    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned) {
        memcpy(b, "Juan", 4);
        return 4;
    });

    // last_race_time = 1000
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned) {
        uint32_t v = htonl(1000);
        memcpy(b, &v, 4);
        return 4;
    });

    // total_race_time = 5000
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned) {
        uint32_t v = htonl(5000);
        memcpy(b, &v, 4);
        return 4;
    });

    // finished race = 1
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        reinterpret_cast<uint8_t*>(b)[0] = 1;
        return 1;
    });

    // --- PLAYER 2 ---
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned) {
        uint16_t v = htons(3);
        memcpy(b, &v, 2);
        return 2;
    });

    EXPECT_CALL(mock, recvall(_, 3)).WillOnce([](void* b, unsigned) {
        memcpy(b, "Ana", 3);
        return 3;
    });

    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned) {
        uint32_t v = htonl(777);
        memcpy(b, &v, 4);
        return 4;
    });

    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned) {
        uint32_t v = htonl(8888);
        memcpy(b, &v, 4);
        return 4;
    });

    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        reinterpret_cast<uint8_t*>(b)[0] = 0;
        return 1;
    });

    auto ev = protocol.receive_event(closed);

    ASSERT_EQ(ev.type, ServerEventReceiverType::RACE_RESULTS);

    auto& rr = ev.race_result;

    EXPECT_FALSE(rr.is_last_race);
    ASSERT_EQ(rr.players.size(), 2);

    EXPECT_EQ(rr.players[0].player_name, "Juan");
    EXPECT_EQ(rr.players[0].last_race_time, 1000);
    EXPECT_EQ(rr.players[0].total_race_time, 5000);
    EXPECT_TRUE(rr.players[0].finish_last_race);

    EXPECT_EQ(rr.players[1].player_name, "Ana");
    EXPECT_EQ(rr.players[1].last_race_time, 777);
    EXPECT_EQ(rr.players[1].total_race_time, 8888);
    EXPECT_FALSE(rr.players[1].finish_last_race);
}

TEST(ProtocolClientSendTest, SendKeyMapsCorrectly) {
    MockSocket mock;
    ProtocolClient protocol(mock);

    EXPECT_CALL(mock, is_stream_send_closed()).WillOnce(Return(false));
    EXPECT_CALL(mock, sendall(_, 2)).WillOnce([](const void* buf, unsigned) {
        const uint8_t* b = (uint8_t*)buf;
        EXPECT_EQ(b[0], INPUT_KEY);
        EXPECT_EQ(b[1], UP_PRESSED);
        return 2;
    });

    ServerEventSender ev;
    ev.type = ServerEventSenderType::SEND_KEY;
    ev.send_key.key = DirectionKey::UP_PRESSED;

    protocol.send_event(ev);
}

TEST(ProtocolClientSendTest, SendLeaveLobbySendsOpcode) {
    MockSocket mock;
    ProtocolClient protocol(mock);

    EXPECT_CALL(mock, is_stream_send_closed()).WillOnce(Return(false));
    EXPECT_CALL(mock, sendall(_, 1)).WillOnce([](const void* buf, unsigned) {
        EXPECT_EQ(((uint8_t*)buf)[0], 0x34);
        return 1;
    });

    ServerEventSender ev;
    ev.type = ServerEventSenderType::LEAVE_LOBBY;

    protocol.send_event(ev);
}

TEST(ProtocolClientSendTest, DoesNotSendIfSocketClosed) {
    MockSocket mock;
    ProtocolClient protocol(mock);

    EXPECT_CALL(mock, is_stream_send_closed()).WillOnce(Return(true));
    EXPECT_CALL(mock, sendall(_, _)).Times(0);

    ServerEventSender ev;
    ev.type = ServerEventSenderType::SEND_KEY;
    ev.send_key.key = DirectionKey::UP_PRESSED;

    protocol.send_event(ev);
}

TEST(ProtocolClientTest, ReceiveSnapshotLobbyEmpty) {
    MockSocket mock;
    ProtocolClient protocol(mock);
    bool closed = false;
    InSequence seq;

    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        ((uint8_t*)b)[0] = RECEIVE_SNAPSHOT_LOBBY;
        return 1;
    });

    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned) {
        uint32_t v = htonl(8888);
        memcpy(b, &v, 4);
        return 4;
    });

    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned) {
        uint16_t v = htons(0);
        memcpy(b, &v, 2);
        return 2;
    });

    auto ev = protocol.receive_event(closed);

    ASSERT_EQ(ev.type, ServerEventReceiverType::SNAPSHOT_LOBBY);
    EXPECT_EQ(ev.snapshot_lobby.players_id.size(), 0);
}

TEST(ProtocolClientTest, ReceiveSnapshotMultiplePlayers) {
    MockSocket mock;
    ProtocolClient protocol(mock);
    bool closed = false;
    InSequence seq;

    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        ((uint8_t*)b)[0] = RECEIVE_SNAPSHOT;
        return 1;
    });

    EXPECT_CALL(mock, recvall(_, 4))  // time
            .WillOnce([](void* b, unsigned) {
                uint32_t v = htonl(999);
                memcpy(b, &v, 4);
                return 4;
            });

    EXPECT_CALL(mock, recvall(_, 2))  // players=2
            .WillOnce([](void* b, unsigned) {
                uint16_t v = htons(2);
                memcpy(b, &v, 2);
                return 2;
            });

    // Player 1
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned) {
        uint32_t v = htonl(1);
        memcpy(b, &v, 4);
        return 4;
    });
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        ((uint8_t*)b)[0] = 1;
        return 1;
    });
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned) {
        uint16_t v = htons(10);
        memcpy(b, &v, 2);
        return 2;
    });
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned) {
        uint16_t v = htons(3);
        memcpy(b, &v, 2);
        return 2;
    });
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        ((uint8_t*)b)[0] = 7;
        return 1;
    });
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        ((uint8_t*)b)[0] = 1;
        return 1;
    });
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned) {
        uint32_t v = htonl(10);
        memcpy(b, &v, 4);
        return 4;
    });
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned) {
        uint32_t v = htonl(20);
        memcpy(b, &v, 4);
        return 4;
    });
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        ((uint8_t*)b)[0] = 1;
        return 1;
    });
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned) {
        uint32_t v = htonl(45);
        memcpy(b, &v, 4);
        return 4;
    });
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned) {
        uint16_t v = htons(0);
        memcpy(b, &v, 2);
        return 2;
    });
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        ((uint8_t*)b)[0] = 1;
        return 1;
    });

    // is_secondary_check = 0 (no tiene segundo checkpoint)
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        ((uint8_t*)b)[0] = 0;
        return 1;
    });

    // Player 2 (idéntico)
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce([](void* b, unsigned) {
        uint32_t v = htonl(2);
        memcpy(b, &v, 4);
        return 4;
    });
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce(Return(1));
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce(Return(2));
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce(Return(2));
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce(Return(1));
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce(Return(1));
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce(Return(4));
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce(Return(4));
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce(Return(1));
    EXPECT_CALL(mock, recvall(_, 4)).WillOnce(Return(4));
    EXPECT_CALL(mock, recvall(_, 2)).WillOnce([](void* b, unsigned) {
        uint16_t v = htons(0);
        memcpy(b, &v, 2);
        return 2;
    });
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce(Return(1));

    // is_secondary_check = 0
    EXPECT_CALL(mock, recvall(_, 1)).WillOnce([](void* b, unsigned) {
        ((uint8_t*)b)[0] = 0;
        return 1;
    });

    EXPECT_CALL(mock, recvall(_, 2))  // npc=0
            .WillOnce([](void* b, unsigned) {
                uint16_t v = htons(0);
                memcpy(b, &v, 2);
                return 2;
            });

    auto ev = protocol.receive_event(closed);
    EXPECT_EQ(ev.snapshot.players.size(), 2);
}

TEST(ProtocolClientTest, ReceiveEventSocketClosed) {
    MockSocket mock;
    ProtocolClient protocol(mock);
    bool closed = false;

    EXPECT_CALL(mock, recvall(_, 1)).WillOnce(Return(0));

    auto ev = protocol.receive_event(closed);

    EXPECT_TRUE(closed);
}
