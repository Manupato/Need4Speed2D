#include "resource_paths.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>

std::string ResourcePaths::assets_prefix;  // NOLINT(runtime/string)
std::string ResourcePaths::config_prefix;  // NOLINT(runtime/string)
std::string ResourcePaths::user_data_dir;  // NOLINT(runtime/string)
std::string ResourcePaths::user_maps_dir;  // NOLINT(runtime/string)

using std::filesystem::create_directories;
using std::filesystem::exists;
namespace fs = std::filesystem;

std::string ResourcePaths::resolveAssetsPrefix(const std::string& name) {
    const std::string local = "var/" + name;
    const std::string system = "/var/" + name;

    if (exists(local)) {
        return local;
    }
    if (exists(system)) {
        return system;
    }

    std::cerr << "[ResourcePaths] Error: No se encontró carpeta de assets en '" << local
              << "' ni en '" << system << "'\n";
    return system;
}

std::string ResourcePaths::resolveConfigPrefix(const std::string& name) {
    const std::string local = "etc/" + name;
    const std::string system = "/etc/" + name;

    if (exists(local)) {
        return local;
    }
    if (exists(system)) {
        return system;
    }

    std::cerr << "[ResourcePaths] Error: No se encontró carpeta de config en '" << local
              << "' ni en '" << system << "'\n";
    return system;
}

// ~/.local/share/need4speed
std::string ResourcePaths::resolveUserDataDir(const std::string& name) {
    const char* home = std::getenv("HOME");
    std::string base;

    if (home && *home != '\0') {
        base = std::string(home) + "/.local/share/";
    } else {
        base = "/tmp/";
    }

    return base + name;
}

void ResourcePaths::ensureUserMapsInitialized() {
    if (user_maps_dir.empty()) {
        return;
    }

    create_directories(user_maps_dir);

    // Si ya hay mapas en la carpeta de usuario, no copiamos nada.
    const bool has_any_json =
            std::any_of(std::filesystem::directory_iterator(user_maps_dir),
                        std::filesystem::directory_iterator(), [](const auto& entry) {
                            return entry.is_regular_file() && entry.path().extension() == ".json";
                        });

    if (has_any_json) {
        return;
    }

    // Copiar .json iniciales desde assets/mapas_jugables
    const std::string src_dir = assets_prefix + "/mapas_jugables";
    if (!exists(src_dir)) {
        std::cerr << "[ResourcePaths] Warning: No se encontró " << src_dir
                  << " para inicializar mapas de usuario.\n";
        return;
    }

    try {
        for (const auto& entry: fs::directory_iterator(src_dir)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".json") {
                continue;
            }
            fs::path dst = fs::path(user_maps_dir) / entry.path().filename();
            if (!exists(dst)) {
                fs::copy_file(entry.path(), dst, fs::copy_options::skip_existing);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[ResourcePaths] Error copiando mapas de usuario: " << e.what() << "\n";
    }
}

void ResourcePaths::init(const std::string& name) {
    assets_prefix = resolveAssetsPrefix(name);
    config_prefix = resolveConfigPrefix(name);
    user_data_dir = resolveUserDataDir(name);
    user_maps_dir = user_data_dir + "/mapas_jugables";

    create_directories(user_data_dir);
    create_directories(user_maps_dir);
    ensureUserMapsInitialized();
}

const std::string& ResourcePaths::assets() { return assets_prefix; }

const std::string& ResourcePaths::config() { return config_prefix; }

const std::string& ResourcePaths::userMaps() { return user_maps_dir; }
