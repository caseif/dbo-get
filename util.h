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

#include <iostream>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include <vector>

#include "flags.h"

inline void printQ(std::string str) {
    if (!testFlag(Flag::kSilent)) {
        std::cout << str.c_str() << std::endl;
    }
}

inline void print(std::string str) {
    if (!testFlag(Flag::kQuiet) && !testFlag(Flag::kSilent)) {
        std::cout << str.c_str() << std::endl;
    }
}

inline void printV(std::string str) {
    if (testFlag(Flag::kVerbose)) {
        std::cout << str.c_str() << std::endl;
    }
}

inline void err(std::string str) {
    std::cout << "E: " << str.c_str() << std::endl;
}

inline void tooFewArgs(std::string command, std::string usage) {
    err("Too few args!");
    print("    Usage: dbo-get [flags] " + command + " " + usage);
}

inline void tooManyArgs(std::string command, std::string usage) {
    err("Too many args!");
    print("    Usage: dbo-get [flags] " + command + " " + usage);
}

inline void invalidFlag(std::string flag) {
    err("Unknown flag '" + flag + "', try `dbo-get help`.");
}

inline void invalidFlag(char flag) {
    err("Unknown flag '" + std::string(&flag, 1) + "', try `dbo-get help`.");
}

inline void printArt() {
    printQ(R"R(           _.  )R");
    printQ(R"R(        ;=',_  )R");
    printQ(R"R(       S" .--` )R");
    printQ(R"R(      sS  \__  )R");
    printQ(R"R(   __.' ( \--> )R");
    printQ(R"R( _=/    _./-\/  )R");
    printQ(R"R(((\( /-'   -'l  )R");
    printQ(R"R( ) |/ \\    )R");
    printQ(R"R(    \\  \ )R");
    printQ(R"R(     `~ `~)R");
}

size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s);

std::vector<std::string> explode(std::string const & s, char delim);

bool makePath(const std::string& path);

std::string getAppdataDir();

std::string getConfigDir();

std::string getConfigFile();

std::string getDownloadCache();

std::string md5(FILE* file);

inline bool endsWith(std::string const &value, std::string const &ending) {
    if (ending.size() > value.size()) {
        return false;
    }
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

inline bool isDirectory(std::string path) {
    //TODO: dis shit ain't portable
    struct stat fs;
    return stat(path.c_str(), &fs) && (fs.st_mode & S_IFDIR);
}
