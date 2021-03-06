#include <algorithm>
#include <cstring>
#include <functional>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <curl/curl.h>

#include "command.h"
#include "config.h"
#include "dbo_project.h"
#include "flags.h"
#include "main.h"
#include "stage.h"
#include "store_file.h"
#include "util.h"

static const std::string kVersion = "0.4.0";

static const int kInstallDialogLineLength = 80;
static const int kHelpCmdIndentSize = 10;
static const int kHelpFlagIndentSize = 18;

Command* const kCmdStore = new Command("store", "[location]",
        "Gets or sets the current store location.", &handleStoreCmd);
Command* const kCmdInstall = new Command("install", "<projects>...",
        "Installs or upgrades the specified projects to the current store.", &handleInstallCmd);
Command* const kCmdUpgrade = new Command("upgrade", "",
        "Attempts to upgrade all projects installed to the current store.", &handleUpgradeCmd);
Command* const kCmdRemove = new Command("remove", "<projects>...",
        "Removes the specified projects from the current store.", &handleRemoveCmd);
Command* const kCmdHelp = new Command("help", "[command]",
        "Displays help for one or all commands.", &handleHelpCmd);
Command* const kCmdMoo = new Command("moo", "",
        "Have you mooed today?", &handleMooCmd, false);
Command* const kCmds[] = {kCmdStore, kCmdInstall, kCmdRemove, kCmdUpgrade, kCmdHelp, kCmdMoo};

static std::vector<RemoteProject*>* const kEmptyRppVec = new std::vector<RemoteProject*>(0);

static std::vector<std::string>* params;

int main(int argc, char* argv[]) {
    parseParamsAndFlags(argc, argv);

    if (argc < 2) {
        err("Too few args!");
        print("    Usage: dbo_get <command>");
        return 1;
    }

    char* cmd = argv[1];
    for (auto it = std::begin(kCmds); it != std::end(kCmds); ++it) {
        if (matchCmd(cmd, (*it)->getLabel())) {
            return (*it)->getHandler()(argc, argv);
        }
    }
    err("Invalid command, try `dbo-get help`.");
    return 1;
}

void parseParamsAndFlags(int argc, char* argv[]) {
    int paramCount = 0;
    int flagCount = 0;
    for (int i = 2; i < argc; i++) {
        if (argv[i][0] == '-') {
            flagCount += argv[i][1] == '-' ? 1 : std::strlen(argv[i]) - 1;
        } else {
            paramCount++;
        }
    }

    params = new std::vector<std::string>(paramCount);
    std::vector<Flag>* flags = new std::vector<Flag>(flagCount);
    int p = 0;
    int f = 0;
    for (int i = 2; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == '-') {
                std::string name = std::string(argv[i] + 2);
                const Flag* flag = matchFlag(name);
                if (flag == NULL) {
                    invalidFlag(name);
                    exit(1);
                } else {
                    (*flags)[f++] = *flag;
                }
            } else {
                for (size_t j = 1; j < std::strlen(argv[i]); j++) {
                    const Flag* flag = matchFlag(argv[i][j]);
                    if (flag == NULL) {
                        invalidFlag(argv[i][j]);
                        exit(1);
                    } else {
                        (*flags)[f++] = *flag;
                    }
                }
            }
        } else {
            (*params)[p++] = argv[i];
        }
    }

    setFlags(flags);

    int collisions = testFlag(Flag::kQuiet) + testFlag(Flag::kVerbose) + testFlag(Flag::kSilent);
    if (collisions > 1) {
        err("Quiet, verbose, and silent flags cannot be used in conjunction with each other.");
        exit(1);
    }
}

int handleStoreCmd(int argc, char* argv[]) {
    if (argc < 3) {
        std::string* loc = Config::getInstance().get(Config::KEY_STORE);
        printQ("Current store location: " + (loc == NULL ? "NOT SET" : *loc));
        return 0;
    }

    std::string path = "";
    for (size_t i = 0; i < params->size(); i++) {
        path += (*params)[i];
        if (i < params->size() - 1) {
            path += " ";
        }
    }
    delete(params);
    std::replace(path.begin(), path.end(), '\\', '/');

    if (existsNonDir(path)) {
        err("Cannot set store path: provided path exists and is not directory.");
    }

    Config::getInstance().set(Config::KEY_STORE, path);
    if (!makePath(path)) {
        err("Failed to make store path.");
        return 1;
    }
    printQ("Successfully set store location as \"" + path + "\".");

    return 0;
}

