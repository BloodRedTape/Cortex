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

void CommitHistory::Add(Commit commit) {
    push_back(std::move(commit));
}

void CommitHistory::Add(const std::vector<FileAction>& actions){
    for(const auto &action: actions)
        Add(action);
}

void CommitHistory::Add(const std::vector<Commit>& commits){
    for(const Commit &commit: commits){
        CX_ASSERT(commit.Previous == HashLastCommit());
        Add(commit);
    }
}

void CommitHistory::Clear() {
    clear();
}

DirState CommitHistory::TraceDirState() const{
    DirState state;

    for (const Commit& commit : *this) {
        switch (commit.Action.Type) {
        case FileActionType::Write: {
            FileMeta *file_state = state.Find(commit.Action.RelativeFilepath);
            if(!file_state)
                state.emplace_back(commit.Action.RelativeFilepath, commit.Action.Time);
            else
                file_state->ModificationTime = commit.Action.Time;
        }break;
        case FileActionType::Delete: {
            FileMeta *file_state = state.Find(commit.Action.RelativeFilepath);
            CX_ASSERT(file_state);
            state.Remove(file_state);
        }break;
        default:
            CX_ASSERT(false);
        }
    }

    return state;
}

Hash CommitHistory::HashLastCommit(){
	if (!size()) 
		return Hash();

	const Commit &last = back();

	return Hash(last);
}

size_t CommitHistory::FindNextCommitIndex(Hash commit_hash) const{
    for (size_t i = 0; i < size(); i++) {
        if(operator[](i).Previous == commit_hash)
            return i;
    }
    return InvalidIndex;
}

std::vector<Commit> CommitHistory::CollectCommitsAfter(Hash commit_hash){
    size_t index = FindNextCommitIndex(commit_hash);
    
    std::vector<Commit> result;

    for (size_t i = index; i < size(); i++) 
        result.push_back(operator[](i));

    return result;
}

std::string CommitHistory::ToBinary() const{
    std::stringstream stream;

    Serializer<CommitHistory>::Serialize(stream, *this);
    
    return stream.str();
}

void ApplyCommits(Dir* dir, CommitHistory& history, const std::vector<Commit>& commits, const std::vector<FileData>& files_data) {
    //XXX: This shit should not copy entire thing
    //ApplyCommits is called on the client, but ApplyActions hashes actions to make them commits,
    //Hashing should be done on the server
    std::vector<FileAction> actions;
    actions.reserve(commits.size());
    for (auto commit : commits)
        actions.push_back(commit.Action);
    
    ApplyActions(dir, history, actions, files_data);
}

void ApplyActions(Dir *dir, CommitHistory &history, const std::vector<FileAction> &actions, const std::vector<FileData> &files_data){
    DirState state = history.TraceDirState();

    for (const FileData& file : files_data)
        dir->WriteEntireFile(file.RelativeFilepath, file.Content);

    for (const FileAction &action: actions){
        if(action.Type == FileActionType::Delete) 
            //XXX: Handle failure
            dir->DeleteFile(action.RelativeFilepath);
        if (action.Type == FileActionType::Write) {
            if (state.Has(action.RelativeFilepath)) {
                dir->SetFileTime(action.RelativeFilepath, {dir->GetFileTime(action.RelativeFilepath)->Created, action.Time});
            } else {
                dir->SetFileTime(action.RelativeFilepath, {action.Time, action.Time});
            }
        }
    }

    history.Add(actions);
}

std::vector<FileData> CollectFilesData(Dir *dir, const FileActionAccumulator& actions){
	std::vector<FileData> result;
		
	for (const auto &action : actions) {
		//XXX: Validate
        if(action.Type == FileActionType::Write)
		    result.push_back(
			    FileData{
				    action.RelativeFilepath, 
				    dir->ReadEntireFile(action.RelativeFilepath).second
			    }
		    );
	}

	return result;
}
