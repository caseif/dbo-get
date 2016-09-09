#include <assert.h>
#include <cstring>
#include <string>

#include <curl/curl.h>

#include "DboProject.h"
#include "util.h"

std::string const CF_SEARCH_URL = "https://api.curseforge.com/servermods/projects?search=";

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
    assert(DboProject::isResolved);
    return DboProject::numId;
}

std::string DboProject::getLatestVersion() {
    assert(DboProject::isResolved);
    return DboProject::latestVer;
}

std::string DboProject::getAlternatives() {
    assert(!DboProject::isResolved);
    return DboProject::alternatives;
}

bool DboProject::resolve() {
    print("Resolving project " + getId() + "...");

    CURL* curl = curl_easy_init();
	if (!curl) {
		return false;
	}

	std::string json;
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);
	curl_easy_setopt(curl, CURLOPT_CAINFO, "res/curl-ca-bundle.crt");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json);
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);

    curl_easy_setopt(curl, CURLOPT_URL, (CF_SEARCH_URL + getId()).c_str());
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::string errStr = std::string(curl_easy_strerror(res));
		err("curl_easy_perform() failed: " + errStr);
		return false;
    }
	print(json);
    curl_easy_cleanup(curl);

	isResolved = true;
    return true; //TODO
}

bool DboProject::install() {
    print("Installing project " + getId() + "...");
    return true; //TODO
}

bool DboProject::remove() {
    return false; //TODO
}
