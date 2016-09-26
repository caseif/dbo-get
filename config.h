#pragma once

#ifdef _WIN32
#define OS "win"
#elif __APPLE__
#define OS "osx"
#elif __linux__ || __unix__
#define OS "nix"
#else
#error "Unknown compiler platform"
#endif

#include <map>
#include <string>

class Config {
    private:
        static std::map<std::string, std::string> configMap;
        static std::string getAppdataDir();
        static std::string getConfigDir();
        static std::string getConfigFile();
        static void save();
    public:
        const static std::string KEY_STORE;

        static void load();
        static std::string* get(std::string key);
        static void set(std::string key, std::string value);
};
