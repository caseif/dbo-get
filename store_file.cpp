#include <algorithm>
#include <assert.h>
#include <fstream>
#include <string>

#include "config.h"
#include "json/json.h"
#include "store_file.h"
#include "util.h"

StoreFile::StoreFile() {
    projects = std::map<std::string, LocalProject>();
    load();
}

StoreFile& StoreFile::getInstance() {
    static StoreFile instance;
    return instance;
}

std::string StoreFile::getPath() {
    return *Config::getInstance().get(Config::KEY_STORE) + "/.dbostore";
}

void StoreFile::load() {
    assert(!initialized);

    Json::Value root;
    std::ifstream fileStream(getPath());
    if (!fileStream.is_open()) {
        initialized = true;
        return;
    }
    fileStream >> root;

    for (Json::ArrayIndex i = 0; i < root.size(); i++) {
        Json::Value projJson = root[i];
        std::string id = projJson["id"].asString();
        int numId = projJson["numId"].asInt();
        int version = projJson["version"].asInt();
        Json::Value files = projJson["files"];

        int size = files.size();
        std::vector<std::string> fileVec = std::vector<std::string>(size);
        for (Json::ArrayIndex j = 0; j < size; j++) {
            fileVec[j] = files[j].asString();
        }

        projects[id] = LocalProject(id, numId, version, &fileVec);
    }

    initialized = true;
}

void StoreFile::save() {
    Json::Value root = Json::Value();
    Json::ArrayIndex ind = 0;
    typedef std::map<std::string, LocalProject>::iterator str_locProj_map_it;
    for (str_locProj_map_it it = projects.begin(); it != projects.end(); it++) {
        Json::Value projJson = Json::Value();
        projJson["id"] = it->second.getId();
        projJson["numId"] = it->second.getNumericId();
        projJson["version"] = it->second.getVersion();

        Json::Value files = Json::Value(Json::arrayValue);
        std::vector<std::string> fileVec = it->second.getFiles();
        for (Json::ArrayIndex fi = 0; fi < fileVec.size(); fi++) {
            files[fi] = fileVec[fi];
        }
        projJson["files"] = files;
        root[ind] = projJson;
        ind++;
    }

    std::ofstream fileStream(getPath(), std::fstream::trunc);
    fileStream << root;
    fileStream.close();
}

LocalProject* StoreFile::getProject(std::string id) {
    assert(initialized);
    return projects.count(id) != 0  ? &projects.at(id) : NULL;
}

void StoreFile::addProject(LocalProject* project) {
    assert(initialized);
    projects[project->getId()] = *project;
}

void StoreFile::removeProject(std::string id) {
    assert(initialized);
    projects.erase(id);
}
