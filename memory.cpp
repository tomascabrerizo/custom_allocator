#include "memory.h"
#include <Windows.h>

namespace mem
{

void safe_memcpy(void *dst, void *src, u64 number_bytes)
{
    if(dst < src)
    {
        for(u64 index = 0; index < number_bytes; ++index)
        {
            u8 *dst_ptr = (u8 *)dst + index;
            u8 *src_ptr = (u8 *)src + index;
            *dst_ptr = *src_ptr;
        }
    }
    else if(dst > src)
    {
        for(u64 index = 0; index < number_bytes; ++index)
        {
            u8 *dst_ptr = ((u8 *)dst + (number_bytes - 1)) - index;
            u8 *src_ptr = ((u8 *)src + (number_bytes - 1)) - index;
            *dst_ptr = *src_ptr;
        }
    }
}

Memory::Memory(u64 size)
{
    _size = align8(size);
    _base = (u8 *)VirtualAlloc(0, _size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    _used = 0;
}

Memory::~Memory()
{
    VirtualFree(_base, 0, MEM_RELEASE);
}

};
