#include "heap.h"
#include <stdio.h>

struct Entity
{
    f32 x, y;
    f32 vx, vy;
};

struct BigEntity
{
    f32 x, y;
    u64 data[256];
};

int main()
{
    printf("size of Entity = %lld\n", sizeof(Entity));
    
    mem::Memory memory(MB(256));
    mem::Arena arena(&memory, MB(128)); 
    mem::Heap heap(&memory, MB(128));
    
    Entity *e1 = (Entity *)heap.malloc(sizeof(Entity));
    Entity *e2 = (Entity *)heap.malloc(sizeof(Entity));
    Entity *e3 = (Entity *)heap.malloc(sizeof(Entity));
    Entity *e4 = (Entity *)heap.malloc(sizeof(Entity));
    BigEntity *e5 = (BigEntity *)heap.malloc(sizeof(BigEntity));
    Entity *e6 = (Entity *)heap.malloc(sizeof(Entity));
    Entity *e7 = (Entity *)heap.malloc(sizeof(Entity));
    Entity *e8 = (Entity *)heap.malloc(sizeof(Entity));

    heap.free((u8 *)e3);

    heap.debug_print_state();

    heap.free((u8 *)e4);

    heap.debug_print_state();
    
    heap.free((u8 *)e2);
    
    heap.debug_print_state();

    (void)e1;
    (void)e2;
    (void)e3;
    (void)e4;
    (void)e5;
    (void)e6;
    (void)e7;
    (void)e8;
    
    return 0;
}
