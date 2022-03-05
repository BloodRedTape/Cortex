#ifndef CORTEX_COMMIT_HPP
#define CORTEX_COMMIT_HPP

#include <string>
#include <cassert>
#include <ostream>
#include "time.hpp"
#include "utils.hpp"
#include "filesystem.hpp"

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


struct Commit{
	FileAction  Action;
	Hash        Previous;	

	Commit(FileAction action, Hash previous):
		Action(std::move(action)),
		Previous(previous)
	{}
};


#endif//CORTEX_COMMIT_HPP