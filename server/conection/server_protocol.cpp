#include "server_protocol.h"

CommandReceiverType ServerProtocol::get_type_of_command(ISocket& skt) {
    try {
        switch (op_bytes.receive_one_byte(skt)) {
            case INPUT_KEY:
                return CommandReceiverType::Move;
                break;
            case CREATE_LOBBY:
                return CommandReceiverType::CreateLobby;
                break;
            case JOIN_LOBBY:
                return CommandReceiverType::JoinLobby;
                break;
            case START_LOBBY:
                return CommandReceiverType::StartLobby;
                break;
            case CMD_UPGRADE:
                return CommandReceiverType::Upgrade;
                break;
            case CMD_DISCONNECT:
                return CommandReceiverType::Disconect;
            default:
                break;
        }
    } catch (const PeerCloseError& e) {
        // El cliente cerro la conexion
        return CommandReceiverType::DefiniteDisconect;
    }
    std::cerr << "ServerProtocol: Comando recibido desconocido, desconectar" << std::endl;
    return CommandReceiverType::DefiniteDisconect;
}

CommandReceiver ServerProtocol::get_command_move(ISocket& skt, int id) {
    uint8_t direccion = op_bytes.receive_one_byte(skt);
    return (CommandReceiver{id, CommandReceiverType::Move, direccion});
}

CommandReceiverJoinLobby ServerProtocol::get_command_join_lobby(ISocket& skt, int id) {
    uint32_t id_lobby = (op_bytes.receive_four_bytes(skt));
    uint8_t model_car = op_bytes.receive_one_byte(skt);
    uint16_t size_string = (op_bytes.receive_two_bytes(skt));
    std::string name = op_bytes.receive_string(size_string, skt);
    return (CommandReceiverJoinLobby{id, CommandReceiverType::JoinLobby, id_lobby, model_car,
                                     name});
}

CommandReceiver ServerProtocol::get_command_upgrade(ISocket& skt, int id) {
    uint8_t upgrade = op_bytes.receive_one_byte(skt);
    return (CommandReceiver{id, CommandReceiverType::Upgrade, upgrade});
}

CommandReceiverCreateLobby ServerProtocol::get_command_create_lobby(ISocket& skt, int id) {
    uint8_t model_car = op_bytes.receive_one_byte(skt);

    uint16_t size_string = (op_bytes.receive_two_bytes(skt));
    std::string name = op_bytes.receive_string(size_string, skt);

    uint16_t maps_size = (op_bytes.receive_two_bytes(skt));

    std::vector<std::string> maps;

    for (int i = 0; i < static_cast<int>(maps_size); i++) {
        uint16_t map_size_string = (op_bytes.receive_two_bytes(skt));
        std::string map = op_bytes.receive_string(map_size_string, skt);
        maps.emplace_back(map);
    }

    return (CommandReceiverCreateLobby{id, CommandReceiverType::CreateLobby, model_car, maps,
                                       name});
}

CommandReceiverStartLobby ServerProtocol::get_command_start_lobby(ISocket& skt, int id) {
    uint32_t id_lobby = (op_bytes.receive_four_bytes(skt));
    return (CommandReceiverStartLobby{id, CommandReceiverType::StartLobby, id_lobby});
}

void ServerProtocol::send_id_to_client(ISocket& skt, const int id) {
    std::vector<uint8_t> buff;
    buff.reserve(5);

    op_bytes.add_one_byte(EVENT_SEND_ID, buff);
    op_bytes.add_four_bytes((static_cast<uint32_t>(id)), buff);

    if (skt.sendall(buff.data(), buff.size()) == 0) {
        throw std::runtime_error("Error sending ID to client (peer closed): id=" +
                                 std::to_string(id));
    }
}

bool ServerProtocol::send_event_to_client(ISocket& skt, Queue<std::shared_ptr<IEvent>>& queue_out) {
    auto ev = queue_out.pop();
    return ev->send(skt, *this);
}

bool ServerProtocol::send_phase_change_to_client(ISocket& skt) {
    std::vector<uint8_t> buff;
    op_bytes.add_one_byte(EVENT_PHASE_CHANGE, buff);
    return (skt.sendall(buff.data(), buff.size()) != 0);
}

