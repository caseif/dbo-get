#include <iostream>
#include <string>
#include <sstream>
#include <vector>

static inline void print(std::string str) {
    std::cout << str.c_str() << std::endl;
}

static inline void err(std::string str) {
    std::cout << "E: " << str.c_str() << std::endl;
}

static inline void tooFewArgs(std::string usage) {
    err("Too few args!");
    std::string str(usage);
    print("    Usage: dbo_get " + str + " <projects>...");
}

// courtesy of http://stackoverflow.com/a/36401787/1988755
static size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s) {
	size_t newLength = size*nmemb;
	size_t oldLength = s->size();
	try {
		s->resize(oldLength + newLength);
	} catch (std::bad_alloc &e) {
		//handle memory problem
		return 0;
	}

	std::copy((char*)contents, (char*)contents + newLength, s->begin() + oldLength);
	return size*nmemb;
}
