#include <assert.h>
#include <cstring>
#include <fstream>
#include <string>

#include <curl/curl.h>
#include "json/json.h"

#include "DboProject.h"
#include "util.h"

std::string const CF_SEARCH_URL = "https://api.curseforge.com/servermods/projects?search=";
std::string const CF_QUERY_URL = "https://api.curseforge.com/servermods/files?projectIds=";

DboProject::DboProject() {
    DboProject::id = "";
}

DboProject::DboProject(std::string id) {
    DboProject::id = id;
}

std::string DboProject::getId() {
    return DboProject::id;
}

int DboProject::getNumericId() {
    return DboProject::numId;
}

std::string DboProject::getLatestVersion() {
    return DboProject::latestVer;
}

std::string DboProject::getFileUrl() {
    return DboProject::fileUrl;
}

std::string DboProject::getFileName() {
    return DboProject::fileName;
}

std::string DboProject::getFileMD5() {
    return DboProject::fileName;
}

std::string parseVersion(std::string name) {
    //TODO
    return "";
}

void setOptions(CURL* curl) {
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);
    curl_easy_setopt(curl, CURLOPT_CAINFO, "res/curl-ca-bundle.crt");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
}

bool DboProject::resolve() {
    print("Resolving project " + getId() + "...");

    isResolved = (doLookup() && doQuery());
    return isResolved;
}

bool DboProject::doLookup() {
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
        err("curl_easy_perform() failed: " + errStr);
        return false;
    }
    curl_easy_cleanup(search);

    return parseId(json);
}

bool DboProject::doQuery() {
    CURL* query = curl_easy_init();
    std::string json;
    setOptions(query);
    curl_easy_setopt(query, CURLOPT_WRITEDATA, &json);
    curl_easy_setopt(query, CURLOPT_URL, (CF_QUERY_URL + std::to_string(getNumericId())).c_str());

    CURLcode res = curl_easy_perform(query);
    if (res != CURLE_OK) {
        std::string errStr = std::string(curl_easy_strerror(res));
        err("curl_easy_perform() failed: " + errStr);
        return false;
    }
    curl_easy_cleanup(query);

    return populateFields(json);
}

bool DboProject::parseId(std::string json) {
    Json::Value root;
    std::stringstream contentStream(json);
    std::string alts = "";
    contentStream >> root;
    for (Json::ArrayIndex i = 0; i < root.size(); i++) {
        if (root[i].get("slug", "") == getId()) {
            numId = root[i].get("id", -1).asInt();
            return true;
        } else {
            alts += root[i].get("slug", "").asString();
            if (i < root.size() - 1) {
                alts += ", ";
            }
        }
    }
    err("Could not find project with ID " + getId());
    if (alts.length() > 0) {
        print("    Possible alternatives: " + alts);
    }
    return false;
}

bool DboProject::populateFields(std::string json) {
    Json::Value root;
    std::stringstream contentStream(json);
    contentStream >> root;
    if (root.size() == 0) {
        err("No artifacts available for project " + getId());
        return false;
    }
    Json::Value latest = root[root.size() - 1];
    std::string name = latest.get("name", "").asString();
    latestVer = parseVersion(name);
    fileUrl = latest.get("downloadUrl", "").asString();
    fileName = latest.get("fileName", "").asString();
    fileMD5 = latest.get("md5", "").asString();

    if (fileUrl == "" || fileName == "" || fileMD5 == "") {
        err("Failed to fetch metadata of latest artifact for project " + getId());
        return false;
    }
    return true;
}

size_t write_callback(void* buffer, size_t size, size_t nitems, void* userp) {
    FILE* file = (FILE*)userp;
    size_t write;
    size *= nitems;
    write = fwrite(buffer, size, nitems, file);
    return size;
}

bool DboProject::install(std::string storeLoc) {
    assert(isResolved);
    print("Installing project " + getId() + "...");
    
    CURL* query = curl_easy_init();
    FILE* data;
    fopen_s(&data, storeLoc.c_str(), "wb");
    curl_easy_setopt(query, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(query, CURLOPT_FILE, data);

    std::string url = getFileUrl();
    curl_easy_setopt(query, CURLOPT_URL, url);

    curl_easy_setopt(query, CURLOPT_FOLLOWLOCATION, true);
    curl_easy_setopt(query, CURLOPT_SSL_VERIFYPEER, false);

    CURLcode res = curl_easy_perform(query);
    if (res != CURLE_OK) {
        std::string errStr = std::string(curl_easy_strerror(res));
        err("curl_easy_perform() failed: " + errStr);
        return false;
    }
    curl_easy_cleanup(query);

    print("Done installing " + getId());

    return true;
}

bool DboProject::remove(std::string storeLoc) {
    return false; //TODO
}
