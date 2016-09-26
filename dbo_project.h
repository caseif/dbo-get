#pragma once

class DboProject {
protected:
        std::string id;
        int numId;
        std::string version;

    public:
        std::string getId();
        int getNumericId();
        std::string getVersion();
};

class RemoteProject : DboProject {
    private:
        bool isResolved = false;
        std::string fileUrl;
        std::string fileName;
        std::string fileMD5;

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
        bool resolve();
        bool install(std::string storeLoc);
};

class LocalProject : DboProject {
    private:
        std::vector<std::string> files;
    public:
        LocalProject();
        LocalProject(std::string id);
        std::vector<std::string> getFiles();
        bool remove(std::string storeLoc);
};
