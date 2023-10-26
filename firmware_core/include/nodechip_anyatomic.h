#ifndef NODECHIP_ATOMICRUN_H
#define NODECHIP_ATOMICRUN_H

#ifndef WIN32
#define WITH_PLATFORM(x) __attribute__((packed)) x
#else
#define WITH_PLATFORM(x) x
#endif

#include "tpu_kernel.h"

typedef struct
{
    int length;
    char data[8];
} WITH_PLATFORM(sg_api_atomic_t);

void tpu_kernel_launch_atomic(const void *param);

#endif