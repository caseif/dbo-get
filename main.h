#pragma once

#define JSON_DLL // required on Windows

#include <vector>

#include "command.h"
#include "dbo_project.h"
#include "flags.h"

void parseParamsAndFlags(int argc, char* argv[]);

int handleStoreCmd(int argc, char* argv[]);
int handleInstallCmd(int argc, char* argv[]);
int handleUpgradeCmd(int argc, char* argv[]);
int handleRemoveCmd(int argc, char* argv[]);
int handleHelpCmd(int argc, char* argv[]);
int handleMooCmd(int argc, char* argv[]);

void printInfoHeader();
void printHelp(Command* cmd);

std::vector<RemoteProject*>* resolve(std::vector<std::string>* projects, bool ignoreFail);

int install(std::vector<std::string>* projects, bool ignoreFail);
int remove(std::vector<std::string>* projects);

bool matchCmd(char* input, std::string cmd) {
    return std::strcmp(input, cmd.c_str()) == 0;
}

static void printInstallDialog(std::vector<RemoteProject*>* projects);
static void printRemoveDialog(std::vector<LocalProject>* projects);
