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
#include <vector>

inline void print(std::string str) {
    std::cout << str.c_str() << std::endl;
}

inline void err(std::string str) {
    std::cout << "E: " << str.c_str() << std::endl;
}

inline void tooFewArgs(std::string command, std::string usage) {
    err("Too few args!");
    print("    Usage: dbo-get " + command + " " + usage);
}

inline void tooManyArgs(std::string command, std::string usage) {
    err("Too many args!");
    print("    Usage: dbo-get " + command + " " + usage);
}

size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s);

std::vector<std::string> explode(std::string const & s, char delim);

bool makePath(const std::string& path);

std::string getAppdataDir();

std::string getConfigDir();

std::string getConfigFile();

std::string getDownloadCache();

std::string md5(FILE* file);

inline bool ends_with(std::string const &value, std::string const &ending) {
	if (ending.size() > value.size()) {
		return false;
	}
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}
