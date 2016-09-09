#pragma once

class DboProject {
    private:
        bool isResolved = false;
        std::string id;
        int numId;
        std::string latestVer;
        std::string alternatives;
    public:
        DboProject();
        DboProject(std::string id);
        std::string getId();
        int getNumericId();
        std::string getLatestVersion();
        std::string getAlternatives();
        bool resolve();
        bool install();
        bool remove();
};
