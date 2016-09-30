#pragma once

#include <map>
#include <string>

#include "dbo_project.h"

class StoreFile {
    private:
        bool initialized;
        std::map<std::string, LocalProject> projects;

        std::string getPath();
    public:
        StoreFile();
        StoreFile(StoreFile const&) = delete;
        void operator = (StoreFile const&) = delete;
        static StoreFile& getInstance();

        void load();
        void save();
        bool hasProject(std::string id);
        LocalProject* getProject(std::string id);
        void addProject(LocalProject* project);
        void removeProject(std::string id);
};
