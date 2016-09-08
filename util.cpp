#include <iostream>

#include "util.h"

void print(char* str) {
	std::cout << str << std::endl;
}

void err(char* str) {
	std::cerr << "E: " << str << std::endl;
}