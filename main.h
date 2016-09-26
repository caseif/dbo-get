#pragma once

#include <vector>
#include "dbo_project.h"

int setStore(int argc, char* argv[]);

std::vector<DboProject*> resolve(int argc, char* argv[]);

int install(int argc, char* argv[]);

bool matchCmd(char* input, std::string cmd) {
    return std::strcmp(input, cmd.c_str()) == 0;
}
