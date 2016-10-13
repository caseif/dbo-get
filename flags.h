#pragma once

#include <string>

enum Flag {
	kForce,
	kQuiet,
	kUpdate,
	kVerbose
};

const Flag* matchFlag(char shorthand);

const Flag* matchFlag(std::string name);
