#pragma once

#include <string>
#include <vector>

enum Flag {
    kForce,
    kQuiet,
    kSilent,
    kUpdate,
    kVerbose
};

void setFlags(std::vector<Flag>* paramFlags);

const Flag* matchFlag(char shorthand);

const Flag* matchFlag(std::string name);

bool testFlag(Flag flag);

int testFlagi(Flag flag);
