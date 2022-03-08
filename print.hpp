#ifndef CORTEX_PRINT_HPP
#define CORTEX_PRINT_HPP

#include <iostream>

template <typename T, typename ...ArgsType>
void Print(const char *fmt, const T &arg, const ArgsType&...args) {
	while (*fmt) {
		char ch = *fmt++;
		if (ch == '%') {
			std::cout << arg;
			return Print(fmt, args...);
		}
		std::cout << ch;
	}
}

void Print(const char *fmt){
	std::cout << fmt;	
}

template <typename ...ArgsType>
void Println(const char *fmt, const ArgsType&...args){
	Print(fmt, args...);
	Print("\n");
}

#endif//CORTEX_PRINT_HPP