#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <curl/curl.h>

#include "main.h"
#include "dbo_project.h"
#include "config.h"
#include "util.h"

std::string const CMD_STORE = "store";
std::string const CMD_INSTALL = "install";
std::string const CMD_REMOVE = "remove";
std::string const CMD_UPGRADE = "upgrade";
std::string const CMD_HELP = "help";
std::string const cmds[] = {CMD_STORE, CMD_INSTALL, CMD_REMOVE, CMD_UPGRADE, CMD_HELP};

std::string const USG_STORE = "[location]";
std::string const USG_INSTALL = "<projects>...";
std::string const USG_UPGRADE = "";
std::string const USG_REMOVE = "<projects>...";
std::string const USG_HELP = "[command]";

int main(int argc, char* argv[]) {
    if (argc < 2) {
        err("Too few args!");
        print("    Usage: dbo_get <command>");
        return 1;
    }

    Config::load();

    char* cmd = argv[1];
    if (matchCmd(cmd, CMD_STORE)) {
        return setStore(argc, argv);
    } else if (matchCmd(cmd, CMD_INSTALL)) {
        return install(argc, argv);
    } else if (matchCmd(cmd, CMD_REMOVE)) {
        if (argc < 3) {
            tooFewArgs(CMD_REMOVE + "<projects>...");
            return 1;
        }
        err("Command not yet implemented");
        return 1;
    } else if (matchCmd(cmd, CMD_UPGRADE)) {
        err("Command not yet implemented");
        return 1;
    } else if (matchCmd(cmd, CMD_HELP)) {
        err("Command not yet implemented");
        return 1;
    } else {
        err("Invalid command, try `dbo-get help`.");
        return 1;
    }
}

int setStore(int argc, char* argv[]) {
    if (argc < 3) {
        std::string* loc = Config::get(Config::KEY_STORE);
        print("Current store location: " + (loc == NULL ? "NOT SET" : *loc));
        return 0;
    }

    std::string path = "";
    for (int i = 2; i < argc; i++) {
        path += argv[i];
        if (i < argc - 1) {
            path += " ";
        }
    }
    std::replace(path.begin(), path.end(), '\\', '/');
    Config::set(Config::KEY_STORE, path);
    print("Successfully set store location as \"" + path + "\"");

    return 0;
}

std::vector<DboProject*> resolve(int argc, char* argv[]) {
    std::vector<DboProject*> vec = std::vector<DboProject*>(argc - 2);
    bool fail = false;
    curl_global_init(CURL_GLOBAL_ALL);
    for (int i = 0; i < argc - 2; i++) {
        std::string project = argv[2 + i];
        DboProject* dbo = new DboProject(project);
        vec[i] = dbo;

        if (!dbo->resolve()) {
            err("Failed to resolve project " + project);
            fail = true;
        }
    }
    curl_global_cleanup();
    return fail ? std::vector<DboProject*>() : vec;
}

int install(int argc, char* argv[]) {
    if (argc < 3) {
        tooFewArgs(CMD_INSTALL + " " + USG_INSTALL);
        return 1;
    }

    std::string* loc = Config::get(Config::KEY_STORE);
    if (loc == NULL) {
        err("Store location is not set; please run store command first");
        return 1;
    }
    makePath(*loc);

    std::vector<DboProject*> projects = resolve(argc, argv);
    if (projects.size() == 0) {
        err("No projects specified for installation");
        return 1;
    }

    for (size_t i = 0; i < projects.size(); i++) {
        if (!projects[i]->install(*loc)) {
            return 1;
        }
    }
    return 0;
}
