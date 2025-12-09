set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

target_sources(taller_server
    PRIVATE
    # .cpp files
    conection/acceptor.cpp
    game/car.cpp
    game/checkpoint_sensor.cpp
    conection/client_handler.cpp
    conection/client_registry.cpp
    conection/game_manager.cpp
    conection/game.cpp
    event.cpp
    game/gameloop.cpp
    main.cpp
    game/map_loader.cpp
    game/physic_world.cpp
    game/pole.cpp
    game/race_context.cpp
    game/race_system.cpp
    conection/receiver.cpp
    conection/sender.cpp
    conection/server_logic.cpp
    conection/server_protocol.cpp
    game/snapshot_builder.cpp
    game/world_state.cpp
    config.cpp
    PUBLIC
    # .h files
    conection/acceptor.h
    game/car_design.h
    game/car.h
    game/categories.h
    game/checkpoint_sensor.h
    conection/client_handler.h
    conection/client_registry.h
    conection/game_manager.h
    conection/game.h
    conection/op_codes.h
    command.h
    event.h
    game/gameloop.h
    game/map_loader.h
    game/physic_world.h
    game/pole.h
    game/race_context.h
    game/race_progress.h
    game/race_system.h
    conection/receiver.h
    conection/sender.h
    server_error.h
    conection/server_logic.h
    conection/server_protocol.h
    game/snapshot_builder.h
    game/world_state.h
    config.h
    )
