#include "flags.h"

#include <algorithm>
#include <map>

static const std::map<char, Flag> FLAGS_SHORTHAND = {{'f', kForce}, {'q', kQuiet}, {'s', kSilent}, {'u', kUpdate}, {'v', kVerbose}};

static const std::map<std::string, Flag> FLAGS_LONGHAND = {{"force", kForce}, {"quiet", kQuiet}, {"silent", kSilent}, {"update", kUpdate}, {"verbose", kVerbose}};

static std::vector<Flag>* flags;

void setFlags(std::vector<Flag>* paramFlags) {
    flags = paramFlags;
}

const Flag* matchFlag(char shorthand) {
    int count = FLAGS_SHORTHAND.count(shorthand);
    return count > 0 ? &FLAGS_SHORTHAND.at(shorthand) : NULL;
}

const Flag* matchFlag(std::string name) {
    return FLAGS_LONGHAND.count(name) > 0 ? &FLAGS_LONGHAND.at(name) : NULL;
}

bool testFlag(Flag flag) {
    const void* loc = flags;
    return std::find(flags->begin(), flags->end(), flag) != flags->end();
}

int testFlagi(Flag flag) {
    return std::count(flags->begin(), flags->end(), flag);
}
