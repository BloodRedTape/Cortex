#pragma once

#include <string>
#include <sstream>
#include "utils.hpp"
#include "commit.hpp"
#include "serializer.hpp"

enum class RequestType: u32{
	Pull,
	Push
};

struct PullRequest {
	Hash TopHash;
};

struct PushRequest {
	std::vector<FileCommit> Commits;
};

struct Request {
	const RequestType Type;
	const std::string Content;

	Request(PullRequest request):
		Type(RequestType::Pull),
		Content((char*)&request.TopHash.Data8[0], sizeof(request.TopHash.Data8))
	{}

	Request(PushRequest request):
		Type(RequestType::Push),
		Content(StringSerializer<std::vector<FileCommit>>::Serialize(request.Commits))
	{}

	PullRequest AsPullRequest()const{
		assert(Type == RequestType::Pull);
		assert(Content.size() == sizeof(Hash));

		return {{(const u8*)Content.data()}};
	}

	PushRequest AsPushRequest()const {
		assert(Type == RequestType::Push);

		return {StringSerializer<std::vector<FileCommit>>::Deserialize(Content)};
	}
};

enum class ResponceType: u32{
	Failure,
	Success,
	Diff
};

struct DiffResponce {
	std::vector<FileCommit> Commits;
};

struct SuccessResponce { };

struct FailureResponce { };

struct Responce {
	const ResponceType Type;
	const std::string Content;

	Responce(SuccessResponce):
		Type(ResponceType::Success)
	{}

	Responce(FailureResponce):
		Type(ResponceType::Failure)
	{}

	Responce(DiffResponce diff):
		Type(ResponceType::Diff),
		Content(StringSerializer<std::vector<FileCommit>>::Serialize(diff.Commits))
	{}

	DiffResponce AsDiffResponce()const {
		assert(Type == ResponceType::Diff);

		return {StringSerializer<std::vector<FileCommit>>::Deserialize(Content)};
	}
};

constexpr u64 TransitionMagicWord = 0xCC8800DDDD0088CC;

struct TransitionProtocolHeader {
	u64 MagicWord = TransitionMagicWord;
	u64 TransitionSize = 0;
};

constexpr u64 BroadcastMagicWord = 0xBB4400AAAA0044BB;

struct BroadcastProtocolHeader{
	u64 MagicWord = BroadcastMagicWord;
};