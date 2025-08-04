#ifndef __STRING_UTILITY_H__
#define __STRING_UTILITY_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

class StringUtility
{
public:
	// Trimming functions
	static std::string& ltrim(std::string& s);
	static std::string& rtrim(std::string& s);
	static std::string& trim(std::string& s);

	// Split
	static std::vector<std::string>& split(const std::string& s, char delim, std::vector<std::string>& elems);
	static std::vector<std::string> split(const std::string& s, char delim);

	static bool endsWith(const std::string& str, const std::string& suffix);
	static bool startsWith(const std::string& str, const std::string& prefix);

	// Get a substring of specified length from a UTF-8 string.
	static std::string substring(std::string const& utf8str, int32 length);
};


NS_END

#endif // __STRING_UTILITY_H__