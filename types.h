#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float  f32;
typedef double f64;

#define KB(n) (n * 1024LL)
#define MB(n) (n * 1024LL * 1024LL)
#define GB(n) (n * 1024LL * 1024LL * 1024LL)

#endif // TYPES_H
