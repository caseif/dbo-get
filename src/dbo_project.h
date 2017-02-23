#pragma once

#include "stage.h"

class DboProject {
protected:
        std::string id;
        int numId;
        int version;

    public:
        std::string getId();
        int getNumericId();
        int getVersion();
};

class RemoteProject : public DboProject {
    private:
        bool isResolved = false;
        std::string fileUrl;
        std::string fileName;
        std::string fileMD5;
        Stage stage;

        std::vector<std::string>* installFiles();
        int parseVersion(std::string url);

    public:
        RemoteProject();
        RemoteProject(std::string id);
        bool parseId(std::string json);
        bool populateFields(std::string json);
        bool doLookup();
        bool doQuery();
        std::string getFileUrl();
        std::string getFileName();
        std::string getFileMD5();
        Stage getStage();
        bool resolve();
        bool download();
        bool install();
};

class LocalProject : public DboProject {
    private:
        std::vector<std::string> files;
    public:
        LocalProject();
        LocalProject(std::string id, int numId, int version, std::vector<std::string>* files);
        std::vector<std::string> getFiles();
        bool remove();
};
