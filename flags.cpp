#include "flags.h"

#include <map>

static const std::map<char, Flag> FLAGS_SHORTHAND = {{'f', kForce}, {'q', kQuiet}, {'u', kUpdate}, {'v', kVerbose}};
 
static const std::map<std::string, Flag> FLAGS_LONGHAND = {{"force", kForce}, {"quiet", kQuiet}, {"update", kUpdate}, {"verbose", kVerbose}};

const Flag* matchFlag(char shorthand) {
	return FLAGS_SHORTHAND.count(shorthand) > 0 ? &FLAGS_SHORTHAND.at(shorthand) : NULL;
}

const Flag* matchFlag(std::string name) {
	return FLAGS_LONGHAND.count(name) > 0 ? &FLAGS_LONGHAND.at(name) : NULL;
}
