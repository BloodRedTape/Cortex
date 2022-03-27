#include "commit.hpp"
#include "error.hpp"
#include <sstream>

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

CommitHistory::CommitHistory(std::string binary_history){
    std::stringstream stream(binary_history);
    
    *this = std::move(Serializer<CommitHistory>::Deserialize(stream));
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

DirState CommitHistory::TraceDirState()const {
    DirState state;

    for (const Commit& commit : *this) {
        FileState *file_state = state.Find(commit.Action.RelativeFilepath);

        if(!file_state){
            state.push_back(commit.Action);
            continue;
        }
        
        if (file_state->ModificationTime != commit.Action.ModificationTime)
            file_state->ModificationTime = commit.Action.ModificationTime;
    }

    return state;
}

std::string CommitHistory::ToBinary() const{
    std::stringstream stream;

    Serializer<CommitHistory>::Serialize(stream, *this);
    
    return stream.str();
}
