#include "StrManip.h"

// Trim start of string
std::string ltrim(const std::string& s) {
	const std::string WHITESPACE = " \n\r\t\f\v";
	size_t start = s.find_first_not_of(WHITESPACE);
	return (start == std::string::npos) ? "" : s.substr(start);
}

// Trim end of string
std::string rtrim(const std::string& s) {
	const std::string WHITESPACE = " \n\r\t\f\v";
	size_t end = s.find_last_not_of(WHITESPACE);
	return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

// Trim string
std::string trim(const std::string& s) {
	return rtrim(ltrim(s));
}