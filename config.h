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
        std::map<std::string, std::string> configMap;
        std::string getAppdataDir();
        std::string getConfigDir();
        std::string getConfigFile();
        void save();
    public:
        const static std::string KEY_STORE;

        void load();
        std::string* get(std::string key);
        void set(std::string key, std::string value);
};
