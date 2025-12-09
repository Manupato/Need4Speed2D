#ifndef EVENT_H
#define EVENT_H

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "conection/op_codes.h"

class ISocket;
class ServerProtocol;

enum class MapId : uint8_t { LibertyCity = 0, SanAndreas = 1, ViceCity = 2 };

struct Coord {
    uint32_t x_px;
    uint32_t y_px;
};

struct PlayerSnapshot {
    uint32_t id;
    uint8_t ghost = 0;
    uint16_t car_life;
    uint16_t model;
    uint8_t animation;
    uint8_t sound_code;
    uint32_t x_px;
    uint32_t y_px;
    uint8_t z;
    uint32_t angle;
    std::vector<Coord> next_checkpoint;
    // Por defecto NO hay segundo checkpoint
    uint8_t there_is_second_checkpoint = 0;
    std::vector<Coord> next_next_checkpoint;
    uint8_t goal = 0x00;
    uint8_t next_next_goal = 0x00;
};

struct NpcSnapshot {
    uint16_t model;
    uint8_t animation;
    uint32_t x_px;
    uint32_t y_px;
    uint8_t z;
    uint32_t angle;
};

struct LobbyPlayer {
    std::string name;
    uint8_t model;
};

struct PlayerRaceResult {
    uint32_t id;
    std::string name;
    uint32_t race_time_seconds;
    uint32_t total_time_seconds;
    uint8_t status;
};

struct PoleCoordsAndDirec {
    Coord coord_up_left;
    Coord coord_down_right;
    uint8_t direc;
};

struct GameSnapshotData {
    uint32_t time_seconds_remained = 0;
    std::vector<PlayerSnapshot> players;
    std::vector<NpcSnapshot> npcs;
};

struct LobbySnapshotData {
    uint32_t lobby_id = 0;
    std::vector<LobbyPlayer> lobby_players;
};

struct PreGameSnapshotData {
    uint16_t remaining_races = 0;
    MapId map_id = MapId::LibertyCity;
    PoleCoordsAndDirec pole;
    uint32_t race_total_time_seconds;
    uint32_t race_move_enabled_time_seconds;
};

struct RaceResultsData {
    std::vector<PlayerRaceResult> race_results;
    uint8_t last_race = 0;
    uint8_t podium_count = 0;
};

class IEvent {
public:
    virtual ~IEvent() = default;

    // Cada evento sabe como enviarse usando el protocolo
    virtual bool send(ISocket& skt, ServerProtocol& proto) const = 0;
};


class GameSnapshotEvent: public IEvent {
public:
    GameSnapshotData data;

    explicit GameSnapshotEvent(GameSnapshotData d): data(std::move(d)) {}

    bool send(ISocket& skt, ServerProtocol& proto) const override;
};

class LobbySnapshotEvent: public IEvent {
public:
    LobbySnapshotData data;

    explicit LobbySnapshotEvent(LobbySnapshotData d): data(std::move(d)) {}

    bool send(ISocket& skt, ServerProtocol& proto) const override;
};

class JoinErrorEvent: public IEvent {
public:
    JoinErrorEvent() = default;

    bool send(ISocket& skt, ServerProtocol& proto) const override;
};

class StartLobbyEvent: public IEvent {
public:
    StartLobbyEvent() = default;

    bool send(ISocket& skt, ServerProtocol& proto) const override;
};

class ExitJoinEvent: public IEvent {
public:
    ExitJoinEvent() = default;

    bool send(ISocket& skt, ServerProtocol& proto) const override;
};

class PreGameSnapshotEvent: public IEvent {
public:
    PreGameSnapshotData data;

    explicit PreGameSnapshotEvent(PreGameSnapshotData d): data(std::move(d)) {}

    bool send(ISocket& skt, ServerProtocol& proto) const override;
};

class RaceResultsEvent: public IEvent {
public:
    RaceResultsData data;

    explicit RaceResultsEvent(RaceResultsData d): data(std::move(d)) {}

    bool send(ISocket& skt, ServerProtocol& proto) const override;
};

// Usa la misma RaceResultsData
class RaceResultsLastEvent: public IEvent {
public:
    RaceResultsData data;

    explicit RaceResultsLastEvent(RaceResultsData d): data(std::move(d)) {}

    bool send(ISocket& skt, ServerProtocol& proto) const override;
};

class PhaseChangeEvent: public IEvent {
public:
    PhaseChangeEvent() = default;

    bool send(ISocket& skt, ServerProtocol& proto) const override;
};

#endif  // EVENT_H
