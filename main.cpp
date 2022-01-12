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

inline u64 align8(u64 size)
{
    u64 size_align = (size + 0x7) & ~0x7;
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

inline Block *heap_next_free_block(Heap *heap)
{
    return *(Block **)((u8 *)heap->free_list + sizeof(Block));
}

u8 *heap_alloc(Heap *heap, u64 size)
{
    u64 size_a = align8(size);
    
    while(heap->free_list)
    {
        Block *free_block = (Block *)heap->free_list;

        (void)free_block;

        heap->free_list = heap_next_free_block(heap);
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

void heap_free(Heap *heap, u8 *data)
{
    Block *block = block_get_from(data);
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
    *a = 123;
    u8 *b = heap_alloc(&heap, 9);
    *b = 231;
    heap_free(&heap, a);
    heap_free(&heap, b);

    printf("------------------------\n");
    printf("-     memory test      -\n");
    printf("------------------------\n");
    

    while(heap.free_list)
    {
        Block *free_block = (Block *)heap.free_list;

        printf("block header size = %lld\n", sizeof(Block));
        printf("block size  = %lld\n", block_get_size(free_block));
        printf("block free  = %d\n", block_is_free(free_block));
        printf("block vaule = %p\n", *(Block **)((u8 *)free_block + sizeof(Block)));
        printf("------------------------\n");

        heap.free_list = heap_next_free_block(&heap);
    }
    
    printf("heap base  = %p\n", heap.base);
    printf("heap size  = %lld\n", heap.size);
    printf("arena base = %p\n", arena.base);
    printf("arena size = %lld\n", arena.size);
    printf("------------------------\n");
    
    memory_destroy(&memory);
    return 0;
}
