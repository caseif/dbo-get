#include <assert.h>
#include <string>

#include "DboProject.h"

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
    return false; //TODO
}

bool DboProject::install() {
    return false; //TODO
}

bool DboProject::remove() {
    return false; //TODO
}
