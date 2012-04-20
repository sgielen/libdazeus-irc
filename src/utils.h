/*
 * Copyright (c) Sjors Gielen, 2010-2012
 * See LICENSE for license.
 */

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>

std::string strToLower(const std::string &f);
std::string strToUpper(const std::string &f);
std::string strToIdentifier(const std::string &f);
std::string trim(const std::string &s);
std::vector<std::string> split(const std::string &s, const std::string &sep);
bool contains(std::string x, char v);
// does X start with Y?
bool startsWith(std::string x, std::string y, bool caseInsensitive);

template <typename Container, typename Key>
bool contains(Container x, Key k) {
	return x.count(k) != 0;
}

template <typename Value>
bool contains(std::vector<Value> x, Value v) {
	return find(x.begin(), x.end(), v) != x.end();
}

template <typename Container, typename Value>
void erase(Container &x, Value v) {
	x.erase(remove(x.begin(), x.end(), v), x.end());
}

template <typename Container, typename Value>
Value takeFirst(Container c) {
	Value v = c[0];
	c.erase(c.begin());
	return v;
}

template <typename Value>
typename std::map<std::string,Value>::const_iterator find_ci(const std::map<std::string,Value> &m, std::string s) {
	std::string sl = strToLower(s);
	typename std::map<std::string,Value>::const_iterator it;
	for(it = m.begin(); it != m.end(); ++it) {
		if(strToLower(it->first) == sl) {
			return it;
		}
	}
	return m.end();
}


template <typename Container>
std::string join(Container c, std::string s) {
	std::stringstream ss;
	typename Container::const_iterator it;
	bool first = true;
	for(it = c.begin(); it != c.end(); ++it) {
		if(!first)
			ss << s;
		ss << *it;
		first = false;
	}
	return ss.str();
}

std::vector<std::string> &operator<<(std::vector<std::string> &x, const char *v);
template <typename T>
std::vector<T> &operator<<(std::vector<T> &x, T v) {
	x.push_back(v);
	return x;
}

template <typename T>
std::vector<T> &operator<<(std::vector<T> &x, const std::vector<T> &v) {
	x.insert(x.end(), v.begin(), v.end());
	return x;
}

#endif
