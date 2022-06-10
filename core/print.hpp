#ifndef CORTEX_PRINT_HPP
#define CORTEX_PRINT_HPP

#include <iostream>
#include <string>
#include <sstream>

inline void StreamPrint(std::ostream &stream, const char *fmt){
	stream << fmt;	
}

template <typename T, typename ...ArgsType>
inline void StreamPrint(std::ostream &stream, const char *fmt, const T &arg, const ArgsType&...args) {
	while (*fmt) {
		char ch = *fmt++;
		if (ch == '%') {
			stream << arg;
			return StreamPrint(stream, fmt, args...);
		}
		stream << ch;
	}
}
template <typename ...ArgsType>
inline void Print(const char* fmt, const ArgsType&...args) {
	StreamPrint(std::cout, fmt, args...);
}

template <typename ...ArgsType>
inline void Println(const char *fmt, const ArgsType&...args){
	Print(fmt, args...);
	Print("\n");
}

template <typename ...ArgsType>
inline std::string Format(const char* fmt, const ArgsType&...args) {
	std::stringstream stream;
	StreamPrint(stream, fmt, args...);
	return stream.str();
}

#endif//CORTEX_PRINT_HPP