#include "heap.h"
#include <stdio.h>

namespace mem
{

///////////////////////////////////////////////////////
//      Block methods:
//
///////////////////////////////////////////////////////

void Block::set_used(bool used)
{
    used == false ? _size |= 0x1 : _size &= ~0x1;
}

bool Block::is_used()
{
    return !(_size & 0x1);
}

void Block::set_size(u64 size)
{
    if(!is_used())
    {
        _size = size;
        set_used(false);
    }
    else
    {
        _size = size;
    }
}

u8 *Block::get_data()
{
    return (u8 *)this + sizeof(Block);
}

u64 Block::get_size()
{
    return _size & ~0x1;
}


///////////////////////////////////////////////////////
//      Inline Heap functions:
//
///////////////////////////////////////////////////////

inline bool next_block_have_size(Block *block, u64 size)
{
    if(block->_next)
    {
        return (block->get_size() + block->_next->get_size()) >= size;
    }
    return false;
}

inline bool prev_block_have_size(Block *block, u64 size)
{
    if(block->_prev)
    {
        return (block->get_size() + block->_prev->get_size()) >= size;
    }
    return false;
}

///////////////////////////////////////////////////////
//      Inline Heap methods:
//      Public interface
///////////////////////////////////////////////////////

/*@docs------------------------------------------------
[FNC]:  - Heap(Memory *mem, u64 size)
[IN ]:
        - mem (Memory *): pointer to a memory object
        - size (u64): total size of the heap in bytes
[OUT]:
        - heap (Heap): new Heap object
-----------------------------------------------------*/
Heap::Heap(Memory *mem, u64 size) :
    Arena(mem, size)
{
    _top = 0;
    _freelist = 0;
}

/*@docs------------------------------------------------
[FNC]:  - Heap::allocate(u64 size)
[IN ]:
        - size (u64): number of bytes to allocate 
[OUT]:
        - data (u8 *): pointer to the new allocated data 
-----------------------------------------------------*/
u8 *Heap::allocate(u64 size)
{
    size = align8(size);
    
    Block *block = get_best_fit_from_freelist(size);
    if(block) 
    {
        try_to_split_block(block, size); 
        remove_block_from_freelist(block);
        block->set_used(true);
        return block->get_data();
    }
    
    block = (Block *)push_size(sizeof(Block) + size);
    add_block(block, size);
    
    return block->get_data();
}

/*@docs------------------------------------------------
[FNC]:  - Heap::deallocate(u8 *base)
[IN ]:
        - base (u8 *): already allocated pointer to be free 
-----------------------------------------------------*/
void Heap::deallocate(u8 *base)
{
    Block *block = get_block_from_data(base);
    add_block_to_freelist(block);
    block = try_to_merge_block(block);
    block->set_used(false);
}

/*@docs------------------------------------------------
[FNC]:  - Heap::reallocate(u64 size)
[IN ]:
        - data (u8 *): pointer to the memory buffer to be reallocated 
        - size (u64): size of the new memory buffer 
[OUT]:
        - data (u8 *): pointer to the new allocated data 
-----------------------------------------------------*/
u8 *Heap::reallocate(u8 *data, u64 size)
{
    Block *block = get_block_from_data(data);
    u64 block_size = block->get_size();
    if(block_size == size) return data;
    
    if(last_allocated_block(block))
    {
        resize_block(block, size);
        return data;
    }
    else if(size > block_size)
    {
        if(next_block_have_size(block, size))
        {
            try_to_merge_block_right(block);
            return data;
        }
        else if(prev_block_have_size(block, size))
        {
            Block *new_block = try_to_merge_block_left(block);
            safe_memcpy(new_block->get_data(), block->get_data(), block->get_size());
            return new_block->get_data();
        }
    }
    return slow_realloc(block, size);
}


///////////////////////////////////////////////////////
//      Inline Heap methods:
//      Private 
///////////////////////////////////////////////////////

// (Heap) functions.

u8 *Heap::slow_realloc(Block *block, u64 size)
{
    u8 *new_data = allocate(size);
    safe_memcpy(new_data, block->get_data(), block->get_size());
    deallocate(block->get_data());
    return new_data;
}

// (Heap - Block) functions.

void Heap::add_block(Block *block, u64 size)
{
    block->set_size(size);
    block->set_used(true);
    block->_next = 0;
    block->_prev = 0;
    block->_next_free = 0;
    block->_prev_free = 0;
    
    if(_top)
    {
        _top->_next = block;
        block->_prev = _top;
    }
    _top = block;
}

void Heap::insert_block_after(Block *block, Block *other_block)
{
    other_block->_next = block->_next;
    other_block->_prev = block;
    block->_next = other_block;
    if(block == _top) _top = other_block;
}

void Heap::remove_block(Block *block)
{
    if(!block->_next)
    {
        _top = block->_prev; 
        _top->_next = 0;
    }
    else
    {
        Block *prev = block->_prev; 
        Block *next = block->_next;
        prev->_next = next;
        next->_prev = prev;
    }
    block->_prev = 0;
    block->_next = 0;
}

Block *Heap::get_block_from_data(u8 *data)
{
    Block *block = (Block *)(data - sizeof(Block));
    return block;
}

void Heap::resize_block(Block *block, u64 size)
{
    s64 size_diff = (s64)size - (s64)block->get_size();
    push_offset(size_diff);
    block->set_size(size);
}

bool Heap::last_allocated_block(Block *block)
{
    return _top == block;
}

Block *Heap::try_to_merge_block(Block *block)
{
    try_to_merge_block_right(block);
    block = try_to_merge_block_left(block);
    return block;
}

void Heap::try_to_merge_block_right(Block *block)
{
    Block *right_block = block->_next;
    if(right_block && !right_block->is_used())
    {
        u64 total_size = right_block->get_size() + sizeof(Block);
        block->set_size(block->get_size() + total_size);
        remove_block_from_freelist(right_block);
        remove_block(right_block);
    }
}

Block *Heap::try_to_merge_block_left(Block *block)
{
    Block *left_block = block->_prev;
    if(left_block && !left_block->is_used())
    {
        u64 total_size = block->get_size() + sizeof(Block);
        left_block->set_size(left_block->get_size() + total_size);
        remove_block_from_freelist(block);
        remove_block(block);
        block = left_block;
    }
    return block;
}

void Heap::try_to_split_block(Block *block, u64 size)
{
    u64 block_size = block->get_size();
    u64 new_block_size = block_size - size;
    if(new_block_size >= BLOCK_MIN_SIZE)
    {
        block->set_size(size);
        split_block(block, size, new_block_size);
    }
}

void Heap::split_block(Block *block, u64 size, u64 new_block_size)
{
    Block *new_block = (Block *)(block->get_data() + size);
    new_block->set_used(false);
    new_block->set_size(new_block_size - sizeof(Block));
    insert_block_after(block, new_block);
    insert_block_after_freelist(block, new_block);
}

// (Heap - Freelist) functions

Block *Heap::get_first_fit_from_freelist(u64 size)
{
    Block *block = _freelist;
    while(block)
    {
        if(block->get_size() >= size)
        {
            return block;
        }
        block = block->_next_free;
    }
    return block;
}

Block *Heap::get_best_fit_from_freelist(u64 size)
{
    Block *best_block = get_first_fit_from_freelist(size);
    if(!best_block)
    {
        return 0;
    }
    
    Block *block = best_block->_next_free;
    while(block)
    {
        u64 block_size = block->get_size();
        if(block_size >= size && block_size < best_block->get_size())
        {
            best_block = block;
        }
        block = block->_next_free;
    }
    return best_block;
}

void Heap::add_block_to_freelist(Block *block)
{
    if(!_freelist)
    {
        _freelist = block;
    }
    else
    {
        _freelist->_prev_free = block;
        block->_next_free = _freelist;
        _freelist = block;
    }
}

void Heap::insert_block_after_freelist(Block *block, Block *other_block)
{
    other_block->_next_free = block->_next_free;
    other_block->_prev_free = block;
    block->_next_free = other_block;
}

void Heap::remove_block_from_freelist(Block *block)
{
    if(!block->_prev_free)
    {
        _freelist = block->_next_free;
        if(_freelist) _freelist->_prev_free = 0; 
    }
    else if(!block->_next_free)
    {
        Block *prev = block->_prev_free; 
        prev->_next_free = 0;
    }
    else
    {
        Block *prev = block->_prev_free; 
        Block *next = block->_next_free;
        prev->_next_free = next;
        next->_prev_free = prev;
    }
    block->_prev_free = 0;
    block->_next_free = 0;
}

bool Heap::freelist_have_blocks()
{
    return _freelist != 0;
}

void Heap::debug_print_block(Block *block)
{
    printf("    size:   %lld\n", block->get_size()); 
    printf("    used:   %d\n", block->is_used()); 
    printf("    data:   %lld\n", *(u64 *)block->get_data()); 
}

void Heap::debug_print_state()
{
    u32 count = 0;
    Block *block = (Block *)_base;

    printf("memory info:\n");
    printf("-----------------------------\n");
    while(block)
    {
        printf("-------- block: %d ----------\n", ++count);
        debug_print_block(block);
        block = block->_next;
        printf("-----------------------------\n");
    }
    printf("\n\n");
}

};
