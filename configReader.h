#ifndef _TOOLS_H
#define _TOOLS_H

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <string.h>

#define FILE_OPEN_FAILED 1

size_t split(const std::string &s, std::vector<std::string> &tokens, const char delim = ' ');

int str2int(std::string &str);

class Config
{
public:

	Config(std::string &_filePath):filePath(_filePath){};
	Config(const char* _filePath);
	~Config();
    Config &operator=(Config&) = delete;
    Config(Config&) = delete;

	int Load();
	std::string getValue(std::string &Key);
	std::string getValue(const char *Key);
	void Release();

private:

	constexpr static const int MAX_LINE = 256;
	std::string filePath;
	std::map<std::string, std::string> mItems;

};

#endif