std::vector<RemoteProject*>* resolve(std::vector<std::string>* projects, bool ignoreFail) {
    printQ("Resolving projects...");

    std::vector<RemoteProject*>* vec = new std::vector<RemoteProject*>(projects->size());
    bool fail = false;
    bool empty = true;
    curl_global_init(CURL_GLOBAL_ALL);
    bool forceUpdate = testFlag(Flag::kUpdate);
    for (size_t i = 0; i < projects->size(); i++) {
        std::string id = (*projects)[i];
        print("Resolving project " + id + "...");
        RemoteProject* remote = new RemoteProject(id);

        if (!remote->resolve() && !ignoreFail) {
            err("Failed to resolve " + id + ".");
            fail = true;
            return NULL;
        }

        LocalProject* local = StoreFile::getInstance().getProject(id);

        if (std::find(kDisallowedStages.begin(), kDisallowedStages.end(),
                remote->getStage()) != kDisallowedStages.end()) {
            if (local == NULL) {
                if (!testFlag(kForce)) {
                    err("Project " + id + " has status " + stringFromStage(remote->getStage()) + ". If you wish to " 
                            + "install it anyway, run the program with the force-install flag (-f, --force).");
                    return NULL;
                } else {
                    printV("Going forward with installation of project " + id + " despite having stage "
                            + stringFromStage(remote->getStage()) + " due to force-install flag.");
                }
            } else {
                printV("Going forward with installation project " + id + " despite having stage "
                            + stringFromStage(remote->getStage()) + " due to previously installed artifact.");
            }
        }

        if (local == NULL || forceUpdate || remote->getVersion() > local->getVersion()) {
            (*vec)[i] = remote;
            empty = false;
        }

        print("Done resolving " + id + ".");
    }
    curl_global_cleanup();
    return fail ? NULL : (!empty ? vec : kEmptyRppVec);
}

static void printDialogListing(std::vector<std::string> projects) {
    std::string line = "  ";
    for (size_t i = 0; i < projects.size(); i++) {
        if (projects[i] == "") {
            break;
        }
        std::string newLine = line + projects[i];
        if (line.length() + 1 > kInstallDialogLineLength) {
            printQ(line);
            line = "  " + projects[i];
        } else {
            line = newLine + " ";
        }
    }
    printQ(line);
}

int handleInstallCmd(int argc, char* argv[]) {
    if (argc < 3) {
        tooFewArgs(kCmdInstall->getLabel(), kCmdInstall->getUsage());
        return 1;
    }
    return install(params, false);
}

int handleUpgradeCmd(int argc, char* argv[]) {
    if (argc >= 3) {
        tooFewArgs(kCmdUpgrade->getLabel(), kCmdUpgrade->getUsage());
        return 1;
    }
    std::vector<std::string>* projects = StoreFile::getInstance().getProjectIds();
    return install(projects, true);
}

int handleRemoveCmd(int argc, char* argv[]) {
    if (argc < 3) {
        tooFewArgs(kCmdRemove->getLabel(), kCmdRemove->getUsage());
        return 1;
    }
    return remove(params);
}

void printFlag(std::string name, char shorthand, std::string description) {
    std::string start = "  -" + std::string(&shorthand, 1) + ", --" + name;
    std::string indent = "";
    for (int i = start.length(); i < kHelpFlagIndentSize; i++) {
        indent += " ";
    }
    printQ(start + indent + description);
}

int handleHelpCmd(int argc, char* argv[]) {
    if (argc == 2) {
        printInfoHeader();
        printQ("Flags:");
        printFlag("force", 'f', "Forces dbo-get to install packages without considering remote stages.");
        printFlag("quiet", 'q', "Enables quiet logging and suppresses much of the normal output.");
        printFlag("silent", 's', "Disables standard logging, printing only error-level messages.");
        printFlag("update", 'u', "Forces dbo-get to upgrade packages without comparing versions.");
        printFlag("verbose", 'v', "Enables verbose logging.");
        printQ("Commands:");
        for (auto it = std::begin(kCmds); it != std::end(kCmds); ++it) {
            if ((*it)->isDocumented()) {
                printHelp(*it);
            }
        }
        return 0;
    } else if (argc == 3) {
        for (auto it = std::begin(kCmds); it != std::end(kCmds); ++it) {
            if (matchCmd(argv[2], (*it)->getLabel())) {
                if (!(*it)->isDocumented()) {
                    break;
                }
                printHelp(*it);
                return 0;
            }
        }
        err("Cannot display help: invalid command specified.");
        return 1;
    } else {
        tooManyArgs(kCmdHelp->getLabel(), kCmdHelp->getUsage());
        return 1;
    }
}

void printInfoHeader() {
    printQ("dbo-get v" + kVersion + ".");
    printQ("Copyright (c) 2016-2017 Max Roncace.");
    printQ("");
    printQ("dbo-get is a utility for installing and managing projects hosted by BukkitDev,");
    printQ("the premier hosting service for Bukkit software.");
    printQ("");
}

void printHelp(Command* cmd) {
    std::string indent = "";
    for (int i = cmd->getLabel().length(); i < kHelpCmdIndentSize; i++) {
        indent += " ";
    }
    printQ("  " + cmd->getLabel() + indent + cmd->getDescription());
    std::string indent2 = indent;
    for (size_t i = 0; i < cmd->getLabel().length() + 4; i++) {
        indent2 += " ";
    }
    printQ(indent2 + "Usage: dbo-get " + cmd->getLabel() + " " + cmd->getUsage());
}

