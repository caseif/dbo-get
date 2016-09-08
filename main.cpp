#include <iostream>
#include <string>
#include <vector>

#include <curl/curl.h>

#include "main.h"
#include "DboProject.h"
#include "util.h"

char* const CMD_INSTALL = "install";
char* const CMD_REMOVE = "remove";
char* const CMD_UPGRADE = "upgrade";
char* const CMD_HELP = "help";
char* const cmds[] = {"install", "remove", "upgrade", "help"};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        err("Too few args!");
        print("    Usage: dbo_get <command> <projects>...");
        return 1;
    }

    char* cmd = argv[1];
    if (std::strcmp(cmd, CMD_INSTALL) == 0) {
        return install(argc, argv);
    } else if (std::strcmp(cmd, CMD_REMOVE) == 0) {
        if (argc < 3) {
            tooFewArgs(cmd);
            return 1;
        }
        err("Command not yet implemented");
        return 1;
    } else if (std::strcmp(cmd, CMD_UPGRADE) == 0) {
        err("Command not yet implemented");
        return 1;
    } else if (std::strcmp(cmd, CMD_HELP) == 0) {
        err("Command not yet implemented");
        return 1;
    } else {
        err("Invalid command, try `dbo-get help`.");
        return 1;
    }
}

std::vector<DboProject>* resolve(int argc, char* argv[]) {
    std::vector<DboProject> vec = std::vector<DboProject>(argc - 2);
    bool fail = false;
    curl_global_init(CURL_GLOBAL_ALL);
    for (int i = 0; i < argc - 2; i++) {
        std::string project = argv[2 + i];
        DboProject dbo = DboProject(project);
        vec[i] = dbo;

        if (!dbo.resolve()) {
            err("Failed to resolve project " + project);
            std::string alt = dbo.getAlternatives();
            if (alt.length() > 0) {
                print("    Possible alternatives: " + *alt.c_str());
            }
            fail = true;
        }
    }
    curl_global_cleanup();
    return fail ? NULL : &vec;
}

int install(int argc, char* argv[]) {
    if (argc < 3) {
        std::string usage = CMD_INSTALL;
        usage += " <projects>...";
        tooFewArgs(usage);
        return 1;
    }

    std::vector<DboProject>* projects = resolve(argc, argv);
    if (projects == NULL) {
        return 1;
    }

    for (size_t i = 0; i < projects->size(); i++) {
        if (!(*projects)[i].install()) {
            return 1;
        }
    }
    return 0;
}
