#pragma once

class DboProject {
    private:
        bool isResolved = false;
        std::string id;
        int numId;
        std::string latestVer;
		std::string fileUrl;
		std::string fileName;
		std::string fileMD5;

		bool parseId(std::string json);
		bool populateFields(std::string json);
		bool doLookup();
		bool doQuery();
    public:
        DboProject();
        DboProject(std::string id);
        std::string getId();
        int getNumericId();
        std::string getLatestVersion();
		std::string getFileUrl();
		std::string getFileName();
		std::string getFileMD5();
        bool resolve();
        bool install();
        bool remove();
};
