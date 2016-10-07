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
        LocalProject* getProject(std::string id);
        std::vector<LocalProject*>* getProjects();
        std::vector<std::string>* getProjectIds();
        void addProject(LocalProject* project);
        void removeProject(std::string id);
};
