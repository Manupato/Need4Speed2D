#include "ProtocolClient.h"

#include <cstdint>
#include <iostream>
#include <unordered_map>

#include <netinet/in.h>

#include "../common/operations_bytes.h"

ProtocolClient::ProtocolClient(ISocket& skt): skt(skt) {}

std::vector<uint8_t> ProtocolClient::send_key(SendKey send_key) {

    std::unordered_map<DirectionKey, uint8_t> key_map = {
            {DirectionKey::UP_PRESSED, UP_PRESSED},
            {DirectionKey::LEFT_PRESSED, LEFT_PRESSED},
            {DirectionKey::DOWN_PRESSED, DOWN_PRESSED},
            {DirectionKey::RIGHT_PRESSED, RIGHT_PRESSED},
            {DirectionKey::UP_UNPRESSED, UP_UNPRESSED},
            {DirectionKey::LEFT_UNPRESSED, LEFT_UNPRESSED},
            {DirectionKey::DOWN_UNPRESSED, DOWN_UNPRESSED},
            {DirectionKey::RIGHT_UNPRESSED, RIGHT_UNPRESSED},
            {DirectionKey::WIN, COMMAND_WIN},
            {DirectionKey::LOSE, COMMAND_LOSE},
            {DirectionKey::INFINITE_LIFE, COMMAND_INFINITE_LIFE},
            {DirectionKey::GHOST, COMMAND_GHOST},
    };

    auto it = key_map.find(send_key.key);
    if (it == key_map.end())
        return {};

    return {INPUT_KEY, it->second};
}


void ProtocolClient::send_event(const ServerEventSender& event) {
    std::vector<uint8_t> message;

    if (event.type == ServerEventSenderType::SEND_KEY) {
        message = send_key(event.send_key);

    } else if (event.type == ServerEventSenderType::CREATE_LOBBY) {
        message = send_create_lobby(event.create_lobby);

    } else if (event.type == ServerEventSenderType::JOIN_LOBBY) {
        message = send_join_lobby(event.join_lobby);

    } else if (event.type == ServerEventSenderType::START_GAME) {
        message = send_start_game(event.start_game);

    } else if (event.type == ServerEventSenderType::UPGRADES) {
        message = send_upgrade_car(event.car_upgrade);

    } else if (event.type == ServerEventSenderType::LEAVE_LOBBY) {
        message.push_back(SEND_LEAVE);

    } else {
        return;
    }

    if (!skt.is_stream_send_closed()) {
        skt.sendall(message.data(), message.size());
    }
}


std::vector<uint8_t> ProtocolClient::send_upgrade_car(CarUpgrades car_upgrade) {
    std::vector<uint8_t> message;

    message.push_back(SEND_CAR_UPGRADE);
    message.push_back(static_cast<uint8_t>(car_upgrade));

    return message;
}


std::vector<uint8_t> ProtocolClient::send_start_game(StartGame start_game) {
    std::vector<uint8_t> message;

    message.push_back(SEND_START_GAME);
    operation.add_four_bytes(start_game.lobby_code, message);

    return message;
}


std::vector<uint8_t> ProtocolClient::send_create_lobby(CreateToLobby snapshot) {
    std::vector<uint8_t> message;

    message.push_back(SEND_CREATE_LOBBY);
    message.push_back(snapshot.modeloAuto);

    uint16_t tamanio = static_cast<uint16_t>(snapshot.player_name.size());
    operation.add_two_bytes(tamanio, message);
    operation.add_string(snapshot.player_name, message);

    operation.add_two_bytes(snapshot.nombres_mapa.size(), message);

    for (const auto& nombre_mapa: snapshot.nombres_mapa) {
        tamanio = static_cast<uint16_t>(nombre_mapa.size());
        operation.add_two_bytes(tamanio, message);
        operation.add_string(nombre_mapa, message);
    }

    return message;
}


std::vector<uint8_t> ProtocolClient::send_join_lobby(JoinToLobby join_lobby) {
    std::vector<uint8_t> message;

    message.push_back(SEND_JOIN_LOBBY);
    operation.add_four_bytes(join_lobby.lobby_code, message);

    message.push_back(join_lobby.modeloAuto);

    uint16_t tamanio = static_cast<uint16_t>(join_lobby.player_name.size());
    operation.add_two_bytes(tamanio, message);
    operation.add_string(join_lobby.player_name, message);

    return message;
}


// ------------------ Receive events


