#include "configReader.h"

size_t split(const std::string& s, std::vector<std::string>& tokens, const char delim) 
{
    tokens.clear();
    auto string_find_first_not = [s, delim](size_t pos = 0) -> size_t {
        for (size_t i = pos; i < s.size(); i++) {
            if (s[i] != delim) return i;
        }
        return std::string::npos;
    };   
    size_t lastPos  = string_find_first_not(0);
    size_t pos      = s.find(delim, lastPos);
    while (lastPos != std::string::npos) {
        tokens.emplace_back(s.substr(lastPos, pos - lastPos));
        lastPos     = string_find_first_not(pos);
        pos         = s.find(delim, lastPos);
    }
    return tokens.size();
}

const std::string trim(const std::string& s) {
	auto isntspace = [](const char& ch) {
        return !isspace(ch);
	};

    std::string::const_iterator iter1 = std::find_if(s.begin(), s.end(), isntspace);
    std::string::const_iterator iter2 = std::find_if(s.rbegin(), s.rend(), isntspace).base();

    return iter1 < iter2 ? std::string(iter1, iter2) : std::string("");
}

int str2int(std::string &str)
{
	return strtol(str.c_str(), nullptr, 10);
}

Config::Config(const char *_filePath)
{
	filePath = std::string(_filePath);
}

Config::~Config()
{
	Release();
}

int Config::Load()
{
	std::ifstream fConfig(filePath);
	if (! fConfig.is_open())
	{ 
		std::cout << "Error opening file" << std::endl; 
		return ERR_FILE_OPEN_FAILED; 
	}

	char buffer[MAX_LINE];
	while (!fConfig.eof() )
	{
		memset(buffer, 0, sizeof(buffer));
		fConfig.getline(buffer,100);
		std::string line(buffer);

		std::vector<std::string> tokens;
		int cnt = split(line, tokens , '=');
		if(cnt != 2 )
		{
			continue;
		}
		mItems.insert(std::pair<std::string, std::string>(trim(tokens[0]),trim(tokens[1])) );
	}

	return 0;
}

std::string Config::getValue(std::string &Key)
{
	std::string ans;
	std::map<std::string, std::string>::iterator it = mItems.find(Key);
    if (it != mItems.end())
    {
        ans = it->second;
    }
	return ans;
}

std::string Config::getValue(const char *Key)
{
	std::string strKey;
	std::string ans;
	std::map<std::string, std::string>::iterator it = mItems.find(strKey);
    if (it != mItems.end())
    {
        ans = it->second;
    }
	return ans;
}

void Config::Release()
{

}