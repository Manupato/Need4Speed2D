find_package(Qt6 QUIET COMPONENTS Widgets)
if (Qt6_FOUND)
    set(QT_WIDGETS_LIB Qt6::Widgets)
else()
    find_package(Qt5 REQUIRED COMPONENTS Widgets)
    set(QT_WIDGETS_LIB Qt5::Widgets)
endif()


target_sources(taller_client
    # .cpp files    
    PRIVATE
    Client.cpp
    ProtocolClient.cpp 
    ThreadSenderClient.cpp
    ThreadReceiver.cpp
    main.cpp

    # Game_GUI
    Game_GUI/GuiSDL.cpp
    Game_GUI/GameloopRace.cpp
    Game_GUI/render/CarRender.cpp
    Game_GUI/render/CheckpointRender.cpp
    Game_GUI/render/BarLifeRender.cpp
    Game_GUI/render/MiniMapRender.cpp
    Game_GUI/render/CarRenderAnimation.cpp
    Game_GUI/render/TimeRender.cpp
    Game_GUI/render/HintArrowRender.cpp
    Game_GUI/render/RenderPositions.cpp
    Game_GUI/render/RenderCarUpgrades.cpp 
    Game_GUI/render/Button.cpp
    Game_GUI/render/MuteSimbolRender.cpp
    Game_GUI/sound/Sound.cpp
    Game_GUI/sound/PlayerSound.cpp
    Game_GUI/InputHandler.cpp
    Game_GUI/RacePhaseManager.cpp
    Game_GUI/GameSoundManager.cpp
    
    # Lobby
    Lobby/prelobby.cpp
    Lobby/CreateLobby.cpp
    Lobby/JoinLobby.cpp
    Lobby/Lobby.cpp
    Lobby/StatBar.cpp

    # .h files
    PUBLIC
    Client.h 
    ProtocolClient.h 
    ExceptionClient.h 
    ThreadSenderClient.h
    ThreadReceiver.h
    ServerEvent.h
    
    #Gamu_GUI 
    Game_GUI/GuiSDL.h
    Game_GUI/GameloopRace.h
    Game_GUI/render/CarRender.h
    Game_GUI/render/CheckpointRender.h
    Game_GUI/render/BarLifeRender.h
    Game_GUI/render/MiniMapRender.h
    Game_GUI/render/CarRenderAnimation.h
    Game_GUI/render/TimeRender.h
    Game_GUI/render/HintArrowRender.h
    Game_GUI/render/RenderPositions.h
    Game_GUI/render/RenderCarUpgrades.h 
    Game_GUI/render/Button.h
    Game_GUI/render/MuteSimbolRender.h
    Game_GUI/sound/Sound.h
    Game_GUI/sound/PlayerSound.h
    Game_GUI/InputHandler.h
    Game_GUI/RacePhaseManager.h
    Game_GUI/GameSoundManager.h

    #Lobby
    Lobby/prelobby.h
    Lobby/CreateLobby.h
    Lobby/JoinLobby.h
    Lobby/Lobby.h
    Lobby/StatBar.h
    )



