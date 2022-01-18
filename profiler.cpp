#include "profiler.h"

#include <stdio.h>
#include <Windows.h>

namespace os
{

Profiler::Profiler() 
{
    printf("application start with the profiler...\n\n");
    LARGE_INTEGER l_freq;
    QueryPerformanceFrequency(&l_freq);
    _frequency = l_freq.QuadPart;
}

void Profiler::start(u8 slot)
{
    LARGE_INTEGER l_coun;
    QueryPerformanceCounter(&l_coun);
    _last_counter[slot] = l_coun.QuadPart;
}

void Profiler::stop(u8 slot)
{
    LARGE_INTEGER l_coun;
    QueryPerformanceCounter(&l_coun);
    _data[slot] = (f64)(l_coun.QuadPart - _last_counter[slot]) / _frequency;
}

void Profiler::print(u8 slot)
{
    printf("    profiler %d, takes = %lfs\n", slot, _data[slot]);
}

};
