#include <Windows.h>

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#define KB(n) (n * 1024LL)
#define MB(n) (n * 1024LL * 1024LL)
#define GB(n) (n * 1024LL * 1024LL * 1024LL)

#define BLOCK_MINIMUN_SIZE (sizeof(Block) + sizeof(u64))

inline u64 align8(u64 size)
{
    u64 size_align = (size + sizeof(u64)-1) & ~(sizeof(u64)-1);
    return size_align;
}

struct Memory
{
    u8 *base;
    u64 used;
    u64 size;
};

Memory memory_init(u64 size)
{
    Memory memory = {};
    memory.size = align8(size);
    memory.base = (u8 *)VirtualAlloc(0, memory.size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    memory.used = 0;
    return memory;
}

void memory_destroy(Memory *memory)
{
    VirtualFree(memory->base, 0, MEM_RELEASE);
}

struct Arena 
{
    u8 *base;
    u64 used;
    u64 size;
};

Arena arena_create(Memory *mem, u64 size)
{
    u64 size_a = align8(size);
    assert(mem->used + size_a <= mem->size);
    Arena arena = {};
    arena.size = size_a;
    arena.base = mem->base + mem->used;
    arena.used = 0;
    mem->used += size_a;
    return arena;
}

u8 *arena_push_size(Arena *arena, u64 size)
{
    assert(arena->used + size <= arena->size);
    u8 *result = arena->base + arena->used;
    arena->used += size;
    return result;
}

void arena_pop_size(Arena *arena, u64 size)
{
    assert(arena->used - size >= 0);
    arena->used -= size;
}

struct Block
{
    u64 size;
    Block *next;
};

inline void block_set_used(Block *block)
{
    block->size |= 0x1;
}

inline void block_set_free(Block *block)
{
    block->size &= ~0x1;
}

inline bool block_is_free(Block *block)
{
    return !(block->size & 0x1);
}

inline u64 block_get_size(Block *block)
{
    return block->size & ~0x1; 
}

Block *block_get_from(u8 *data)
{
    Block *block = (Block *)(data - sizeof(Block));
    return block;
}

inline void block_set_next_free_block(Block *block, Block *next)
{
    Block **block_next = (Block **)((u8 *)block + sizeof(Block));
    *block_next = next;
}

inline Block *block_next_free_block(Block *block)
{
    return *(Block **)((u8 *)block + sizeof(Block));
}

void block_try_split(Block *block, u64 size)
{
    u64 block_size = block_get_size(block);
    u64 new_block_size = block_size - size;
    if(new_block_size >= BLOCK_MINIMUN_SIZE)
    {
        Block *new_block = (Block *)((u8 *)block + sizeof(Block) + size);
        
        new_block->size = new_block_size - sizeof(Block);
        block_set_free(new_block);
        block_set_next_free_block(new_block, block_next_free_block(block)); 
        
        block_set_next_free_block(block, new_block); 
        block->size = size;
        block_is_free(block);
    }
}

struct Heap
{
    u8 *base;
    u64 used;
    u64 size;
    Block *top;
    Block *free_list;
};

Heap heap_create(Memory *mem, u64 size)
{
    u64 size_a = align8(size);
    assert(mem->used + size_a <= mem->size);
    Heap heap = {};
    heap.size = size_a;
    heap.base = mem->base + mem->used;
    mem->used += size_a;
    return heap;
}

Block *heap_best_fit(Heap *heap, u64 size)
{
    Block *free_block = heap->free_list;
    Block *prev = 0;
    Block *best_fit = 0;
    Block *best_fit_prev = 0;
    
    while(free_block)
    {
        u64 free_block_size = block_get_size(free_block);
        if(!best_fit && (free_block_size >= size))
        {
            best_fit_prev = prev;
            best_fit = free_block;
        } 
        else if(best_fit && (free_block_size >= size) && (block_get_size(best_fit) > free_block_size))
        {
            best_fit_prev = prev;
            best_fit = free_block;
        }
        
        prev = free_block;
        free_block = block_next_free_block(free_block);
    }

    if(best_fit)
    {
        block_try_split(best_fit, size);

        if(!best_fit_prev)
        {
            heap->free_list = block_next_free_block(best_fit);
        }
        else
        {
            block_set_next_free_block(best_fit_prev, block_next_free_block(best_fit));
        }
    }

    return best_fit;
}

u8 *heap_alloc(Heap *heap, u64 size)
{
    u64 size_a = align8(size);
    if(Block *block = heap_best_fit(heap, size_a))
    {
        block_set_used(block);
        u8 *data = (u8 *)block + sizeof(Block);
        return data;
    }

    u64 alloc_size = sizeof(Block) + size_a;
    assert(heap->used + alloc_size <= heap->size); 
    
    Block *block = (Block *)(heap->base + heap->used);
    block->size = size_a;
    block_set_used(block);

    if(heap->top)
    {
        heap->top->next = block;
    }
    heap->top = block;
    heap->used += alloc_size;
    
    u8 *data = (u8 *)block + sizeof(Block);
    return data;
}

void heap_remove_from_freelist(Heap *heap, Block *block)
{
    Block *free_block = heap->free_list;
    Block *free_block_prev = 0;
    while(free_block)
    {
        if(block == free_block)
        {
            if(!free_block_prev)
            {
                heap->free_list = block_next_free_block(block);
            }
            else
            {
                block_set_next_free_block(free_block_prev, block_next_free_block(block));
            }
            break;
        }
        
        free_block_prev = free_block;
        free_block = block_next_free_block(free_block);
    }
}

void heap_try_to_merge_block(Heap *heap, Block *block)
{
    Block *block_next = block->next;
    if(block_next && block_is_free(block_next))
    {
        block->size += (sizeof(Block) + block_get_size(block_next));
        heap_remove_from_freelist(heap, block_next);
    }
}

void heap_free(Heap *heap, u8 *data)
{
    Block *block = block_get_from(data);
    heap_try_to_merge_block(heap, block);
    block_set_free(block);
     
    Block **next = (Block **)data;
    if(heap->free_list)
    {
        *next = heap->free_list;
    }
    else
    {
        *next = 0;
    }
    heap->free_list = block; 
}

int main()
{
    Memory memory = memory_init(MB(256));
    
    Heap heap = heap_create(&memory, MB(128));
    Arena arena = arena_create(&memory, MB(128));

    u8 *a = heap_alloc(&heap, 1);
    Block *ab = block_get_from(a);
    
    u8 *b = heap_alloc(&heap, 64);
    Block *bb = block_get_from(b);
    
    heap_free(&heap, b);
    heap_free(&heap, a);
    
    u8 *c = heap_alloc(&heap, 3);
    Block *cb = block_get_from(c);
    
    u8 *d = heap_alloc(&heap, 40);
    Block *db = block_get_from(d);

    printf("------------------------\n");
    printf("-       free list      -\n");
    printf("------------------------\n");

    Block *free_block = heap.free_list;
    while(free_block)
    {
        printf("block header size = %lld\n", sizeof(Block));
        printf("block size  = %lld\n", block_get_size(free_block));
        printf("block free  = %d\n", block_is_free(free_block));
        printf("block vaule = %p\n", *(Block **)((u8 *)free_block + sizeof(Block)));
        printf("------------------------\n");

        free_block = block_next_free_block(free_block);
    }
    
    printf("------------------------\n");
    printf("-     memory test      -\n");
    printf("------------------------\n");

    
    printf("a header size = %lld\n", sizeof(Block));
    printf("a size  = %lld\n", block_get_size(ab));
    printf("a free  = %d\n", block_is_free(ab));
    printf("a vaule = %p\n", *(Block **)((u8 *)ab + sizeof(Block)));
    printf("------------------------\n");

    printf("b header size = %lld\n", sizeof(Block));
    printf("b size  = %lld\n", block_get_size(bb));
    printf("b free  = %d\n", block_is_free(bb));
    printf("b vaule = %p\n", *(Block **)((u8 *)bb + sizeof(Block)));
    printf("------------------------\n");

    printf("c header size = %lld\n", sizeof(Block));
    printf("c size  = %lld\n", block_get_size(cb));
    printf("c free  = %d\n", block_is_free(cb));
    printf("c vaule = %p\n", *(Block **)((u8 *)cb + sizeof(Block)));
    printf("------------------------\n");

    printf("d header size = %lld\n", sizeof(Block));
    printf("d size  = %lld\n", block_get_size(db));
    printf("d free  = %d\n", block_is_free(db));
    printf("d vaule = %p\n", *(Block **)((u8 *)db + sizeof(Block)));
    printf("------------------------\n");

    printf("------------------------\n");
    printf("-     memory info      -\n");
    printf("------------------------\n");
    
    printf("heap base  = %p\n", heap.base);
    printf("heap size  = %lld\n", heap.size);
    printf("arena base = %p\n", arena.base);
    printf("arena size = %lld\n", arena.size);
    printf("------------------------\n");
    
    memory_destroy(&memory);
    return 0;
}
