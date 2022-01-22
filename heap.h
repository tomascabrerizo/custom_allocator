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
    Heap(Memory *mem, u64 size);
    ~Heap() = default;
    
    u8 *malloc(u64 size);
    void free(u8 *base);
    u8 *realloc(u8 *data, u64 size);

    void debug_print_block(Block *block);
    void debug_print_state();

private:
    
    Block *_top;
    Block *_freelist;

    void add_block(Block *block, u64 size);
    void remove_block(Block *block);
    Block *get_block_from_data(u8 *data);
    bool last_allocated_block(Block *block);
    void resize_block(Block *block, u64 size);
    
    Block *get_first_fit_from_freelist(u64 size);
    Block *get_best_fit_from_freelist(u64 size);

    void add_block_to_freelist(Block *block);
    bool freelist_have_blocks();
    void remove_block_from_freelist(Block *block);

    void try_to_split_block(Block *block, u64 size);
    void split_block(Block *block, u64 size, u64 new_block_size);
    void insert_block_after(Block *block, Block *other_block);
    void insert_block_after_freelist(Block *block, Block *other_block);

    Block *try_to_merge_block(Block *block);
    void try_to_merge_block_right(Block *block);
    Block *try_to_merge_block_left(Block *block);

    u8 *slow_realloc(Block *block, u64 size);
};

};

#endif // HEAP_H
