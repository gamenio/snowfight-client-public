#include "StringUtility.h"

#include <cctype>

NS_BEGIN

std::string& StringUtility::ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
	}));
	return s;
}

// trim from end
std::string& StringUtility::rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
	}).base(), s.end());
	return s;
}

// trim from both ends
std::string& StringUtility::trim(std::string& s) {
	return StringUtility::ltrim(StringUtility::rtrim(s));
}

std::vector<std::string>& StringUtility::split(const std::string& s, char delim, std::vector<std::string>& elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string> StringUtility::split(const std::string& s, char delim) {
	std::vector<std::string> elems;
	StringUtility::split(s, delim, elems);
	return elems;
}

bool StringUtility::endsWith(const std::string& str, const std::string& suffix)
{
	return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

bool StringUtility::startsWith(const std::string& str, const std::string& prefix)
{
	return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

std::string StringUtility::substring(std::string const& utf8str, int32 length)
{
	char const* cstr = utf8str.c_str();
	uint32 pos = 0;
	uint32 len = static_cast<uint32>(strlen(cstr));
	uint8 ch;
	int32 count = 0;

	while (pos < len)
	{
		ch = cstr[pos];
		uint32 shift = 0;
		uint8 x = 0x80;
		uint8 b = x;
		while ((b & ch) != 0)
		{
			shift++;
			b = x >> shift;
		}

		// Multi-byte character 
		if (shift > 0)
		{
			count += 2;
			if (count <= length)
				pos += shift;
			else
				break;
		}
		// Single-byte character
		else
		{
			count++;
			if (count <= length)
				pos++;
			else
				break;
		}

	}
	return utf8str.substr(0, pos);
}


NS_END