bool ServerProtocol::send_race_results_to_client(ISocket& skt,
                                                 const RaceResultsData& race_results) {
    uint16_t count = static_cast<uint16_t>(race_results.race_results.size());

    std::vector<uint8_t> buff;
    buff.reserve(1 + 1 + 2 + count * (2 + 4 + 4 + 4 + 1));

    op_bytes.add_one_byte(EVENT_RACE_RESULTS, buff);
    op_bytes.add_one_byte(race_results.last_race, buff);
    op_bytes.add_two_bytes((count), buff);

    for (const auto& r: race_results.race_results) {
        op_bytes.add_two_bytes((static_cast<uint16_t>(r.name.size())), buff);
        op_bytes.add_string(r.name, buff);
        op_bytes.add_four_bytes((r.race_time_seconds), buff);
        op_bytes.add_four_bytes((r.total_time_seconds), buff);
        op_bytes.add_one_byte(r.status, buff);
    }

    return (skt.sendall(buff.data(), buff.size()) != 0);
}

bool ServerProtocol::send_pre_game_snapshot_to_client(ISocket& skt,
                                                      const PreGameSnapshotData& pre_game) {
    std::vector<uint8_t> buff;
    buff.reserve(1 + 4 + 4 + 4 + 4 + 1 + 2 + 1 + 4 + 4);

    op_bytes.add_one_byte(EVENT_PRE_GAME_SNAPSHOT, buff);

    op_bytes.add_four_bytes((pre_game.pole.coord_up_left.x_px), buff);
    op_bytes.add_four_bytes((pre_game.pole.coord_up_left.y_px), buff);
    op_bytes.add_four_bytes((pre_game.pole.coord_down_right.x_px), buff);
    op_bytes.add_four_bytes((pre_game.pole.coord_down_right.y_px), buff);
    op_bytes.add_one_byte(pre_game.pole.direc, buff);

    op_bytes.add_two_bytes((pre_game.remaining_races), buff);
    op_bytes.add_one_byte(static_cast<uint8_t>(pre_game.map_id), buff);


    op_bytes.add_four_bytes((pre_game.race_total_time_seconds), buff);
    op_bytes.add_four_bytes((pre_game.race_move_enabled_time_seconds), buff);

    return (skt.sendall(buff.data(), buff.size()) != 0);
}

bool ServerProtocol::send_exit_join(ISocket& skt) {
    std::vector<uint8_t> buff;
    op_bytes.add_one_byte(0x30, buff);
    return (skt.sendall(buff.data(), buff.size()) != 0);
}

bool ServerProtocol::send_start_lobby_to_client(ISocket& skt) {
    std::vector<uint8_t> buff;
    op_bytes.add_one_byte(EVENT_START_LOBBY, buff);
    return (skt.sendall(buff.data(), buff.size()) != 0);
}

bool ServerProtocol::send_join_error_to_client(ISocket& skt) {
    std::vector<uint8_t> buff;
    op_bytes.add_one_byte(EVENT_LOBBY_JOIN_ERROR, buff);
    return (skt.sendall(buff.data(), buff.size()) != 0);
}

bool ServerProtocol::send_snapshot_lobby_to_client(ISocket& skt, const LobbySnapshotData& lobby) {

    const uint16_t count = static_cast<uint16_t>(lobby.lobby_players.size());

    std::vector<uint8_t> buff;
    buff.reserve(7 + count * (2 + 4 + 1));

    op_bytes.add_one_byte(EVENT_LOBBY_SNAPSHOT, buff);
    op_bytes.add_four_bytes((lobby.lobby_id), buff);
    op_bytes.add_two_bytes((count), buff);

    // Para cada jugador
    for (const auto& p: lobby.lobby_players) {
        op_bytes.add_two_bytes((static_cast<uint16_t>(p.name.size())), buff);
        op_bytes.add_string(p.name, buff);
        op_bytes.add_one_byte(p.model, buff);
    }

    return (skt.sendall(buff.data(), buff.size()) != 0);
}

