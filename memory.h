#ifndef MEM_H
#define MEM_H

#include "types.h"

namespace mem
{

inline u64 align8(u64 size)
{
    u64 size_align = (size + sizeof(u64)-1) & ~(sizeof(u64)-1);
    return size_align;
}

void safe_memcpy(void *dst, void *src, u64 number_bytes);

struct Memory
{
    Memory(u64 size);
    ~Memory();
    
    u8 *_base;
    u64 _used;
    u64 _size;
};

};

#endif // MEM_H
