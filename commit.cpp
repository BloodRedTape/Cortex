#include "commit.hpp"
#include "error.hpp"
#include <iomanip>
#include <fstream>
#include <iostream>

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

Hash::Hash(const Commit& commit) {
    hash_combine(Data64[0], (int)commit.Action.Type);
    hash_combine(Data64[0], commit.Action.ModificationTime.Seconds);
    hash_combine(Data64[1], commit.Action.RelativeFilepath);
    hash_combine(Data64[0], commit.Previous.Data64[0]);
    hash_combine(Data64[1], commit.Previous.Data64[1]);
    hash_combine(Data64[1], (int)commit.Action.Type);
}

std::ostream& operator<<(std::ostream& stream, const Hash& hash) {
    if (!hash.Data64[0] && !hash.Data64[1]) {
        stream << "Orphan";
    }else{
        stream << std::hex << hash.Data64[0]; 
        stream << std::hex << hash.Data64[1]; 
    }
    return stream;
}

CommitHistory::CommitHistory(const std::string &save_file_path){
    (void)LoadFrom(save_file_path);
}

void CommitHistory::Add(FileAction action) {
    emplace_back(std::move(action), HashLastCommit());
}

void CommitHistory::Clear() {
    clear();
}

Hash CommitHistory::HashLastCommit(){
	if (!size()) 
		return Hash();

	const Commit &last = back();

	return Hash(last);
}

bool CommitHistory::LoadFrom(const std::string& save_file_path) {
    std::fstream input(save_file_path, std::ios::in | std::ios::binary);
    if(!input)
        return Error("Can't open file % for reading", save_file_path);

    CommitHistory history = Serializer<CommitHistory>::Deserialize(input);

    *this = std::move(history);

    return true;
}

bool CommitHistory::SaveTo(const std::string& save_file_path) {
    std::fstream output(save_file_path, std::ios::out | std::ios::binary);

    if(!output)
        return Error("Can't open file % for writing", save_file_path);
    
    Serializer<CommitHistory>::Serialize(output, *this);

    return true;
}
