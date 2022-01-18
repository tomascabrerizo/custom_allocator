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
    os::Profiler prof;

    prof.start(GLOBAL_ALLOC);
    for(u32 i = 0; i < 1000; ++i)
    {
        Entity *e1 = (Entity *)malloc(sizeof(Entity));
        BigEntity *e2 = (BigEntity *)malloc(sizeof(BigEntity));
        free(e1);
        free(e2);
    }
    prof.stop(GLOBAL_ALLOC);
    
    prof.start(CUSTOM_ALLOC);
    for(u32 i = 0; i < 1000; ++i)
    {
        Entity *e1 = (Entity *)heap.malloc(sizeof(Entity));
        BigEntity *e2 = (BigEntity *)heap.malloc(sizeof(BigEntity));
        heap.free((u8 *)e1);
        heap.free((u8 *)e2);
    }
    prof.stop(CUSTOM_ALLOC);
    
    printf("global allocator takes:\n");
    prof.print(GLOBAL_ALLOC);
    printf("custom allocator takes:\n");
    prof.print(CUSTOM_ALLOC);

    heap.debug_print_state();
     
    return 0;
}
