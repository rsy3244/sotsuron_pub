#pragma once

#include <cstddef>
#include <cstdint>

//SIMD命令を使うときはdefineする
//#define ALLOW_SIMD_INSTRUCTION

using Data = std::uintmax_t;
constexpr std::size_t SIGMA = 128; //alphabet size;
constexpr std::size_t WORDSIZE = sizeof(Data) * 8;
constexpr std::size_t LOG2_WORDSIZE = 6;
#ifdef ALLOW_SIMD_INSTRUCTION
constexpr std::size_t QWORDSIZE = 4*WORDSIZE;
#endif


