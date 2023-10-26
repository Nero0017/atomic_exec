#include "nodechip_anyatomic.h"

void tpu_kernel_launch_atomic(const void *args) {
  // TPUKERNEL_ASSERT_INFO(false, "in kernel hello\n");
  tpu_initialize();
  // sg_api_atomic_t *params = (sg_api_atomic_t *)args;
  
  tpu_poll();
}
TPUKERNEL_FUNC_REGISTER(tpu_kernel_launch_atomic);
