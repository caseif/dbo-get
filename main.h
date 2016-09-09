#pragma once

#include <vector>
#include "DboProject.h"

std::vector<DboProject>* resolve(int argc, char* argv[]);

int install(int argc, char* argv[]);
