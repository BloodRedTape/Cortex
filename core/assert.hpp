#pragma once

#include "error.hpp"

#define CX_ASSERT(expr) ((expr) ? (void)0 : (Error("Assertion failed\nExpr: %\nFile: %\nLine: %", #expr, __FILE__, __LINE__), debugbreak()))