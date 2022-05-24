#ifndef CORTEX_TIME_HPP
#define CORTEX_TIME_HPP

#include "utils.hpp"
#include "serializer.hpp"

struct UnixTime {
	u64 Seconds = 0;

	bool operator==(UnixTime other)const{
		return Seconds==other.Seconds;
	}
	bool operator!=(UnixTime other)const{
		return !(*this == other);
	}

	static UnixTime CurrentTime();
};

template <>
struct Serializer<UnixTime>{
	static void Serialize(std::ostream& stream, const UnixTime& value) {
		Serializer<u64>::Serialize(stream, value.Seconds);
	}
	static UnixTime Deserialize(std::istream& stream) {
		UnixTime value;
		value.Seconds = Serializer<u64>::Deserialize(stream);
		return value;
	}
};

#endif//CORTEX_TIME_HPP