ServerEventReceiver ProtocolClient::receive_event(bool& is_socket_closed) {
    ServerEventReceiver return_event;

    uint8_t protocol = 0x00;
    is_socket_closed = false;

    int leidos = skt.recvall(reinterpret_cast<char*>(&protocol), 1);

    if (leidos <= 0) {
        is_socket_closed = true;
        return return_event;
    }
    switch (protocol) {
        case RECEIVE_SNAPSHOT:
            return receive_snapshot();

        case RECEIVE_ID:
            return receive_id();

        case RECEIVE_JOIN_ERROR:
            return_event.type = ServerEventReceiverType::ERROR;
            break;

        case RECEIVE_SNAPSHOT_LOBBY:
            return receive_snapshot_lobby();

        case RECEIVE_START_GAME:
            return_event.type = ServerEventReceiverType::START_GAME;
            break;

        case RECEIVE_PREGAME_SNAPSHOT:
            return receive_pre_game_snapshot();

        case RECEIVE_RACE_RESULTS:
            return receive_race_results();

        case RECEIVE_SUCESS:
            return_event.type = ServerEventReceiverType::SUCESS;
            break;

        case RECEIVE_CHANGE_FASE:
            return_event.type = ServerEventReceiverType::CHANGE_FASE;
    }
    return return_event;
}


ServerEventReceiver ProtocolClient::receive_race_results() {
    ServerEventReceiver event;
    event.type = ServerEventReceiverType::RACE_RESULTS;

    RaceResults race_results;
    race_results.actual_phase = ShowPlayerPhase::NONE;

    uint8_t is_last_race = operation.receive_one_byte(skt);

    race_results.is_last_race = (is_last_race == 0x01);

    uint16_t amount_players = operation.receive_two_bytes(skt);

    for (int i = 0; i < amount_players; i++) {
        PlayerResults player;

        uint16_t lenght_name = operation.receive_two_bytes(skt);
        player.player_name = operation.receive_string(lenght_name, skt);

        player.last_race_time = operation.receive_four_bytes(skt);
        player.total_race_time = operation.receive_four_bytes(skt);

        uint8_t finish_race = operation.receive_one_byte(skt);
        player.finish_last_race = (finish_race == 0x01);

        race_results.players.push_back(player);
    }

    if (race_results.is_last_race) {  // jugadores del podio

        uint8_t race_phase = operation.receive_one_byte(skt);
        if (race_phase > static_cast<uint8_t>(ShowPlayerPhase::FIRST_PLACE)) {
            race_results.actual_phase = ShowPlayerPhase::NONE;
        } else {
            race_results.actual_phase = static_cast<ShowPlayerPhase>(race_phase);
        }


        for (int i = 0; i < 3; i++) {
            PlayerResults player;

            uint8_t player_exist = operation.receive_one_byte(skt);
            if (player_exist == 0x00) {
                player.exist = false;
                race_results.podium_players.push_back(player);
                continue;
            }

            uint16_t lenght_name = operation.receive_two_bytes(skt);
            player.player_name = operation.receive_string(lenght_name, skt);

            player.last_race_time = operation.receive_four_bytes(skt);
            player.total_race_time = operation.receive_four_bytes(skt);

            uint8_t finish_race = operation.receive_one_byte(skt);
            player.finish_last_race = (finish_race == 0x01);

            race_results.podium_players.push_back(player);
        }
    }

    event.race_result = race_results;
    return event;
}


ServerEventReceiver ProtocolClient::receive_pre_game_snapshot() {
    ServerEventReceiver event;
    event.type = ServerEventReceiverType::PREGAME;

    PreGame pre_game;
    Coords coords_received;

    coords_received.coord_x = operation.receive_four_bytes(skt);
    coords_received.coord_y = operation.receive_four_bytes(skt);

    pre_game.start_line.top_left = coords_received;

    coords_received.coord_x = operation.receive_four_bytes(skt);
    coords_received.coord_y = operation.receive_four_bytes(skt);

    pre_game.start_line.bottom_right = coords_received;

    uint8_t direction = operation.receive_one_byte(skt);
    switch (direction) {
        case (RIGHT):
            pre_game.start_line.direction = Direction::RIGHT;
            break;
        case (LEFT):
            pre_game.start_line.direction = Direction::LEFT;
            break;
        case (UP):
            pre_game.start_line.direction = Direction::UP;
            break;
        case (DOWN):
            pre_game.start_line.direction = Direction::DOWN;
            break;
        default:
            pre_game.start_line.direction = Direction::None;
    }

    uint16_t pending_matches = operation.receive_two_bytes(skt);
    pre_game.is_last_map = (pending_matches == 0);

    uint8_t match_selected = operation.receive_one_byte(skt);
    if (match_selected == 0) {
        pre_game.map_selected = Map::LibertyCity;

    } else if (match_selected == 1) {
        pre_game.map_selected = Map::SanAndreas;

    } else if (match_selected == 2) {
        pre_game.map_selected = Map::ViceCity;

    } else {
        event.type = ServerEventReceiverType::ERROR;
        return event;
    }

    pre_game.game_total_time = operation.receive_four_bytes(skt);
    pre_game.game_start_time = operation.receive_four_bytes(skt);

    event.pre_snapshot = pre_game;
    return event;
}


