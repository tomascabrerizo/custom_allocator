#ifndef PROFILER_H
#define PROFILER_H

#include "types.h"

namespace os
{

class Profiler
{
public:
    Profiler(); 
    
    void start(u8 slot);
    void stop(u8 slot);
    void print(u8 slot);
private:
    u64 _frequency;
    
    u64 _last_counter[256];
    f64 _data[256];
};

};

#endif // PROFILER_H
