#ifdef _MSC_VER
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <assert.h>
#include <cstring>
#include <fstream>
#include <iterator>
#include <regex>
#include <stdio.h>
#include <string>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <curl/curl.h>
#include <json/json.h>

#include "config.h"
#include "dbo_project.h"
#include "store_file.h"
#include "util.h"
#include "zip_util.h"

const int DOWNLOAD_ATTEMPTS = 3;

std::string const CF_SEARCH_URL = "https://api.curseforge.com/servermods/projects?search=";
std::string const CF_QUERY_URL = "https://api.curseforge.com/servermods/files?projectIds=";

std::string DboProject::getId() {
    return DboProject::id;
}

int DboProject::getNumericId() {
    return DboProject::numId;
}

int DboProject::getVersion() {
    return DboProject::version;
}

RemoteProject::RemoteProject() {
    RemoteProject::id = "";
}

RemoteProject::RemoteProject(std::string id) {
    RemoteProject::id = id;
}

std::string RemoteProject::getFileUrl() {
    return RemoteProject::fileUrl;
}

std::string RemoteProject::getFileName() {
    return RemoteProject::fileName;
}

std::string RemoteProject::getFileMD5() {
    return RemoteProject::fileMD5;
}

Stage RemoteProject::getStage() {
    return RemoteProject::stage;
}

int RemoteProject::parseVersion(std::string url) {
    const std::regex r(R"exp(\/(\d{1,4})\/(\d{1,3})\/)exp", std::regex_constants::ECMAScript);
    std::smatch matcher;
    std::regex_search(url, matcher, r);
    if (matcher.empty()) {
        err("Failed to parse file version for project " + getId() + " (matcher failed).");
        return -1;
    }
    try {
        return std::stoi(matcher[1]) * 1000 + std::stoi(matcher[2]);
    } catch (std::invalid_argument ex) {
        err("Failed to parse file version for project " + getId() + " (malformed string).");
        return -1;
    }
}

void setOptions(CURL* curl) {
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);
    curl_easy_setopt(curl, CURLOPT_CAINFO, "res/curl-ca-bundle.crt");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
}

bool RemoteProject::resolve() {
    isResolved = (doLookup() && doQuery());
    return isResolved;
}

bool RemoteProject::doLookup() {
    printV("Looking up project " + getId() + " on remote server...");
    CURL* search = curl_easy_init();
    if (!search) {
        return false;
    }

    std::string json;
    setOptions(search);
    curl_easy_setopt(search, CURLOPT_WRITEDATA, &json);
    curl_easy_setopt(search, CURLOPT_URL, (CF_SEARCH_URL + getId()).c_str());

    CURLcode res = curl_easy_perform(search);
    if (res != CURLE_OK) {
        std::string errStr = std::string(curl_easy_strerror(res));
        err("HTTP request failed: " + errStr);
        return false;
    }

    int responseCode;
    curl_easy_getinfo(search, CURLINFO_RESPONSE_CODE, &responseCode);
    curl_easy_cleanup(search);
    if (responseCode / 100 != 2 || res == CURLE_ABORTED_BY_CALLBACK) {
        err("Remote server returned response code " + std::to_string(responseCode)
                + " on project lookup.");
        return false;
    }

    printV("Done lookup.");

    try {
        return parseId(json);
    } catch (Json::RuntimeError ex) {
        err("Remote returned malformed JSON on project lookup.");
        return false;
    }
}