ServerEventReceiver ProtocolClient::receive_snapshot_lobby() {
    ServerEventReceiver event;
    event.type = ServerEventReceiverType::SNAPSHOT_LOBBY;

    Snapshot_lobby snapshot_lobby;
    snapshot_lobby.lobby_code = operation.receive_four_bytes(skt);

    uint16_t amount_players = operation.receive_two_bytes(skt);

    for (int i = 0; i < amount_players; i++) {
        Player_lobby player;

        uint16_t lenght_name = operation.receive_two_bytes(skt);
        std::string nombre = operation.receive_string(lenght_name, skt);
        player.player_name = nombre;

        player.car_model = operation.receive_one_byte(skt);

        snapshot_lobby.players_id.push_back(player);
    }

    event.snapshot_lobby = snapshot_lobby;
    return event;
}


ServerEventReceiver ProtocolClient::receive_snapshot() {
    ServerEventReceiver event;
    event.type = ServerEventReceiverType::SNAPSHOT;

    uint32_t actual_time = operation.receive_four_bytes(skt);
    event.snapshot.actual_time = actual_time;

    uint16_t amount_players = operation.receive_two_bytes(skt);

    for (int i = 0; i < amount_players; i++) {
        Player player;
        player.user_id = operation.receive_four_bytes(skt);

        uint8_t is_ghost = operation.receive_one_byte(skt);
        player.is_car_ghost = (is_ghost == 0x01);

        player.car_life = operation.receive_two_bytes(skt);
        player.car_model = operation.receive_two_bytes(skt);

        player.car_animation = operation.receive_one_byte(skt);

        uint8_t type_sound = operation.receive_one_byte(skt);
        if (type_sound == 0x01) {
            player.type_sound = TypeSound::STOP;

        } else if (type_sound == 0x02) {
            player.type_sound = TypeSound::FINISH;

        } else {
            player.type_sound = TypeSound::NONE;
        }

        player.player_position.coord_x = operation.receive_four_bytes(skt);
        player.player_position.coord_y = operation.receive_four_bytes(skt);
        player.car_coord_z = operation.receive_one_byte(skt);
        player.rotation = operation.receive_four_bytes(skt);

        uint16_t amount_checkpoints = operation.receive_two_bytes(skt);

        for (int j = 0; j < amount_checkpoints; j++) {
            player.next_checkpoint.push_back(
                    {operation.receive_four_bytes(skt), operation.receive_four_bytes(skt)});
        }

        uint8_t is_finishline = operation.receive_one_byte(skt);
        player.is_checkpoint_finishline = (is_finishline != 0x00);


        uint8_t is_secondaty_check = operation.receive_one_byte(skt);
        player.is_secondary_check = (is_secondaty_check == 0x01);

        player.is_secondary_finishline = false;
        if (player.is_secondary_check) {
            amount_checkpoints = operation.receive_two_bytes(skt);

            for (int j = 0; j < amount_checkpoints; j++) {
                player.secondary_checkpoint.push_back(
                        {operation.receive_four_bytes(skt), operation.receive_four_bytes(skt)});
            }

            is_finishline = operation.receive_one_byte(skt);
            player.is_secondary_finishline = (is_finishline != 0x00);
        }


        event.snapshot.players.push_back(player);
    }

    uint16_t amount_npc = operation.receive_two_bytes(skt);
    for (int i = 0; i < amount_npc; i++) {
        NPC npc;
        npc.model = operation.receive_two_bytes(skt);
        npc.car_animation = operation.receive_one_byte(skt);
        npc.pos = {operation.receive_four_bytes(skt), operation.receive_four_bytes(skt)};
        npc.pos_z = operation.receive_one_byte(skt);
        npc.rotation = operation.receive_four_bytes(skt);

        event.snapshot.npcs.push_back(npc);
    }

    return event;
}


ServerEventReceiver ProtocolClient::receive_id() {
    ServerEventReceiver event;
    event.type = ServerEventReceiverType::RECEIVE_ID;

    event.id_jugador = operation.receive_four_bytes(skt);
    return event;
}
