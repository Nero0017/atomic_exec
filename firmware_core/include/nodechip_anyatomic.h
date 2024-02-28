#ifndef NODECHIP_ATOMICRUN_H
#define NODECHIP_ATOMICRUN_H

#include "tpu_kernel.h"

void tpu_kernel_launch_atomic(const void *param);
void tpu_kernel_mcu_cpy_l2s(const void *param);
void tpu_kernel_gdma_cpy_l2s(const void *param);
void tpu_kernel_launch_modified_pio(const void *param);

#endif