bool RemoteProject::doQuery() {
    printV("Querying remote server for project information...");
    CURL* query = curl_easy_init();
    std::string json;
    setOptions(query);
    curl_easy_setopt(query, CURLOPT_WRITEDATA, &json);
    curl_easy_setopt(query, CURLOPT_URL, (CF_QUERY_URL + std::to_string(getNumericId())).c_str());

    CURLcode res = curl_easy_perform(query);
    if (res != CURLE_OK) {
        std::string errStr = std::string(curl_easy_strerror(res));
        err("HTTP request failed: " + errStr);
        return false;
    }

    int responseCode;
    curl_easy_getinfo(query, CURLINFO_RESPONSE_CODE, &responseCode);
    curl_easy_cleanup(query);
    if (responseCode / 100 != 2 || res == CURLE_ABORTED_BY_CALLBACK) {
        err("Remote server returned response code " + std::to_string(responseCode)
                + " on project query.");
        return false;
    }

    printV("Done querying.");

    try {
        return populateFields(json);
    } catch (Json::RuntimeError ex) {
        err("Remote returned malformed JSON on project query.");
        return false;
    }
}

bool RemoteProject::parseId(std::string json) {
    printV("Searching for project in returned lookup table...");
    Json::Value root;

    // jsoncpp includes some error messages that can't be disabled.
    // We don't want that, so we gotta do some real evil shit to suppress them.
    // Specifically, we're literally closing the error stream temporarily.
    #ifdef _WIN32
    int o = _dup(_fileno(stderr));
    #else
    int o = dup(fileno(stderr));
    #endif
    fclose(stderr);
    std::stringstream contentStream(json);
    std::string alts = "";
    contentStream >> root;
    // this "ISO compatibility" shit is getting real annoying man
    #ifdef _WIN32
    _dup2(o, _fileno(stderr));
    _close(o);
    #else
    dup2(o, fileno(stderr));
    close(o);
    #endif

    for (Json::ArrayIndex i = 0; i < root.size(); i++) {
        if (root[i]["slug"] == getId()) {
            numId = root[i]["id"].asInt();
            stage = stageFromString(root[i]["stage"].asString());
            printV("Found project (ID " + std::to_string(numId) + ").");
            return true;
        } else {
            alts += root[i]["slug"].asString();
            if (i < root.size() - 1) {
                alts += ", ";
            }
        }
    }
    err("Could not find project with ID " + getId() + ".");
    if (alts.length() > 0) {
        print("    Possible alternatives: " + alts);
    }
    return false;
}

bool RemoteProject::populateFields(std::string json) {
    printV("Populating project " + getId() + " with returned remote information...");
    Json::Value root;
    
    // Same deal here - evil shit to suppress jsoncpp error messages.
    #ifdef _WIN32
    int o = _dup(_fileno(stderr));
    #else
    int o = dup(fileno(stderr));
    #endif
    fclose(stderr);
    std::stringstream contentStream(json);
    contentStream >> root;
    #ifdef _WIN32
    _dup2(o, _fileno(stderr));
    _close(o);
    #else
    dup2(o, fileno(stderr));
    close(o);
    #endif

    if (root.size() == 0) {
        err("No artifacts available for project " + getId() + ".");
        return false;
    }

    Json::Value latest = root[root.size() - 1];
    std::string name = latest["name"].asString();
    fileUrl = latest["downloadUrl"].asString();
    version = parseVersion(fileUrl);
    if (version == -1) {
        return false;
    }
    fileName = latest["fileName"].asString();
    fileMD5 = latest["md5"].asString();

    if (fileUrl == "" || fileName == "" || fileMD5 == "") {
        err("Failed to fetch metadata of latest artifact for project " + getId() + ".");
        return false;
    }
    printV("Done populating.");
    return true;
}

static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    std::ofstream* out = static_cast<std::ofstream*>(userdata);
    size_t nbytes = size * nmemb;
    out->write(ptr, nbytes);
    return nbytes;
}

