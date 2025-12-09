#ifndef SERVER_EVENT_H
#define SERVER_EVENT_H

#include <cstdint>
#include <string>
#include <vector>


enum class Map { LibertyCity, SanAndreas, ViceCity };

enum class ServerEventReceiverType {
    RECEIVE_ID,
    SNAPSHOT,
    SNAPSHOT_LOBBY,
    START_GAME,
    SUCESS,
    PREGAME,
    RACE_RESULTS,
    CHANGE_FASE,
    ERROR
};

struct Coords {
    uint32_t coord_x;
    uint32_t coord_y;
};


//----------------------------------------
// Player Lobby

struct Player_lobby {
    std::string player_name;
    uint8_t car_model;
};


//----------------------------------------
// Snapshot Lobby

struct Snapshot_lobby {
    u_int32_t lobby_code;
    std::vector<Player_lobby> players_id;
};

//----------------------------------------
// Pre Snapshot game

enum class Direction { None, RIGHT, LEFT, UP, DOWN };

struct StartLine {
    Coords top_left;
    Coords bottom_right;
    Direction direction;
};

struct PreGame {
    StartLine start_line;
    bool is_last_map;
    Map map_selected;
    uint32_t game_total_time;
    uint32_t game_start_time;
};


//----------------------------------------
// Snapshot game

enum class TypeSound { NONE, STOP, FINISH };


struct Player {
    uint32_t user_id;
    bool is_car_ghost;
    Coords player_position;
    uint8_t car_coord_z;
    std::vector<Coords> next_checkpoint;
    bool is_secondary_check;
    std::vector<Coords> secondary_checkpoint;
    uint16_t car_life;
    uint16_t car_model;
    uint8_t car_animation;
    TypeSound type_sound;
    uint32_t rotation;
    bool is_checkpoint_finishline;
    bool is_secondary_finishline;
};

struct NPC {
    uint16_t model;
    uint8_t car_animation;
    Coords pos;
    uint8_t pos_z;
    uint32_t rotation;
};

struct Snapshot {
    std::vector<Player> players;
    std::vector<NPC> npcs;
    uint32_t actual_time;
};


//----------------------------------------
// Race Results

struct PlayerResults {
    std::string player_name;
    uint32_t last_race_time = 0;
    uint32_t total_race_time = 0;
    bool finish_last_race = true;
    bool exist = true;
};

enum class ShowPlayerPhase { REGULAR, THIRD_PLACE, SECOND_PLACE, FIRST_PLACE, NONE };


struct RaceResults {
    bool is_last_race;
    std::vector<PlayerResults> players;
    std::vector<PlayerResults> podium_players;
    ShowPlayerPhase actual_phase;
};

//----------------------------------------
// ServerEventReceiver

struct ServerEventReceiver {
    ServerEventReceiverType type = ServerEventReceiverType::ERROR;
    uint32_t id_jugador = 0;
    Snapshot snapshot{};
    Snapshot_lobby snapshot_lobby{};
    PreGame pre_snapshot{};
    RaceResults race_result{};
};


//-----------------------------------------------------------------

enum class ServerEventSenderType {
    SEND_KEY,
    CREATE_LOBBY,
    JOIN_LOBBY,
    START_GAME,
    UPGRADES,
    LEAVE_LOBBY,
    MUSIC_CONFIG,
    ERROR,
    NONE
};


enum class DirectionKey {
    None,
    UP_PRESSED,
    DOWN_PRESSED,
    LEFT_PRESSED,
    RIGHT_PRESSED,
    UP_UNPRESSED,
    DOWN_UNPRESSED,
    LEFT_UNPRESSED,
    RIGHT_UNPRESSED,
    WIN,
    LOSE,
    INFINITE_LIFE,
    GHOST
};


struct StartGame {
    uint32_t lobby_code;
};

struct SendKey {
    DirectionKey key;
};

struct CreateToLobby {
    uint8_t modeloAuto;
    std::string player_name;
    std::vector<std::string> nombres_mapa;
};


struct JoinToLobby {
    uint8_t modeloAuto;
    uint32_t lobby_code;
    std::string player_name;
};


enum class CarUpgrades {
    NOTHING,
    VELOCITY_I,
    VELOCITY_II,
    VELOCITY_III,
    SHIELD_I,
    SHIELD_II,
    SHIELD_III,
    DRIVEABILITY_I,
    DRIVEABILITY_II,
    DRIVEABILITY_III
};


enum class MusicConfigType { None, MUTE, INCREASE, DECREASE };


struct ServerEventSender {
    ServerEventSenderType type;
    SendKey send_key;
    CreateToLobby create_lobby;
    JoinToLobby join_lobby;
    StartGame start_game;
    CarUpgrades car_upgrade;
    MusicConfigType music_config;
};


#endif
