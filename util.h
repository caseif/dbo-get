#pragma once

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

inline void tooFewArgs(std::string usage) {
    err("Too few args!");
    std::string str(usage);
    print("    Usage: dbo_get " + str + " <projects>...");
}

size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s);

std::vector<std::string> explode(const std::string& str, const char& ch);

bool makePath(const std::string& path);
