cmake_minimum_required(VERSION 3.24)

if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_EXTENSIONS ON)

project(taller_tp
        VERSION 1.0
        HOMEPAGE_URL "https://github.com/lucasPagani2003/taller-de-programacion-tp-grupal-2025c2-grupo-2.git"
        LANGUAGES CXX
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


option(TALLER_TESTS "Enable / disable tests." ON)
option(TALLER_CLIENT "Enable / disable client program." ON)
option(TALLER_SERVER "Enable / disable server program." ON)
option(TALLER_EDITOR "Enable / disable editor program." ON)
option(TALLER_MAKE_WARNINGS_AS_ERRORS "Enable / disable warnings as errors." ON)

message(CMAKE_CXX_COMPILER_ID="${CMAKE_CXX_COMPILER_ID}")

add_library(protocol_client_lib
    client/ProtocolClient.cpp
)

target_link_libraries(protocol_client_lib
    PRIVATE taller_common
)

add_library(protocol_server_lib
    server/conection/server_protocol.cpp
)

target_link_libraries(protocol_server_lib
    PRIVATE taller_common
)


# -----------------------------------------------
# COMMON LIBRARY
# -----------------------------------------------
add_library(taller_common STATIC)
add_subdirectory(common/)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(taller_common ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)
target_link_libraries(taller_common
    PUBLIC
        box2d
        nlohmann_json::nlohmann_json
        yaml-cpp::yaml-cpp
)


# -----------------------------------------------
# DEPENDENCIAS SDL
# -----------------------------------------------
if(TALLER_CLIENT OR TALLER_EDITOR)
    include(FetchContent)
    FetchContent_Declare(
            SDL2
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-2.30.8
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
            OVERRIDE_FIND_PACKAGE
    )
    FetchContent_Declare(
            SDL2_image
            GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
            GIT_TAG release-2.8.2
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
            OVERRIDE_FIND_PACKAGE
    )
    FetchContent_Declare(
            SDL2_mixer
            GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
            GIT_TAG release-2.8.0
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
            OVERRIDE_FIND_PACKAGE
    )
    FetchContent_Declare(
            SDL2_ttf
            GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
            GIT_TAG release-2.22.0
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
            OVERRIDE_FIND_PACKAGE
    )
    FetchContent_Declare(
            libSDL2pp
            OVERRIDE_FIND_PACKAGE
            URL https://github.com/libSDL2pp/libSDL2pp/archive/cc198c9a5657048bee67ece82de620b2d5661084.zip
    )

    set(SDL2PP_WITH_IMAGE ON)
    set(SDL2PP_WITH_TTF   ON)
    set(SDL2PP_WITH_MIXER ON)
    set(SDL_JACK OFF CACHE BOOL "" FORCE)
    set(SDL2MIXER_VORBIS "STB"   CACHE STRING "" FORCE)
    set(SDL2MIXER_MP3    "DRMP3"  CACHE STRING "" FORCE)
    set(SDL2MIXER_OPUS   OFF      CACHE BOOL   "" FORCE)
    set(SDL2MIXER_MOD    OFF      CACHE BOOL   "" FORCE)  
    set(SDL2MIXER_MIDI           OFF CACHE BOOL "" FORCE)
    set(SDL2MIXER_MIDI_FLUIDSYNTH OFF CACHE BOOL "" FORCE)
    set(SDL2MIXER_MIDI_NATIVE     OFF CACHE BOOL "" FORCE)
    set(SDL2MIXER_MIDI_TIMIDITY   OFF CACHE BOOL "" FORCE)
    set(CMAKE_DISABLE_FIND_PACKAGE_FluidSynth ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SDL2 SDL2_image SDL2_mixer SDL2_ttf libSDL2pp)
endif()

# -----------------------------------------------
# CLIENTE
# -----------------------------------------------
if(TALLER_CLIENT)
    add_executable(taller_client)
    add_dependencies(taller_client taller_common SDL2pp::SDL2pp)
    add_subdirectory(client)
    set_project_warnings(taller_client ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)
    target_include_directories(taller_client PRIVATE ${libSDL2pp_SOURCE_DIR})

    # Qt Widgets para el cliente
    find_package(Qt6 QUIET COMPONENTS Widgets)
    if (Qt6_FOUND)
        set(QT_WIDGETS_LIB Qt6::Widgets)
    else()
        find_package(Qt5 REQUIRED COMPONENTS Widgets)
        set(QT_WIDGETS_LIB Qt5::Widgets)
    endif()

    # Linkeo correcto (firma “plain”, sin PRIVATE/PUBLIC)
    target_link_libraries(taller_client
            taller_common
            SDL2pp::SDL2pp
            SDL2_mixer::SDL2_mixer
            ${QT_WIDGETS_LIB}
    )

    target_include_directories(taller_client PRIVATE
        ${CMAKE_SOURCE_DIR}/client
    )


    add_dependencies(taller_client SDL2 SDL2_image SDL2_mixer SDL2_ttf)

    add_executable(MinimalSdl Valgrind_helpers/MinimalSdl.cpp client/Game_GUI/sound/Sound.cpp)
    target_link_libraries(MinimalSdl
        PRIVATE
        taller_common
        SDL2
        SDL2_mixer
        SDL2_image
        SDL2_ttf
        SDL2pp::SDL2pp
        )

endif()

# -----------------------------------------------
# EDITOR (Qt)
# -----------------------------------------------
if(TALLER_EDITOR)
    message(STATUS "Configuring editor with Qt6...")

    find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core)

    add_executable(taller_editor)
    add_dependencies(taller_editor taller_common)
    add_subdirectory(editor)
    set_project_warnings(taller_editor ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)

    target_link_libraries(taller_editor
            PRIVATE
            taller_common
            Qt6::Core
            Qt6::Gui
            Qt6::Widgets
    )

    set_target_properties(taller_editor PROPERTIES
            AUTOMOC ON
            AUTOUIC ON
            AUTORCC ON
    )

    add_executable(MinimalQt Valgrind_helpers/MinimalQt.cpp)

    target_link_libraries(MinimalQt
            PRIVATE
            Qt6::Core
            Qt6::Gui
            Qt6::Widgets
    )
