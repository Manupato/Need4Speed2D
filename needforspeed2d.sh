#!/usr/bin/env bash
set -euo pipefail

NAME="need4speed"
BIN_CLIENT="taller_client"
BIN_SERVER="taller_server"
BIN_EDITOR="taller_editor"

SUPP_DIR="Valgrind_helpers"
SUPP_SDL="$SUPP_DIR/sdl.supp"
SUPP_QT="$SUPP_DIR/qt.supp"
SUPP_EXTERNAL="$SUPP_DIR/external_libs.supp"

build() {
    echo "Compilando proyecto..."
    CLEAN=false

    # Revisar el primer argumento de la función
    if [[ "${1-}" == "--clean" || "${1-}" == "-c" ]]; then
        CLEAN=true
    fi

    if $CLEAN; then
        echo "[Build] Limpiando carpeta build..."
        rm -rf build
    else
        echo "[Build] No se limpia la carpeta build (usar --clean o -c para forzar)."
    fi

    echo "[Build] Generando archivos con CMake..."
    cmake -B build -S .

    echo "[Build] Compilando proyecto..."
    cmake --build build -- -j"$(nproc)"

    echo "Build completado correctamente."
}

download() {
    echo "Descargando dependencias..."
    
    echo "Actualizamos paquetes"
    apt update

    echo "Descargamos esenciales"
    apt install -y \
        build-essential \
        cmake \
        git \
        valgrind


    echo "Descargamos librerias de SDL"
    apt install -y libwavpack-dev libfreetype-dev libxext-dev 

    echo "Descargamos librerias de QT6"
    apt install -y \
        qt6-base-dev \
        qt6-tools-dev \
        qt6-tools-dev-tools \
        qt6-base-dev-tools \
        libqt6widgets6 \
        libqt6gui6 \
        libqt6core6 \
        libqt6network6 \
        libasound2-dev \
        libpulse-dev 
}

run_tests() {
    echo "Ejecutando tests..."
    EXEC="./build/protocol_tests"
    "$EXEC"
}

install_project() {
    # Necesitás sudo para esto
    if [[ "$EUID" -ne 0 ]]; then
        echo "Error: el comando 'install' debe ejecutarse como root (sudo)."
        exit 1
    fi

    # quien es el usuario que llamo a sudo
    if [[ -z "${SUDO_USER-}" ]]; then
        echo "Error: SUDO_USER no está definido. Ejecutá usando: sudo ./needforspeed2d.sh install"
        exit 1
    fi

    # Guardamos ruta absoluta del proyecto
    PROJECT_DIR="$(pwd)"

    echo "[Download] Descargando dependencias"
    download

    echo "[Install] Construyendo proyecto y corriendo tests (${SUDO_USER})..."
    sudo -u "$SUDO_USER" bash -lc "
        set -euo pipefail
        cd '$PROJECT_DIR'
        ./needforspeed2d.sh build --clean
        ./needforspeed2d.sh tests
    "

    echo "[Install] Instalando binarios en /usr/bin..."
    install -m 755 "$PROJECT_DIR/build/${BIN_CLIENT}" /usr/bin/"${BIN_CLIENT}"
    install -m 755 "$PROJECT_DIR/build/${BIN_SERVER}" /usr/bin/"${BIN_SERVER}"
    install -m 755 "$PROJECT_DIR/build/${BIN_EDITOR}" /usr/bin/"${BIN_EDITOR}"

    echo "[Install] Instalando assets en /var/${NAME}..."
    mkdir -p "/var/${NAME}"
    cp -r "$PROJECT_DIR/var/${NAME}/." "/var/${NAME}/"

    echo "[Install] Instalando configuración en /etc/${NAME}..."
    mkdir -p "/etc/${NAME}"
    cp -r "$PROJECT_DIR/etc/${NAME}/." "/etc/${NAME}/"

    echo "[Install] Instalación completada."
}


generate_suppressions() {
    echo "[Valgrind] Generando supresiones…"

    # Limpiar si se pidió --clean
    if [[ "${1-}" == "--clean" ]]; then
        rm -f "$SUPP_SDL" "$SUPP_QT" "$SUPP_EXTERNAL"
    fi

    # Si ya existen y no se pidió clean return
    if [[ -f "$SUPP_EXTERNAL" ]]; then
        echo "[Valgrind] Supresiones ya generadas."
        return
    fi

    mkdir -p "$SUPP_DIR"

    echo "[Valgrind]: Generando SDL..."
    cmake --build build --target MinimalSdl >/dev/null

    valgrind \
        --leak-check=full \
        --show-reachable=yes \
        --show-leak-kinds=all \
        --gen-suppressions=all \
        --log-file="$SUPP_DIR/MinimalSdl.log" \
        ./build/MinimalSdl >/dev/null 2>&1

    python3 "$SUPP_DIR/parser.py" "$SUPP_DIR/MinimalSdl.log" "$SUPP_SDL"

    echo "[Valgrind]: Generando Qt..."
    cmake --build build --target MinimalQt >/dev/null

    valgrind \
        --leak-check=full \
        --show-reachable=yes \
        --show-leak-kinds=all \
        --gen-suppressions=all \
        --log-file="$SUPP_DIR/MinimalQt.log" \
        ./build/MinimalQt >/dev/null 2>&1

    python3 "$SUPP_DIR/parser.py" "$SUPP_DIR/MinimalQt.log" "$SUPP_QT"

    echo "[Valgrind]: Uniendo supresiones..."
    cat "$SUPP_SDL" "$SUPP_QT" > "$SUPP_EXTERNAL"

    echo "[Valgrind] Supresiones generadas."
}



run() {
    local MODE="$1"
    local FLAG="${2-}"

    case "$MODE" in
        client)
            EXEC="./build/${BIN_CLIENT}"
            ARGS="localhost 8080"
            ;;
        server)
            EXEC="./build/${BIN_SERVER}"
            ARGS="8080"
            ;;
        editor)
            EXEC="./build/${BIN_EDITOR}"
            ARGS=""
            ;;
        *)
            echo "Error: modo inválido '$MODE'"
            echo "Usa: server, client, editor"
            exit 1
            ;;
    esac

    if [[ "$FLAG" == "--valgrind" ]]; then

        
        VALGRIND_CMD="valgrind --leak-check=full --show-leak-kinds=all"

        if [[ "$MODE" != "server" ]]; then
            if [[ "${3-}" == "--clean" ]]; then
                generate_suppressions --clean
            else
                generate_suppressions
            fi
            VALGRIND_CMD+=" --suppressions=$SUPP_EXTERNAL"
        fi

        $VALGRIND_CMD "$EXEC" $ARGS
    else
        "$EXEC" $ARGS
    fi
}

if [[ $# -lt 1 ]]; then
    echo "Uso: $0 {build|download|tests|run|install}"
    exit 1
fi

COMMAND="$1"
shift || true #Saca el primer flag

case "$COMMAND" in
    build)
        build "$@"
        ;;
    download)
        download
        ;;
    tests)
        build "$@"
        run_tests
        ;;
    install)
        install_project
        ;;
    run)
        if [[ $# -lt 1 ]]; then
            echo "Error: run requiere flag: server, client, editor"
            exit 1
        fi
        run "$@"
        ;;
    *)
        echo "Error: comando inválido '$COMMAND'"
        echo "Usa: build, download, tests, run"
        exit 1
        ;;
esac
