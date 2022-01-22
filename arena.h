#ifndef ARENA_H
#define ARENA_H

#include "memory.h"

namespace mem
{

class Arena
{
public:
    Arena(Memory *mem, u64 size);
    ~Arena() = default;
    u8 *push_size(u64 size);
    void free_size(u64 size);

    u64 get_used();
    
protected:
    u64 _used;
    u8 *_base;
    u64 _size;

    void push_offset(s64 offset);
};

};

#endif // ARENA_H
