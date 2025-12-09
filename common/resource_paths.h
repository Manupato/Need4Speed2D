#ifndef RESOURCE_PATHS_H
#define RESOURCE_PATHS_H

#include <string>

class ResourcePaths {
private:
    static std::string assets_prefix;  // NOLINT(runtime/string)
    static std::string config_prefix;  // NOLINT(runtime/string)
    static std::string user_data_dir;  // NOLINT(runtime/string)
    static std::string user_maps_dir;  // NOLINT(runtime/string)

    static std::string resolveAssetsPrefix(const std::string& name);
    static std::string resolveConfigPrefix(const std::string& name);

    static std::string resolveUserDataDir(const std::string& name);
    static void ensureUserMapsInitialized();

public:
    // Llamar una vez al inicio de cada binario (client, server, editor)
    static void init(const std::string& name = "need4speed");

    // Solo lectura
    static const std::string& assets();  // var/... o /var/...
    static const std::string& config();  // etc/... o /etc/...

    // Lectura/escritura sin sudo (mapas jugables de usuario)
    static const std::string& userMaps();
};

#endif  // RESOURCE_PATHS_H