int handleMooCmd(int argc, char* argv[]) {
    int vs = testFlagi(Flag::kVerbose);
    switch (vs) {
    case 0:
        printQ("Yes, you're very clever.");
        break;
    case 1:
        printQ("Hey, what are you doing?");
        break;
    case 2:
        printQ("Mhm, I get it, haha. Funny.");
        break;
    case 3:
        printQ("You really want this, don't you?");
        break;
    case 4:
        printQ("Alright, very well.");
        break;
    case 5:
        printArt();
        break;
    case 50:
        printQ("Someone's been rummaging through the source, mm?");
        break;
    default:
        printQ("Happy?");
        break;
    }
    return 0;
}

int install(std::vector<std::string>* projects, bool ignoreFail) {
    std::string* loc = Config::getInstance().get(Config::KEY_STORE);
    if (loc == NULL) {
        err("Store location is not set; please run store command first.");
        return 1;
    }
    if (!makePath(*loc)) {
        err("Failed to make store path.");
        return 1;
    }

    std::vector<RemoteProject*>* resolved = resolve(projects, ignoreFail);

    delete(projects);

    if (resolved == NULL) {
        err("No projects specified for installation.");
        return 1;
    }

    if (resolved->empty()) {
        printQ("Already up-to-date.");
        return 0;
    }

    printInstallDialog(resolved);

    printQ("Downloading artifacts...");
    for (size_t i = 0; i < resolved->size(); i++) {
        RemoteProject* proj = (*resolved)[i];
        if (proj == NULL) {
            continue;
        }
        print("Downloading artifact for project " + proj->getId() + ".");
        if (!proj->download()) {
            err("Failed to download artifact for project " + proj->getId() + ".");
            return 1;
        }
        printV("Done downloading " + proj->getId() + ".");
    }
    printQ("Done downloading.");

    printQ("Installing projects...");
    for (size_t i = 0; i < resolved->size(); i++) {
        RemoteProject* proj = (*resolved)[i];
        if (proj == NULL) {
            continue;
        }

        LocalProject* local = StoreFile::getInstance().getProject(proj->getId());
        if (local != NULL) {
            print("Upgrading project " + proj->getId() + " (#" + std::to_string(local->getVersion()) + " -> #" + std::to_string(proj->getVersion()) + ").");
            if (!local->remove() || !proj->install()) {
                //TODO: this shit still ain't atomic
                return 1;
            }
        } else {
            print("Installing project " + proj->getId() + "...");
            if (!proj->download() || !proj->install()) {
                return 1;
            }
        }
        printV("Done installing " + proj->getId() + ".");
    }
    printQ("Done.");

    if (!StoreFile::getInstance().save()) {
        return 1;
    }

    return 0;
}

int remove(std::vector<std::string>* projects) {
    std::string* loc = Config::getInstance().get(Config::KEY_STORE);
    if (loc == NULL) {
        err("Store location is not set; please run store command first.");
        return 1;
    }
    if (!makePath(*loc)) {
        err("Failed to make store path.");
        return 1;
    }

    printQ("Resolving projects...");
    bool fail = false;
    std::vector<LocalProject> resolved = std::vector<LocalProject>(projects->size());
    for (size_t i = 0; i < projects->size(); i++) {
        std::string id = (*projects)[i];
        print("Resolving project " + id + "...");
        LocalProject* proj = StoreFile::getInstance().getProject(id);
        if (proj != NULL) {
            if (!fail) {
                resolved[i] = *proj;
                print("Done resolving " + id + ".");
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

    printQ("Removing projects...");
    for (size_t i = 0; i < resolved.size(); i++) {
        LocalProject proj = resolved[i];
        print("Removing project " + proj.getId() + "...");
        proj.remove();
        print("Done removing " + proj.getId() + ".");
    }
    printQ("Done.");

    if (!StoreFile::getInstance().save()) {
        return 1;
    }

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
        printQ("The following projects will be upgraded:");
        printDialogListing(upgradeList);
    }

    if (ii > 0) {
        printQ("The following projects will be newly installed:");
        printDialogListing(installList);
    }

    printQ(std::to_string(ui) + " upgraded, " + std::to_string(ii) + " newly installed, 0 to remove, "
        + std::to_string(nu) + " not upgraded.");
}

static void printRemoveDialog(std::vector<LocalProject>* projects) {
    std::vector<std::string> removeList = std::vector<std::string>(projects->size());
    for (size_t i = 0; i < projects->size(); i++) {
        removeList[i] = (*projects)[i].getId();
    }
    printDialogListing(removeList);

    printQ("0 upgraded, 0 newly installed, " + std::to_string(projects->size()) + " to remove, 0 not upgraded.");
}
