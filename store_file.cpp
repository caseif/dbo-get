#include <algorithm>
#include <assert.h>
#include <fstream>
#include <string>

#include <json/json.h>

#include "config.h"
#include "store_file.h"
#include "util.h"

typedef std::map<std::string, LocalProject>::iterator str_locProj_map_it;

StoreFile::StoreFile() {
    projects = std::map<std::string, LocalProject>();
    if (!load()) {
        exit(1);
    }
}

StoreFile& StoreFile::getInstance() {
    static StoreFile instance;
    return instance;
}

std::string StoreFile::getPath() {
    return *Config::getInstance().get(Config::KEY_STORE) + "/.dbostore";
}

bool StoreFile::load() {
    assert(!initialized);

    std::string path = getPath();

    printV("Loading store file from disk...");
    printV("Loading from " + path + ".");

    if (existsDir(path)) {
        err("Cannot load store file: path is directory.");
        return false;
    }

    Json::Value root;
    std::ifstream fileStream(path);
    if (!fileStream.is_open()) {
        initialized = true;
        return true;
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
    printV("Done loading store file.");

    return true;
}

bool StoreFile::save() {
    std::string path = getPath();

    printV("Saving store file to disk...");
    printV("Saving to " + path);

    if (existsDir(path)) {
        err("Cannot save store file: provided path is directory.");
        return false;
    }

    Json::Value root = Json::Value();
    Json::ArrayIndex ind = 0;
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

    std::ofstream fileStream(path, std::fstream::trunc);
    fileStream << root;
    fileStream.close();

    printV("Done saving store file.");

    return true;
}

LocalProject* StoreFile::getProject(std::string id) {
    assert(initialized);
    return projects.count(id) != 0  ? &projects.at(id) : NULL;
}

std::vector<LocalProject*>* StoreFile::getProjects() {
    assert(initialized);
    std::vector<LocalProject*>* vec = new std::vector<LocalProject*>(projects.size());
    typedef std::map<std::string, LocalProject>::iterator str_locProj_map_it;
    int i = 0;
    for (str_locProj_map_it it = projects.begin(); it != projects.end(); it++) {
        (*vec)[i++] = &it->second;
    }
    return vec;
}

std::vector<std::string>* StoreFile::getProjectIds() {
    assert(initialized);
    std::vector<std::string>* vec = new std::vector<std::string>(projects.size());
    typedef std::map<std::string, LocalProject>::iterator str_locProj_map_it;
    int i = 0;
    for (str_locProj_map_it it = projects.begin(); it != projects.end(); it++) {
        (*vec)[i++] = it->second.getId();
    }
    return vec;
}

void StoreFile::addProject(LocalProject* project) {
    assert(initialized);
    projects[project->getId()] = *project;
}

void StoreFile::removeProject(std::string id) {
    assert(initialized);
    projects.erase(id);
}