endif()

# -----------------------------------------------
# SERVER – Box2D + JSON + YAML deps
# -----------------------------------------------
include(FetchContent)

# --- Box2D 3.1.1: forzamos build estático para evitar símbolos faltantes ---
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_DOCS OFF CACHE BOOL "" FORCE)

# *** ESTA ES LA LÍNEA IMPORTANTE ***
set(BOX2D_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_STATIC ON CACHE BOOL "" FORCE)

FetchContent_Declare(
        box2d
        GIT_REPOSITORY https://github.com/erincatto/box2d.git
        GIT_TAG v3.1.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(box2d)

# --- JSON ---
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# --- YAML ---
FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG yaml-cpp-0.7.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(yaml-cpp)

# -----------------------------------------------
# SERVER BINARIO
# -----------------------------------------------
if(TALLER_SERVER)
    add_executable(taller_server)
    add_dependencies(taller_server taller_common)
    add_subdirectory(server)
    set_project_warnings(taller_server ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)

    target_link_libraries(taller_server
        PRIVATE
            taller_common
            box2d               # usa libbox2d.a estático
            nlohmann_json::nlohmann_json
            yaml-cpp::yaml-cpp
    )
endif()


# -----------------------------------------------
# TESTS
# -----------------------------------------------
include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

# Para evitar warnings en tu build
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(protocol_tests
    tests/test_protocol_client.cpp
    tests/test_protocol_server.cpp
    tests/MockSocket.h

    client/ProtocolClient.cpp
    server/conection/server_protocol.cpp
    server/event.cpp
)


target_link_libraries(protocol_tests
    protocol_client_lib
    protocol_server_lib
    taller_common
    gtest
    gmock
    gtest_main
)


target_include_directories(protocol_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/server
    ${CMAKE_SOURCE_DIR}/client
    ${CMAKE_SOURCE_DIR}/common
)

if(TALLER_TESTS)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    include(GoogleTest)

    add_executable(taller_tests)
    add_dependencies(taller_tests taller_common)
    target_include_directories(taller_tests PUBLIC .)
    add_subdirectory(tests/)
    set_project_warnings(taller_tests ${TALLER_MAKE_WARNINGS_AS_ERRORS} TRUE)

    target_include_directories(taller_tests PUBLIC
            ${gtest_SOURCE_DIR}/include
            ${gmock_SOURCE_DIR}/include
    )
    target_link_libraries(taller_tests
            taller_common
            GTest::gtest_main
    )
endif()

if(TALLER_TESTS)
    add_custom_target(valgrind_tests
            COMMAND ${CMAKE_COMMAND} -E env
            valgrind --leak-check=full --error-exitcode=1 $<TARGET_FILE:taller_tests>
            DEPENDS taller_tests
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()


