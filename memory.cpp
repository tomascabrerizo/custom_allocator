#include "memory.h"
#include <Windows.h>

namespace mem
{

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
