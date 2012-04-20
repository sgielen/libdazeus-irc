/*
 * Copyright (C) 2012, Sjors Gielen <dazeus@sjorsgielen.nl>
 * See LICENSE for license.
 */

#include "utils.h"

std::string strToLower(const std::string &f) {
	std::string res;
	res.reserve(f.length());
	for(unsigned i = 0; i < f.length(); ++i) {
		res.push_back(tolower(f[i]));
	}
	return res;
}

std::string strToUpper(const std::string &f) {
	std::string res;
	res.reserve(f.length());
	for(unsigned i = 0; i < f.length(); ++i) {
		res.push_back(toupper(f[i]));
	}
	return res;
}

std::string strToIdentifier(const std::string &f) {
	std::string res;
	res.reserve(f.length());
	for(unsigned i = 0; i < f.length(); ++i) {
		if(!isalpha(f[i])) continue;
		res.push_back(tolower(f[i]));
	}
	return res;
}

std::string trim(const std::string &s) {
	std::string str;
	bool alpha = true;
	for(unsigned i = 0; i < s.length(); ++i) {
		if(alpha && isalpha(s[i]))
			continue;
		alpha = false;
		str += s[i];
	}
	for(int i = str.length() - 1; i >= 0; --i) {
		if(isalpha(str[i]))
			str.resize(i);
		else break;
	}
	return str;
}

bool contains(std::string x, char v) {
	return find(x.begin(), x.end(), v) != x.end();
}

bool startsWith(std::string x, std::string y, bool caseInsensitive)
{
	std::string z = x.substr(0, y.length());
	if(caseInsensitive)
		return strToLower(z) == strToLower(y);
	else	return z == y;
}

std::vector<std::string> split(const std::string &s, const std::string &sep)
{
	std::vector<std::string> res;
	std::string s_ = s;
	int len = sep.length();
	int remaining = s.length();
	for(int i = 0; i <= remaining - len; ++i) {
		if(s_.substr(i, len) == sep) {
			res.push_back(s_.substr(0, i));
			s_ = s_.substr(i + sep.length());
			remaining -= i + sep.length();
			i = -1;
		}
	}
	res.push_back(s_);
	return res;
}

std::vector<std::string> &operator<<(std::vector<std::string> &x, const char *v) {
	x << std::string(v);
	return x;
}


