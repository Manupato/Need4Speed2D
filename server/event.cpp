#include "event.h"

#include "../common/ISocket.h"
#include "conection/server_protocol.h"

bool LobbySnapshotEvent::send(ISocket& skt, ServerProtocol& proto) const {
    return proto.send_snapshot_lobby_to_client(skt, data);
}

bool GameSnapshotEvent::send(ISocket& skt, ServerProtocol& proto) const {
    return proto.send_snapshot_game_to_client(skt, data);
}

bool JoinErrorEvent::send(ISocket& skt, ServerProtocol& proto) const {
    return proto.send_join_error_to_client(skt);
}

bool StartLobbyEvent::send(ISocket& skt, ServerProtocol& proto) const {
    return proto.send_start_lobby_to_client(skt);
}

bool ExitJoinEvent::send(ISocket& skt, ServerProtocol& proto) const {
    return proto.send_exit_join(skt);
}

bool PreGameSnapshotEvent::send(ISocket& skt, ServerProtocol& proto) const {
    return proto.send_pre_game_snapshot_to_client(skt, data);
}

bool RaceResultsEvent::send(ISocket& skt, ServerProtocol& proto) const {
    return proto.send_race_results_to_client(skt, data);
}

bool RaceResultsLastEvent::send(ISocket& skt, ServerProtocol& proto) const {
    return proto.send_race_results_last_to_client(skt, data);
}

bool PhaseChangeEvent::send(ISocket& skt, ServerProtocol& proto) const {
    return proto.send_phase_change_to_client(skt);
}
