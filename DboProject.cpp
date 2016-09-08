#include <assert.h>
#include <string>

#include "DboProject.h"
#include "util.h"

DboProject::DboProject() {
    DboProject::id = "";
}

DboProject::DboProject(std::string id) {
    DboProject::id = id;
}

std::string DboProject::getId() {
    assert(DboProject::isResolved);
    return DboProject::id;
}

int DboProject::getNumericId() {
    assert(DboProject::isResolved);
    return DboProject::numId;
}

std::string DboProject::getLatestVersion() {
    assert(DboProject::isResolved);
    return DboProject::latestVer;
}

std::string DboProject::getAlternatives() {
    assert(!DboProject::isResolved);
    return DboProject::alternatives;
}

bool DboProject::resolve() {
    print("Resolving project " + getId() + "...");
    return true; //TODO
}

bool DboProject::install() {
    print("Installing project " + getId() + "...");
    return true; //TODO
}

bool DboProject::remove() {
    return false; //TODO
}
