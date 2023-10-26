#include "sg_api_struct.h"
#include "tpu_kernel.h"

extern void nodechip_const_binary_fp(global_addr_t A_global_addr,
                                     global_addr_t res_global_addr,
                                     const int *shape, int shape_dim,
                                     float B_const_val, int inversed,
                                     int binary_type, data_type_t dtype,
                                     int if_relu, float relu_upper_limit);

void tpu_kernel_api_const_binary(const void *api_buf) {
  tpu_kernel_api_const_binary_t *api = (tpu_kernel_api_const_binary_t *)api_buf;
  tpu_initialize();
  nodechip_const_binary_fp(
      api->input_global_offset, api->output_global_offset, api->shape,
      api->dims, api->const_value, api->inversed, api->binary_type,
      (data_type_t)api->dtype, api->if_relu, api->relu_upper_limit);
  tpu_poll();
}

TPUKERNEL_FUNC_REGISTER(tpu_kernel_api_const_binary);