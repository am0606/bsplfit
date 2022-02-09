#include "strtools.h"

//вспомогательная импорт-процедура
// 0 - всё нормально
//-1 - ошибка
int getProperty(const std::string& line, const char* prop_name, int& prop)
{
	std::size_t pos0 = line.find(prop_name);
	if (pos0 == std::string::npos) return -1;
	char delimiters[] = {' ', '\t', '\n'};
	std::size_t pos1 = line.find_first_of(delimiters, pos0, sizeof(delimiters));
	std::size_t pos2 = line.find_first_of(delimiters, pos1+1, sizeof(delimiters));
	std::string substr = std::string(line,pos1,pos2-pos1);
	prop = atoi(substr.data());
	return 0;
}

bool equals(const char* str1, const char* str2)
{
	if (strcmp(str1, str2) == 0) return true;
	return false;
}

bool empty(std::string line)
{
	bool is_empty = (line.empty() || line.size() <= 1);
	return is_empty;
}
