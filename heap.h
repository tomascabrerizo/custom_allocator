#ifndef HEAP_H
#define HEAP_H

#include "arena.h"

namespace mem
{

#define BLOCK_MIN_SIZE (sizeof(Block) + sizeof(u64))

struct Block
{
    void set_used(bool used);
    bool is_used();
    void set_size(u64 size);
    u64 get_size();
    u8 *get_data();
    
    Block *_next;
    Block *_prev;

    Block *_next_free;
    Block *_prev_free;
private:
    u64 _size;
};

class Heap : public Arena 
{
public:
    /*@docs------------------------------------------------
    [FNC]:  - Heap(Memory *mem, u64 size)
    [IN ]:
            - mem (Memory *): pointer to a memory object
            - size (u64): total size of the heap in bytes
    [OUT]:
            - heap (Heap): new Heap object
    -----------------------------------------------------*/
    Heap(Memory *mem, u64 size);
    
    /*@docs------------------------------------------------
    [FNC]:  - Heap::allocate(u64 size)
    [IN ]:
            - size (u64): number of bytes to allocate 
    [OUT]:
            - data (u8 *): pointer to the new allocated data 
    -----------------------------------------------------*/
    u8 *allocate(u64 size);

    /*@docs------------------------------------------------
    [FNC]:  - Heap::deallocate(u8 *base)
    [IN ]:
            - base (u8 *): already allocated pointer to be free 
    -----------------------------------------------------*/
    void deallocate(u8 *base);

    /*@docs------------------------------------------------
    [FNC]:  - Heap::reallocate(u64 size)
    [IN ]:
            - data (u8 *): pointer to the memory buffer to be reallocated 
            - size (u64): size of the new memory buffer 
    [OUT]:
            - data (u8 *): pointer to the new allocated data 
    -----------------------------------------------------*/
    u8 *reallocate(u8 *data, u64 size);

    ~Heap() = default;

    void debug_print_block(Block *block);
    void debug_print_state();

private:
    
    Block *_top;
    Block *_freelist;
    
    // (Heap) functions.
    u8 *slow_realloc(Block *block, u64 size);
    
    // (Heap - Block) functions.
    void add_block(Block *block, u64 size);
    void insert_block_after(Block *block, Block *other_block);
    void remove_block(Block *block);
    Block *get_block_from_data(u8 *data);
    void resize_block(Block *block, u64 size);
    bool last_allocated_block(Block *block);
    Block *try_to_merge_block(Block *block);
    void try_to_merge_block_right(Block *block);
    Block *try_to_merge_block_left(Block *block);
    void try_to_split_block(Block *block, u64 size);
    void split_block(Block *block, u64 size, u64 new_block_size);
    
    // (Heap - Freelist) functions
    Block *get_first_fit_from_freelist(u64 size);
    Block *get_best_fit_from_freelist(u64 size);
    void add_block_to_freelist(Block *block);
    void insert_block_after_freelist(Block *block, Block *other_block);
    void remove_block_from_freelist(Block *block);
    bool freelist_have_blocks();
};

};

#endif // HEAP_H
