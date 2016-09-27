#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <streambuf>

#include "config.h"
#include "util.h"

const std::string Config::KEY_STORE = "store";

std::string Config::getAppdataDir() {
    if (OS == "win") {
        return getenv("APPDATA");
    } else {
        std::string userHome = getenv("HOME");
        if (OS == "osx") {
            return userHome + "/Library/Application Support";
        } else {
            return userHome + "/.config";
        }
    }
}

std::string Config::getConfigDir() {
    return getAppdataDir() + "/dbo-get";
}

std::string Config::getConfigFile() {
    return getConfigDir() + "/global.properties";
}

void Config::load() {
    std::ifstream fileStream(getConfigFile());
    if (!fileStream.is_open()) {
        print("Warning: failed to read global config file");
        return;
    }
    std::string str((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
    std::vector<std::string> arr = explode(str, '\n');
    for (size_t i = 0; i < arr.size(); i++) {
        int eqInd = arr[i].find('=');
        std::string key = str.substr(0, eqInd);
        std::string value = str.substr(eqInd + 1, str.length() - eqInd - 2);
        configMap[key] = value;
    }
}

void Config::save() {
    makePath(getConfigDir());
    std::ofstream fileStream(getConfigFile(), std::fstream::trunc);
    typedef std::map<std::string, std::string>::iterator it_type;
    for (it_type it = configMap.begin(); it != configMap.end(); it++) {
        fileStream << it->first << "=" << it->second << "\n";
    }
    fileStream.close();
}

std::string* Config::get(std::string key) {
    return configMap.count(key) > 0 ? &configMap.at(key) : NULL;
}

void Config::set(std::string key, std::string value) {
    configMap[key] = value;
    save();
}
