#pragma once

#include <string>
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
	std::string Content;

	Request(PullRequest request):
		Type(RequestType::Pull),
		Content((char*)&request.TopHash.Data8[0], sizeof(request.TopHash.Data8))
	{}

	Request(PushRequest request):
		Type(RequestType::Push)
	{
		std::stringstream stream;
		Serializer<std::vector<FileCommit>>::Serialize(stream, request.Commits);
		Content = stream.str();
	}

	PullRequest AsPullRequest()const{
		assert(Type == RequestType::Pull);
		assert(Content.size() == sizeof(Hash));

		return {{(const u8*)Content.data()}};
	}

	PushRequest AsPushRequest()const {
		assert(Type == RequestType::Push);

		return {Serializer<std::vector<FileCommit>>::Deserialize(std::stringstream(Content))};
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
	std::string Content;

	Responce(SuccessResponce):
		Type(ResponceType::Success)
	{}

	Responce(FailureResponce):
		Type(ResponceType::Failure)
	{}

	Responce(DiffResponce diff):
		Type(ResponceType::Diff)
	{
		std::stringstream stream;
		Serializer<std::vector<FileCommit>>::Serialize(stream, diff.Commits);
		Content = stream.str();
	}

	DiffResponce AsDiffResponce()const {
		assert(Type == ResponceType::Diff);

		return {Serializer<std::vector<FileCommit>>::Deserialize(std::stringstream(Content))};
	}
};


constexpr u64 BroadcastMagicWord = 0xBB4400AAAA0044BB;

struct BroadcastDatagram {
	u64 MagicWord = BroadcastMagicWord;
};