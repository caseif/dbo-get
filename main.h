#pragma once

#include <vector>
#include "dbo_project.h"

std::vector<std::string>* parseParams(int argc, char* argv[]);

int setStore(int argc, char* argv[]);

std::vector<RemoteProject*>* resolve(std::vector<std::string>* projects);

int handleInstallCmd(int argc, char* argv[]);

int install(std::vector<std::string>* projects);

int handleRemoveCmd(int argc, char* argv[]);

int remove(std::vector<std::string>* projects);

bool matchCmd(char* input, std::string cmd) {
    return std::strcmp(input, cmd.c_str()) == 0;
}
