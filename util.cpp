#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "util.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <sys/stat.h> // stat
#include <errno.h>    // errno, ENOENT, EEXIST
#if defined(_WIN32)
#include <direct.h>   // _mkdir
#endif

// courtesy of http://stackoverflow.com/a/36401787/1988755
size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s) {
    size_t newLength = size*nmemb;
    size_t oldLength = s->size();
    try {
        s->resize(oldLength + newLength);
    } catch (std::bad_alloc &e) {
        //handle memory problem
        return 0;
    }

    std::copy((char*)contents, (char*)contents + newLength, s->begin() + oldLength);
    return size*nmemb;
}

// from https://stackoverflow.com/a/12967010/1988755
std::vector<std::string> explode(std::string const & s, char delim) {
    std::vector<std::string> result;
    std::istringstream iss(s);

    for (std::string token; std::getline(iss, token, delim); ) {
        result.push_back(std::move(token));
    }

    return result;
}

// from https://stackoverflow.com/a/29828907/1988755
bool isDirExist(const std::string& path) {
#if defined(_WIN32)
    struct _stat info;
    if (_stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & _S_IFDIR) != 0;
#else 
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
#endif
}

bool makePath(const std::string& path) {
#if defined(_WIN32)
    int ret = _mkdir(path.c_str());
#else
    mode_t mode = 0755;
    int ret = mkdir(path.c_str(), mode);
#endif
    if (ret == 0)
        return true;

    switch (errno) {
    case ENOENT:
        // parent didn't exist, try to create it
    {
        int pos = path.find_last_of('/');
        if (pos == std::string::npos)
#if defined(_WIN32)
            pos = path.find_last_of('\\');
        if (pos == std::string::npos)
#endif
            return false;
        if (!makePath(path.substr(0, pos)))
            return false;
    }
    // now, try to create again
#if defined(_WIN32)
    return 0 == _mkdir(path.c_str());
#else 
    return 0 == mkdir(path.c_str(), mode);
#endif

    case EEXIST:
        // done!
        return isDirExist(path);

    default:
        return false;
    }
}

std::string getAppdataDir() {
    if (OS == "win") {
        std::string dir = getenv("APPDATA");
        std::replace(dir.begin(), dir.end(), '\\', '/');
        return dir;
    } else {
        std::string userHome = getenv("HOME");
        if (OS == "osx") {
            return userHome + "/Library/Application Support";
        } else {
            return userHome + "/.config";
        }
    }
}

std::string getConfigDir() {
    return getAppdataDir() + "/dbo-get";
}

std::string getConfigFile() {
    return getConfigDir() + "/global.properties";
}

std::string getDownloadCache() {
    return getConfigDir() + "/download-cache";
}