bool ServerProtocol::send_snapshot_game_to_client(ISocket& skt, const GameSnapshotData& game) {

    const uint16_t count = static_cast<uint16_t>(game.players.size());

    std::vector<uint8_t> buff;
    buff.reserve(7 + count * (22 + 5 * 8 + 1) + 2 + game.npcs.size() * (13));

    op_bytes.add_one_byte(EVENT_SEND_SNAPSHOT, buff);
    op_bytes.add_four_bytes((game.time_seconds_remained), buff);
    op_bytes.add_two_bytes((count), buff);

    // Para cada jugador
    for (const auto& p: game.players) {
        op_bytes.add_four_bytes((p.id), buff);
        op_bytes.add_one_byte(p.ghost, buff);
        op_bytes.add_two_bytes((p.car_life), buff);
        op_bytes.add_two_bytes((p.model), buff);
        op_bytes.add_one_byte(p.animation, buff);
        op_bytes.add_one_byte(p.sound_code, buff);
        op_bytes.add_four_bytes((p.x_px), buff);
        op_bytes.add_four_bytes((p.y_px), buff);
        op_bytes.add_one_byte(p.z, buff);
        op_bytes.add_four_bytes((p.angle), buff);
        op_bytes.add_two_bytes((static_cast<uint16_t>(p.next_checkpoint.size())), buff);

        // Por cada coord del checkpoint:
        // x_px, y_px (cada uno 4 bytes)
        for (const auto& coord: p.next_checkpoint) {
            op_bytes.add_four_bytes((coord.x_px), buff);
            op_bytes.add_four_bytes((coord.y_px), buff);
        }
        op_bytes.add_one_byte(p.goal, buff);

        // Enviamos el segundo checkpoint

        op_bytes.add_one_byte(p.there_is_second_checkpoint, buff);
        // Si HAY second checkpoint (Sino, ya con el flag avisamos que no hay nada mas por parte de
        // los checkpoints)
        if (p.there_is_second_checkpoint == 1) {
            op_bytes.add_two_bytes((static_cast<uint16_t>(p.next_next_checkpoint.size())), buff);
            for (const auto& coord: p.next_next_checkpoint) {
                op_bytes.add_four_bytes((coord.x_px), buff);
                op_bytes.add_four_bytes((coord.y_px), buff);
            }
            op_bytes.add_one_byte(p.next_next_goal, buff);
        }
    }

    const uint16_t cant_npcs = static_cast<uint16_t>(game.npcs.size());
    op_bytes.add_two_bytes((cant_npcs), buff);

    for (const auto& p: game.npcs) {
        op_bytes.add_two_bytes((p.model), buff);
        op_bytes.add_one_byte(p.animation, buff);
        op_bytes.add_four_bytes((p.x_px), buff);
        op_bytes.add_four_bytes((p.y_px), buff);
        op_bytes.add_one_byte(p.z, buff);
        op_bytes.add_four_bytes((p.angle), buff);
    }

    return (skt.sendall(buff.data(), buff.size()) != 0);
}

bool ServerProtocol::send_race_results_last_to_client(ISocket& skt,
                                                      const RaceResultsData& race_results) {

    const auto& results = race_results.race_results;
    const int n = static_cast<int>(results.size());

    if (n == 0) {
        return true;
    }

    int non_podium_count = 0;
    if (n > 3) {
        non_podium_count = n - 3;
    }

    uint8_t podium_flag = race_results.podium_count;
    if (podium_flag > 3) {
        podium_flag = 3;
    }

    std::vector<uint8_t> buff;
    buff.reserve(1 + 1 + 2 + non_podium_count * (2 + 32 + 4 + 4 + 1) + 1 +
                 podium_flag * (1 + 1 + 32 + 4 + 4 + 1));

    op_bytes.add_one_byte(EVENT_RACE_RESULTS, buff);
    op_bytes.add_one_byte(0x01, buff);

    // Los no podio
    op_bytes.add_two_bytes(static_cast<uint16_t>(non_podium_count), buff);
    for (int i = 3; i < n; ++i) {
        const auto& r = results[i];
        op_bytes.add_two_bytes((static_cast<uint16_t>(r.name.size())), buff);
        op_bytes.add_string(r.name, buff);
        op_bytes.add_four_bytes(r.race_time_seconds, buff);
        op_bytes.add_four_bytes(r.total_time_seconds, buff);
        op_bytes.add_one_byte(r.status, buff);
    }

    // Los no podio los metemos en orden (3 || 2-3 || 1-2-3)
    // 0 -> no mostrar podio
    // 1 -> solo 3
    // 2 -> 2 y 3
    // 3 -> 1, 2 y 3

    op_bytes.add_one_byte(podium_flag, buff);

    for (int place = 1; place <= 3; ++place) {
        int idx = place - 1;

        if (idx >= n) {
            // No existe ese lugar de podio (por ejemplo partida de 2 personas, el 3er puesto no
            // existe)
            op_bytes.add_one_byte(0x00, buff);  // EXISTE_JUGADOR = 0
            continue;
        }

        const auto& r = results[idx];

        op_bytes.add_one_byte(0x01, buff);  // EXISTE_JUGADOR = 1
        op_bytes.add_two_bytes(static_cast<uint16_t>(r.name.size()), buff);
        op_bytes.add_string(r.name, buff);
        op_bytes.add_four_bytes(r.race_time_seconds, buff);
        op_bytes.add_four_bytes(r.total_time_seconds, buff);
        op_bytes.add_one_byte(r.status, buff);
    }

    return (skt.sendall(buff.data(), buff.size()) != 0);
}
