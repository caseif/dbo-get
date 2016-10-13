#pragma once

#include <map>
#include <string>

class Config {
    private:
        std::map<std::string, std::string> configMap;
		void load();
        void save();
    public:
        Config();
        Config(Config const&) = delete;
        void operator = (Config const&) = delete;
        static Config& getInstance();
        const static std::string KEY_STORE;

        std::string* get(std::string key);
        void set(std::string key, std::string value);
};
