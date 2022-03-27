#ifndef CORTEX_COMMIT_HPP
#define CORTEX_COMMIT_HPP

#include <string>
#include <cassert>
#include "time.hpp"
#include "utils.hpp"
#include "filesystem.hpp"
#include "serializer.hpp"

struct Hash {
	union{
		u8  Data[16] = {};
		u8  Data8[16];
		u16 Data16[8];
		u32 Data32[4];
		u64 Data64[2];
	};

	Hash() = default;

	Hash(const struct Commit &commit);

	friend std::ostream &operator<<(std::ostream &stream, const Hash &hash);
};

template<>
struct Serializer<Hash>{
	static void Serialize(std::ostream& stream, const Hash& value) {
		Serializer<u64>::Serialize(stream, value.Data64[0]);
		Serializer<u64>::Serialize(stream, value.Data64[1]);
	}

	static Hash Deserialize(std::istream& stream) {
		Hash result;
		result.Data64[0] = Serializer<u64>::Deserialize(stream);
		result.Data64[1] = Serializer<u64>::Deserialize(stream);
		return result;
	}
};


struct Commit{
	FileAction  Action;
	Hash        Previous;	

	Commit(FileAction action, Hash previous):
		Action(std::move(action)),
		Previous(previous)
	{}
};

template <>
struct Serializer<Commit>{

	static void Serialize(std::ostream& stream, const Commit& value) {
		Serializer<FileAction>::Serialize(stream, value.Action);
		Serializer<Hash>::Serialize(stream, value.Previous);
	}
	static Commit Deserialize(std::istream& stream) {
		FileAction action = Serializer<FileAction>::Deserialize(stream);
		Hash hash = Serializer<Hash>::Deserialize(stream);
		return Commit(action, hash);
	}
};


class CommitHistory : private std::vector<Commit>{
private:
	friend class Serializer<CommitHistory>;
public:
	CommitHistory() = default;

	CommitHistory(CommitHistory &&) = default;

	CommitHistory(const std::string &save_file_path);

	CommitHistory &operator=(CommitHistory &&) = default;

	void Add(FileAction action);

	void Clear();

	Hash HashLastCommit();

	DirState TraceDirState()const;

	bool LoadFrom(const std::string &save_file_path);

	bool SaveTo(const std::string &save_file_path);
	
	auto begin()const { return std::vector<Commit>::begin(); }
	auto end()const { return std::vector<Commit>::end(); }
};

template <>
struct Serializer<CommitHistory>{
	static void Serialize(std::ostream& stream, const CommitHistory& value) {
		Serializer<u32>::Serialize(stream, value.size());
		for(u32 i = 0; i < value.size(); i++)
			Serializer<Commit>::Serialize(stream, value[i]);
	}

	static CommitHistory Deserialize(std::istream& stream) {
		u32 size = Serializer<u32>::Deserialize(stream);
		CommitHistory result;
		result.reserve(size);
		for(u32 i = 0; i<size; i++)
			result.push_back(Serializer<Commit>::Deserialize(stream));
		return result;
	}
};


#endif//CORTEX_COMMIT_HPP