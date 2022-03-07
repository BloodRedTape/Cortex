#include "commit.hpp"
#include <iomanip>

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
    stream << std::hex << hash.Data64[0]; 
    stream << std::hex << hash.Data64[1]; 
    return stream;
}