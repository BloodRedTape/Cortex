#ifndef CORTEX_IGNORE_LIST_HPP
#define CORTEX_IGNORE_LIST_HPP

#include "file_action.hpp"
#include <vector>
#include <regex>

class IgnoreList {
private:
	std::vector<std::regex> m_Ignorers;
public:
	IgnoreList() = default;

	IgnoreList(std::initializer_list<std::regex> list) {
		m_Ignorers.reserve(list.size());
		for(const auto &regex: list)
			Add(regex);
	}

	IgnoreList(IgnoreList &&) = default;

	IgnoreList &operator=(IgnoreList &&) = default;

	void Add(std::regex regex) {
		m_Ignorers.push_back(std::move(regex));
	}

	bool ShouldBeIgnored(const std::string& item) {
		for (const std::regex& regex : m_Ignorers) {
			if(std::regex_match(item, regex))
				return true;
		}
		return false;
	}
};

#endif//CORTEX_IGNORE_LIST_HPP