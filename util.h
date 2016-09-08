#include <iostream>
#include <string>
#include <sstream>
#include <vector>

static inline void print(std::string str) {
    std::cout << str.c_str() << std::endl;
}

static inline void err(std::string str) {
    std::cout << "E: " << str.c_str() << std::endl;
}

static inline void tooFewArgs(char* usage) {
    err("Too few args!");
    std::string str(usage);
    print("    Usage: dbo_get " + str + " <projects>...");
}
