#ifndef CORTEX_ERROR_HPP
#define CORTEX_ERROR_HPP

#include "print.hpp"

template <typename ...ArgsType>
bool Error(const char *fmt, const ArgsType &...args) {
	Print("[Error]: ");
	Println(fmt, args...);
	return false;
}

template <typename ...ArgsType>
void Info(const char *fmt, const ArgsType &...args) {
	Print("[Info]: ");
	Println(fmt, args...);
}

template <typename ...ArgsType>
void Warning(const char *fmt, const ArgsType &...args) {
	Print("[Warning]: ");
	Println(fmt, args...);
}

#endif//CORTEX_ERROR_HPP