bool RemoteProject::download() {
    assert(isResolved);

    for (int i = 1; i <= DOWNLOAD_ATTEMPTS; i++) {
        makePath(getDownloadCache());
        std::string fileName = getDownloadCache() + "/" + getFileName();

        if (exists(*Config::getInstance().get(Config::KEY_STORE) + "/" + getFileName())
                && StoreFile::getInstance().getProject(getId())) {
            std::string conflict = "(unknown)";
            for (LocalProject* proj : *StoreFile::getInstance().getProjects()) {
                if (proj->getId() == getId()) {
                    continue;
                }
                for (std::string file : proj->getFiles()) {
                    if (file == getFileName()) {
                        conflict = proj->getId();
                    }
                }
            }

            if (conflict != "(unknown)") {
                err("File \"" + getFileName() + "\" conflicts with file installed by project " + conflict + ".");
                err("Cannot proceed, aborting.");
                return false;
            }
        }

        printV("Downloading to " + fileName + " from " + getFileUrl() + ".");
        std::ofstream output(fileName, std::ios::binary | std::ios_base::out);
        CURL* query = curl_easy_init();
        curl_easy_setopt(query, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(query, CURLOPT_WRITEDATA, &output);

        curl_easy_setopt(query, CURLOPT_URL, getFileUrl().c_str());

        curl_easy_setopt(query, CURLOPT_FOLLOWLOCATION, true);
        curl_easy_setopt(query, CURLOPT_SSL_VERIFYPEER, false);

        CURLcode res = curl_easy_perform(query);

        output.close();

        if (res != CURLE_OK) {
            std::string errStr = std::string(curl_easy_strerror(res));
            err("HTTP request failed: " + errStr);
            return false;
        }
        curl_easy_cleanup(query);

        FILE* data = fopen(fileName.c_str(), "rb");
        if (!data) {
            err("Failed to open destination file for writing.");
            return false;
        }

        printV("Verifying MD5 checksum of downloaded file...");
        std::string actualMD5 = md5(data);
        if (getFileMD5() != actualMD5) {
            err("Unexpected MD5 for file for project " + getId() + ".");
            if (i < DOWNLOAD_ATTEMPTS) {
                printQ("Retrying download (attempt " + std::to_string(i + 1) + "/" + std::to_string(DOWNLOAD_ATTEMPTS) + ")...");
                fclose(data);
                continue;
            } else {
                err("Exhausted download retry attempts - giving up.");
                fclose(data);
                return false;
            }
        }
        fclose(data);
        break;
    }
    return true;
}

bool RemoteProject::install() {
    std::vector<std::string>* files = installFiles();
    StoreFile::getInstance().addProject(new LocalProject(id, numId, version, files));
    if (!StoreFile::getInstance().save()) {
        return false;
    }
    return true;
}

std::vector<std::string>* RemoteProject::installFiles() {
    if (endsWith(getFileName(), ".jar")) {
        std::ifstream src(getDownloadCache() + "/" + getFileName(), std::ios::binary);
        if (!src.is_open()) {
            perror("Failed to read file from download cache.");
        }
        std::ofstream dst(*Config::getInstance().get(Config::KEY_STORE) + "/" + getFileName(), std::ios::binary | std::ios_base::out);
        dst << src.rdbuf();
        dst.flush();
        dst.close();
        src.close();

        std::vector<std::string>* files = new std::vector<std::string>(1); //TODO
        (*files)[0] = getFileName();
        return files;
    } else { // it's probably an archive
        print("Extracting files for project " + getId() + ".");
        return unzip(getDownloadCache() + "/" + getFileName(), *Config::getInstance().get(Config::KEY_STORE));
    }
}

LocalProject::LocalProject() {
}

LocalProject::LocalProject(std::string id, int numId, int version, std::vector<std::string>* files) {
    LocalProject::id = id;
    LocalProject::numId = numId;
    LocalProject::version = version;
    LocalProject::files = *files;
}

std::vector<std::string> LocalProject::getFiles() {
    return LocalProject::files;
}

bool LocalProject::remove() {
    for (size_t i = 0; i < getFiles().size(); i++) {
        std::remove((*Config::getInstance().get(Config::KEY_STORE) + "/" + getFiles()[i]).c_str());
    }
    StoreFile::getInstance().removeProject(getId());
    return true;
}
