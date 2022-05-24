#ifndef CORTEX_COMMIT_HPP
#define CORTEX_COMMIT_HPP

#include <string>
#include <cassert>
#include "time.hpp"
#include "utils.hpp"
#include "fs/dir_state.hpp"
#include "fs/dir.hpp"
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

	Hash(const u8* data) {
		for (int i = 0; i < 16; i++) 
			Data8[i] = data[i];
	}

	Hash(const struct Commit &commit);

	friend std::ostream &operator<<(std::ostream &stream, const Hash &hash);
	
	bool operator==(const Hash& other)const {
		return Data64[0] == other.Data64[0]
			&& Data64[1] == other.Data64[1];
	}
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

	friend std::ostream& operator<<(std::ostream& stream, const Commit& commit) {
		return stream << '(' << commit.Previous << '|' << FileActionTypeString(commit.Action.Type) << '|' << commit.Action.RelativeFilepath << ')';
	}
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
public:
	static constexpr size_t InvalidIndex = -1;
private:
	friend class Serializer<CommitHistory>;
public:
	CommitHistory() = default;

	CommitHistory(std::string binary_history);

	CommitHistory(CommitHistory &&) = default;

	CommitHistory &operator=(CommitHistory &&) = default;

	void Add(FileAction action);

	void Add(Commit commit);

	void Add(const std::vector<FileAction> &actions);

	void Clear();

	Hash HashLastCommit();

	size_t FindNextCommitIndex(Hash commit_hash)const;

	std::vector<Commit> CollectCommitsAfter(Hash commit_hash);

	std::string ToBinary()const;
	
	using std::vector<Commit>::begin;

	using std::vector<Commit>::end;
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

extern void ApplyActionsToDir(Dir *dir, const std::vector<FileAction> &actions, const std::vector<FileData> &files_data);

extern std::vector<FileData> CollectFilesData(Dir *dir, const FileActionAccumulator& actions);



#endif//CORTEX_COMMIT_HPP