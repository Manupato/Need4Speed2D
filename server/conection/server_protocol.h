#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include <atomic>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../../common/ISocket.h"
#include "../../common/operations_bytes.h"
#include "../../common/queue.h"
#include "../command.h"
#include "../event.h"

#include "op_codes.h"

class ServerProtocol {

private:
    // Usamos esta clase para las operaciones con bytes repetidas
    OperationsBytes op_bytes;

public:
    ServerProtocol() = default;

    CommandReceiverType get_type_of_command(ISocket& skt);

    CommandReceiver get_command_move(ISocket& skt, int id);

    CommandReceiverJoinLobby get_command_join_lobby(ISocket& skt, int id);

    CommandReceiver get_command_upgrade(ISocket& skt, int id);

    CommandReceiverCreateLobby get_command_create_lobby(ISocket& skt, int id);

    CommandReceiverStartLobby get_command_start_lobby(ISocket& skt, int id);

    bool send_event_to_client(ISocket& skt, Queue<std::shared_ptr<IEvent>>& queue_out);

    void send_id_to_client(ISocket& skt, const int id);

    bool send_snapshot_game_to_client(ISocket& skt, const GameSnapshotData& game);

    bool send_snapshot_lobby_to_client(ISocket& skt, const LobbySnapshotData& lobby);

    bool send_pre_game_snapshot_to_client(ISocket& skt, const PreGameSnapshotData& pre_game);

    bool send_join_error_to_client(ISocket& skt);

    bool send_start_lobby_to_client(ISocket& skt);

    bool send_race_results_to_client(ISocket& skt, const RaceResultsData& race_results);

    bool send_race_results_last_to_client(ISocket& skt, const RaceResultsData& race_results);

    bool send_exit_join(ISocket& skt);

    bool send_phase_change_to_client(ISocket& skt);

    ServerProtocol(const ServerProtocol&) = delete;
    ServerProtocol& operator=(const ServerProtocol&) = delete;
    ~ServerProtocol() = default;
};

#endif  // SERVER_PROTOCOL_H
