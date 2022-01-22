#include "heap.h"
#include "profiler.h"
#include <stdio.h>
#include <stdlib.h>

struct Entity
{
    f32 x, y;
    u64 data[256];
};

struct BigEntity
{
    f32 x, y;
    u64 data[1024];
};

int main()
{
    mem::Memory memory(MB(256));
    mem::Arena arena(&memory, MB(128)); 
    mem::Heap heap(&memory, MB(128));

#define GLOBAL_ALLOC 0
#define CUSTOM_ALLOC 1
#define ARENA_ALLOC 2

#define TEST_COUNT 1500
    static Entity *ge_ptr[TEST_COUNT];
    static Entity *ce_ptr[TEST_COUNT];
    static Entity *ae_ptr[TEST_COUNT];

    os::Profiler prof;

    prof.start(GLOBAL_ALLOC);
    for(u32 i = 0; i < TEST_COUNT; ++i)
    {
        ge_ptr[i] = (Entity *)malloc(sizeof(Entity));
    }
    prof.stop(GLOBAL_ALLOC);
    
    prof.start(CUSTOM_ALLOC);
    for(u32 i = 0; i < TEST_COUNT; ++i)
    {
        ce_ptr[i] = (Entity *)heap.allocate(sizeof(Entity));
    }
    prof.stop(CUSTOM_ALLOC);
    
    prof.start(ARENA_ALLOC);
    for(u32 i = 0; i < TEST_COUNT; ++i)
    {
        ae_ptr[i] = (Entity *)arena.push_size(sizeof(Entity));
    }
    prof.stop(ARENA_ALLOC);

    printf("global allocator takes:\n");
    prof.print(GLOBAL_ALLOC);
    printf("custom allocator takes:\n");
    prof.print(CUSTOM_ALLOC);
    printf("arena allocator takes:\n");
    prof.print(ARENA_ALLOC);
    
    //printf("\n");
    //heap.debug_print_state();
    printf("heap used %lld, arena used %lld\n", heap.get_used(), arena.get_used());

    return 0;
}
