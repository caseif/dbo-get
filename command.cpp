#include "command.h"

Command::Command(std::string label, std::string usage, std::string description,
        std::function<int(int argc, char* argv[])> handler, bool documented) {
    Command::label = label;
    Command::usage = usage;
    Command::description = description;
    Command::handler = handler;
    Command::documented = documented;
}

Command::Command(std::string label, std::string usage, std::string description,
    std::function<int(int argc, char* argv[])> handler) : Command(label, usage, description, handler, true) {}

std::string Command::getLabel() {
    return Command::label;
}

std::string Command::getUsage() {
    return Command::usage;
}

std::string Command::getDescription() {
    return Command::description;
}

std::function<int(int argc, char* argv[])> Command::getHandler() {
    return Command::handler;
}

bool Command::isDocumented() {
    return Command::documented;
}
