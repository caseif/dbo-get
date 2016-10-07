#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <curl/curl.h>

#include "config.h"
#include "dbo_project.h"
#include "main.h"
#include "store_file.h"
#include "util.h"

static const int INSTALL_DIALOG_LINE_LENGTH = 80;

std::string const CMD_STORE = "store";
std::string const CMD_INSTALL = "install";
std::string const CMD_REMOVE = "remove";
std::string const CMD_UPGRADE = "upgrade";
std::string const CMD_HELP = "help";
std::string const CMD_MOO = "moo";
std::string const cmds[] = {CMD_STORE, CMD_INSTALL, CMD_REMOVE, CMD_UPGRADE, CMD_HELP, CMD_MOO};

static std::string const USG_STORE = "[location]";
static std::string const USG_INSTALL = "<projects>...";
static std::string const USG_UPGRADE = "";
static std::string const USG_REMOVE = "<projects>...";
static std::string const USG_HELP = "[command]";

static std::vector<RemoteProject*>* const EMPTY_RPP_VEC = new std::vector<RemoteProject*>(0);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        err("Too few args!");
        print("    Usage: dbo_get <command>");
        return 1;
    }

    Config::getInstance().load();

    char* cmd = argv[1];
    if (matchCmd(cmd, CMD_STORE)) {
        return setStore(argc, argv);
    } else if (matchCmd(cmd, CMD_INSTALL)) {
        return handleInstallCmd(argc, argv);
    } else if (matchCmd(cmd, CMD_REMOVE)) {
        return handleRemoveCmd(argc, argv);
    } else if (matchCmd(cmd, CMD_UPGRADE)) {
        return handleUpgradeCmd(argc, argv);
        return 1;
    } else if (matchCmd(cmd, CMD_HELP)) {
        err("Command not yet implemented.");
        return 1;
    } else if (matchCmd(cmd, CMD_MOO)) {
        print("Yes, you're very clever.");
    } else {
        err("Invalid command, try `dbo-get help`.");
        return 1;
    }
}

std::vector<std::string>* parseParams(int argc, char* argv[]) {
    std::vector<std::string>* params = new std::vector<std::string>(argc - 2);
    int j = 0;
    for (int i = 2; i < argc; i++) {
        if (argv[i][0] != '-') {
            (*params)[j++] = argv[i];
        }
    }
    return params;
}

int setStore(int argc, char* argv[]) {
    if (argc < 3) {
        std::string* loc = Config::getInstance().get(Config::KEY_STORE);
        print("Current store location: " + (loc == NULL ? "NOT SET" : *loc));
        return 0;
    }

    std::string path = "";
    std::vector<std::string>* params = parseParams(argc, argv);
    for (int i = 0; i < params->size(); i++) {
        path += (*params)[i];
        if (i < params->size() - 1) {
            path += " ";
        }
    }
    delete(params);
    std::replace(path.begin(), path.end(), '\\', '/');
    Config::getInstance().set(Config::KEY_STORE, path);
    makePath(path);
    print("Successfully set store location as \"" + path + "\".");

    return 0;
}

std::vector<RemoteProject*>* resolve(std::vector<std::string>* projects, bool ignoreFail) {
    print("Resolving projects...");

    std::vector<RemoteProject*>* vec = new std::vector<RemoteProject*>(projects->size());
    bool fail = false;
    bool empty = true;
    curl_global_init(CURL_GLOBAL_ALL);
    for (int i = 0; i < projects->size(); i++) {
        std::string id = (*projects)[i];
        print("Resolving project " + id + "...");
        RemoteProject* remote = new RemoteProject(id);

        if (!remote->resolve() && !ignoreFail) {
            err("Failed to resolve " + id + ".");
            fail = true;
            return NULL;
        }

        LocalProject* local = StoreFile::getInstance().getProject(id);

        if (local == NULL || remote->getVersion() > local->getVersion()) {
            (*vec)[i] = remote;
            empty = false;
        }

        print("Done resolving " + id + ".");
    }
    curl_global_cleanup();
    return fail ? NULL : (!empty ? vec : EMPTY_RPP_VEC);
}

static void printDialogListing(std::vector<std::string> projects) {
    std::string line = "  ";
    for (size_t i = 0; i < projects.size(); i++) {
        if (projects[i] == "") {
            break;
        }
        std::string newLine = line + projects[i];
        if (line.length() + 1 > INSTALL_DIALOG_LINE_LENGTH) {
            print(line);
            line = "  " + projects[i];
        } else {
            line = newLine + " ";
        }
    }
    print(line);
}

