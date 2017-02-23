#pragma once

#include <functional>
#include <string>

class Command {
    private:
        std::string label;
        std::string usage;
        std::string description;
        std::function<int(int argc, char* argv[])> handler;
        bool documented;
    public:
        Command(std::string label, std::string usage, std::string description,
                std::function<int(int argc, char* argv[])> handler, bool documented);
        Command(std::string label, std::string usage, std::string description,
                std::function<int(int argc, char* argv[])> handler);
        std::string getLabel();
        std::string getUsage();
        std::string getDescription();
        std::function<int(int argc, char* argv[])> getHandler();
        bool isDocumented();
};
