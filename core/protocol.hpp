#pragma once

#include <string>
#include "utils.hpp"

enum class RequestType: u32{
	File,
	History
};

enum class ResponceStatus: u32{
	Success,
	Outdated
};

struct Request {
	RequestType Type;
};

struct Responce {
	ResponceStatus Status;
	std::string Content;
};


constexpr u64 BroadcastMagicWord = 0xBB4400AAAA0044BB;

struct BroadcastDatagram {
	u64 MagicWord = BroadcastMagicWord;
};