int handleInstallCmd(int argc, char* argv[]) {
    if (argc < 3) {
        tooFewArgs(CMD_INSTALL, USG_INSTALL);
        return 1;
    }
    std::vector<std::string>* projects = parseParams(argc, argv);
    install(projects, false);
}

int handleUpgradeCmd(int argc, char* argv[]) {
    if (argc >= 3) {
        tooFewArgs(CMD_UPGRADE, USG_UPGRADE);
        return 1;
    }
    std::vector<std::string>* projects = StoreFile::getInstance().getProjectIds();
    install(projects, true);
}

int install(std::vector<std::string>* projects, bool ignoreFail) {
    std::string* loc = Config::getInstance().get(Config::KEY_STORE);
    if (loc == NULL) {
        err("Store location is not set; please run store command first.");
        return 1;
    }
    makePath(*loc);

    std::vector<RemoteProject*>* resolved = resolve(projects, ignoreFail);

    delete(projects);

    if (resolved == NULL) {
        err("No projects specified for installation.");
        return 1;
    }

    if (resolved->empty()) {
        print("Already up-to-date.");
        return 0;
    }

    printInstallDialog(resolved);

    for (size_t i = 0; i < resolved->size(); i++) {
        RemoteProject* proj = (*resolved)[i];
        if (proj == NULL) {
            continue;
        }

        LocalProject* local = StoreFile::getInstance().getProject(proj->getId());
        if (local != NULL) {
            print("Upgrading project " + proj->getId() + " (#" + std::to_string(local->getVersion()) + " -> #" + std::to_string(proj->getVersion()) + ").");
            if (!local->remove() || !proj->install()) {
                //TODO: this shit ain't atomic
                return 1;
            }
        } else {
            print("Installing project " + proj->getId() + "...");
            if (!proj->install()) {
                return 1;
            }
        }
        print("Done installing " + proj->getId() + ".");
    }

    StoreFile::getInstance().save();

    return 0;
}

int handleRemoveCmd(int argc, char* argv[]) {
    if (argc < 3) {
        tooFewArgs(CMD_REMOVE, USG_REMOVE);
        return 1;
    }
}

int remove(std::vector<std::string>* projects) {
    std::string* loc = Config::getInstance().get(Config::KEY_STORE);
    if (loc == NULL) {
        err("Store location is not set; please run store command first.");
        return 1;
    }
    makePath(*loc);

    bool fail = false;;
    std::vector<LocalProject> resolved = std::vector<LocalProject>(projects->size());
    for (int i = 0; i < projects->size(); i++) {
        std::string id = (*projects)[i];
        print("Resolving project " + id + "...");
        LocalProject* proj = StoreFile::getInstance().getProject(id);
        if (proj != NULL) {
            if (!fail) {
                resolved[i] = *proj;
            }
        } else {
            err("No project with ID " + id + " is currently installed.");
            fail = true;
        }
    }

    delete(projects);

    if (fail) {
        err("No projects specified for removal.");
        return 1;
    }

    printRemoveDialog(&resolved);

    for (size_t i = 0; i < resolved.size(); i++) {
        LocalProject proj = resolved[i];
        print("Removing project " + proj.getId() + "...");
        proj.remove();
        print("Done removing " + proj.getId() + ".");
    }

    StoreFile::getInstance().save();

    return 0;
}

static void printInstallDialog(std::vector<RemoteProject*>* projects) {
    std::vector<std::string> upgradeList = std::vector<std::string>(projects->size());
    std::vector<std::string> installList = std::vector<std::string>(projects->size());
    int ui = 0;
    int ii = 0;
    int nu = 0;
    for (size_t i = 0; i < projects->size(); i++) {
        RemoteProject* remote = (*projects)[i];
        if (remote == NULL) {
            nu++;
            continue;
        }
        LocalProject* local = StoreFile::getInstance().getProject(remote->getId());
        if (local != NULL) {
            upgradeList[ui++] = remote->getId();
        } else {
            installList[ii++] = remote->getId();
        }
    }

    if (ui > 0) {
        print("The following projects will be upgraded:");
        printDialogListing(upgradeList);
    }

    if (ii > 0) {
        print("The following projects will be newly installed:");
        printDialogListing(installList);
    }

    print(std::to_string(ui) + " upgraded, " + std::to_string(ii) + " newly installed, 0 to remove, "
        + std::to_string(nu) + " not upgraded.");
}

static void printRemoveDialog(std::vector<LocalProject>* projects) {
    std::vector<std::string> removeList = std::vector<std::string>(projects->size());
    for (size_t i = 0; i < projects->size(); i++) {
        removeList[i] = (*projects)[i].getId();
    }
    printDialogListing(removeList);

    print("0 upgraded, 0 newly installed, " + std::to_string(projects->size()) + " to remove, 0 not upgraded.");
}
