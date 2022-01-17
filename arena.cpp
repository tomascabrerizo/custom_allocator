#include "arena.h"
#include <assert.h>

namespace mem
{

Arena::Arena(Memory *mem, u64 size)
{
    u64 size_a = align8(size);
    assert(mem->_used + size_a <= mem->_size);
    _size = size_a;
    _base = mem->_base + mem->_used;
    _used = 0;
    mem->_used += size_a;
}

u8 *Arena::push_size(u64 size)
{
    assert(_used + size <= _size);
    u8 *result = _base + _used;
    _used += size;
    return result;
}

void Arena::free_size(u64 size)
{
    assert(_used >= size);
    _used -= size;
}

};
