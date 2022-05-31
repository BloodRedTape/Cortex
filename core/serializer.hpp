#ifndef CORTEX_SERIALIZER_HPP
#define CORTEX_SERIALIZER_HPP

#include <ostream>
#include <istream>
#include <string>
#include <sstream>
#include <vector>
#include "utils.hpp"

template <typename T>
struct Serializer{
	static void Serialize(std::ostream& stream, const T& value) {
		stream.write((const char*)&value, sizeof(value));
	}
	static T Deserialize(std::istream& stream) {
		T value{};
		stream.read((char*)&value, sizeof(value));
		return value;
	}
};

template <typename T>
struct StringSerializer {
	static std::string Serialize(const T& value) {
		std::stringstream stream;
		Serializer<T>::Serialize(stream, value);
		return stream.str();
	}

	static T Deserialize(const std::string& string) {
        std::stringstream stream(string);
		return Serializer<T>::Deserialize(stream);
	}
};

template <>
struct Serializer<std::string>{
	static void Serialize(std::ostream& stream, const std::string& value) {
		Serializer<u32>::Serialize(stream, value.size());
		for(u32 i = 0; i < value.size(); i++)
			Serializer<char>::Serialize(stream, value[i]);
	}

	static std::string Deserialize(std::istream& stream) {
		u32 size = Serializer<u32>::Deserialize(stream);
		std::string result;
		result.resize(size);
		for(u32 i = 0; i<size; i++)
			result[i] = Serializer<char>::Deserialize(stream);
		return result;
	}
};

template <typename T>
struct Serializer<std::vector<T>>{
	static void Serialize(std::ostream& stream, const std::vector<T>& value) {
		Serializer<u32>::Serialize(stream, value.size());
		for(u32 i = 0; i < value.size(); i++)
			Serializer<T>::Serialize(stream, value[i]);
	}

	static std::vector<T> Deserialize(std::istream& stream) {
		u32 size = Serializer<u32>::Deserialize(stream);
		std::vector<T> result;
		result.reserve(size);
		for(u32 i = 0; i<size; i++)
			result.push_back(Serializer<T>::Deserialize(stream));
		return result;
	}
};

#endif//CORTEX_SERIALIZER_HPP