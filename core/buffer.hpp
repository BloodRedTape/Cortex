#ifndef CORTEX_STREAM_HPP
#define CORTEX_STREAM_HPP

#include "utils.hpp"

class OutputDevice {
public:
	virtual void Write(const void *data, size_t size) = 0;
};

class InputDevice {
public:
	virtual size_t Read(void *data, size_t size) = 0;

	virtual size_t Size()const = 0;
};

class BufferedInputStream: public InputStream{
public:

	size_t Read(void *data, size_t size) = 0;

	size_t Size()const = 0;
};

#endif//CORTEX_STREAM_HPP