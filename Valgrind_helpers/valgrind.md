# Para poder ejecutar con valgrind desde root

### 1° Limpiar archivos pasados
Si ya hay archivos .supp eliminelos (qt.supp , sdl.supp y external_libs.supp)

```bash 
    rm  Valgrind_helpers/qt.supp  Valgrind_helpers/sdl.supp Valgrind_helpers/external_libs.supp
``` 

### 2° Generar supresiones de SDL
 Compilamos el codigo de SDL y generamos las supresiones en sdl.supp:
```bash 
    cmake --build build --target MinimalSdl

    valgrind \
    --leak-check=full \
    --show-reachable=yes \
    --show-leak-kinds=all \
    --gen-suppressions=all \
    --log-file=Valgrind_helpers/MinimalSdl.log \
    ./build/MinimalSdl

    python3 Valgrind_helpers/parser.py \
    Valgrind_helpers/MinimalSdl.log \
    Valgrind_helpers/sdl.supp
``` 

### 3° Generar supresiones de QT:
 Compilamos el codigo de QT y generamos las supresiones en qt.supp:
```bash 
    cmake --build build --target MinimalQt

    valgrind \
    --leak-check=full \
    --show-reachable=yes \
    --show-leak-kinds=all \
    --gen-suppressions=all \
    --log-file=Valgrind_helpers/MinimalQt.log \
    ./build/MinimalQt

    python3 Valgrind_helpers/parser.py \
    Valgrind_helpers/MinimalQt.log \
    Valgrind_helpers/qt.supp
``` 

### 4° Unir supresiones
Combinar SDL y Qt en un único archivo external_libs.supp
``` bash
    cat Valgrind_helpers/sdl.supp Valgrind_helpers/qt.supp > Valgrind_helpers/external_libs.supp
``` 

### 5° Ejecutar Valgrind con las supresiones
Usar las supresiones al correr los distintos componentes del proyecto (servidor no necesita supresiones)
``` bash
    ./need4speed.sh run server --valgrind
    ./need4speed.sh run client --valgrind
    ./need4speed.sh run editor --valgrind
``` 