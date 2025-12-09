# TP Grupal: *Need for Speed 2D*

## Grupo 2

Este juego fue desarrollado por los alumnos:
- **Lucas Nicolás Pagani**
- **Federico Zanor**
- **Manuel Pato**
- **Joaquín Schapira**

como trabajo práctico final de la materia **Taller de Programación** en la Facultad de Ingeniería de la UBA (FIUBA).  
El objetivo fue recrear *Need for Speed 2D* con soporte multijugador para aplicar e integrar los conocimientos adquiridos durante el curso.

## Página del juego
El proyecto cuenta con su [página web oficial](https://joaquinschapira.github.io/taller_TP_pagina_web/) donde se podrá observar un gameplay y tutoriales tanto de instalación como de jugabilidad. 

---

## Instalación y uso básico

### 1. Clonar el repositorio
```bash
git clone https://github.com/lucasPagani2003/taller-de-programacion-tp-grupal-2025c2-grupo-2.git
```

### 2. Otorgar permisos de ejecución al instalador
```bash
chmod +x needforspeed2d.sh
```

### 3. Instalar el proyecto *(una única vez, requiere sudo)*
Este paso debe ejecutarse como root porque instala los binarios y copia assets/config:
```bash
sudo ./needforspeed2d.sh install
```
- ¿Que hace internamente?
    - Instala dependencias esenciales
    - Compila el proyecto
    - Ejecuta los tests del protocolo
    - Instala los ejecutables en el sistema:
        - /usr/bin/taller_client
        - /usr/bin/taller_server
        - /usr/bin/taller_editor
    - Copia recursos del juego a /var/need4speed
    - Copia la configuración a /etc/need4speed
    - En cambio, los mapas jugables se guardan en ~/.local/share/need4speed/mapas_jugables

No es necesario modificar nada para jugar, pero es posible personalizar desde config

### 4. Ejecutar los programas
Editor de mapas:
```bash
taller_editor
```

Servidor:
```bash
taller_server 8080
```

Cliente:
```bash
taller_client localhost 8080
```

---

### 5. Video mostrando features

[🎥 Ver video de demostración](https://www.youtube.com/watch?v=uabR9KEANGU)

## Más información
Para ver todos los comandos disponibles y detalles completos de compilación/uso, consultá el [manual de usuario](./Manual_de_Usuario.pdf).

## Licencia

Este proyecto incorpora código proveniente de repositorios de **@eldipa**, todos bajo licencia **GPL v2**:

### Código de sockets (GPL v2)
Del repositorio original: `hands-on-sockets-in-cpp`  
Archivos utilizados en este proyecto:
- `liberror.cpp`, `liberror.h`
- `resolver.cpp`, `resolver.h`
- `resolvererror.cpp`, `resolvererror.h`
- `socket.cpp`, `socket.h`

### Código de threading (GPL v2)
Del repositorio original: `hands-on-threads`  
Archivos utilizados:
- `queue.h`
- `thread.h`

