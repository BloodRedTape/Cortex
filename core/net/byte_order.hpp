#ifndef CORTEX_BYTE_ORDER_HPP
#define CORTEX_BYTE_ORDER_HPP

#include "utils.hpp"

constexpr bool IsLittleEndian() {
    constexpr u16 U16 = 0x0102;
    constexpr u8 U8 = (const u8 &)U16;
    return U8 == 0x02;
}

static_assert(IsLittleEndian(), "CortexCore assumes little endian machine");

inline u8 SwapEndianess(u8 value) {
    return value;
}
inline u16 SwapEndianess(u16 value) {
    return (value >> 8) | (value << 8);
}

inline u32 SwapEndianess(std::uint32_t &value) {
    u32 tmp = ((value << 8) & 0xFF00FF00) | ((value >> 8) & 0xFF00FF);
    return (tmp << 16) | (tmp >> 16);
}

inline u64 SwapEndianess(u64 value) {
    return ((value & 0x00000000FFFFFFFFull) << 32) | ((value & 0xFFFFFFFF00000000ull) >> 32)
        |  ((value & 0x0000FFFF0000FFFFull) << 16) | ((value & 0xFFFF0000FFFF0000ull) >> 16)
        |  ((value & 0x00FF00FF00FF00FFull) << 8)  | ((value & 0xFF00FF00FF00FF00ull) >> 8 );
}

//Assumes all targets are little endian

inline u8 ToNetByteOrder(u8 value) {
    return SwapEndianess(value);
}
inline u16 ToNetByteOrder(u16 value) {
    return SwapEndianess(value);
}
inline u32 ToNetByteOrder(u32 value) {
    return SwapEndianess(value);
}
inline u64 ToNetByteOrder(u64 value) {
    return SwapEndianess(value);
}

inline u8 ToHostByteOrder(u8 value) {
    return SwapEndianess(value);
}
inline u16 ToHostByteOrder(u16 value) {
    return SwapEndianess(value);
}
inline u32 ToHostByteOrder(u32 value) {
    return SwapEndianess(value);
}
inline u64 ToHostByteOrder(u64 value) {
    return SwapEndianess(value);
}

#endif//CORTEX_BYTE_ORDER_HPP