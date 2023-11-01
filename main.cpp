#include "bmlib_runtime.h"
#include "sg_api_struct.h"
#include "support.h"
#include "tpu_defs.h"
#include <iostream>

void device_example() {

  bm_handle_t bm_handle;
  auto ret = bm_dev_request(&bm_handle, 0);

  bm_device_mem_t device_mem;
  size_t data_size = 5;
  char data[] = {0, 1, 2, 3, 4};

  char *d_data = (char *)malloc(data_size);

  ASSERT_SUCCESS({bm_malloc_device_byte(bm_handle, &device_mem, data_size)});
  ASSERT_SUCCESS(
      {bm_memcpy_s2d_partial(bm_handle, device_mem, (void *)data, data_size)});
  ASSERT_SUCCESS({bm_memcpy_d2s_partial(bm_handle, (void *)d_data, device_mem,
                                        data_size)});
  ASSERT_SUCCESS({bm_malloc_device_byte(bm_handle, &device_mem, data_size)});

  for (int i = 0; i < data_size; i++) {
    if (d_data[i] != data[i]) {
      std::cerr << "err cmp " << std::endl;
    }
  }
  delete d_data;
  bm_free_device(bm_handle, device_mem);
  bm_dev_free(bm_handle);
}

void tpu_kernel_launch_example() {

  bm_handle_t bm_handle;
  auto ret = bm_dev_request(&bm_handle, 0);
  tpu_kernel_module_t tpu_module;
  tpu_kernel_function_t func_id;

  tpu_module = tpu_kernel_load_module_file(
      bm_handle,
      "/workspace/atomic_exec/build/firmware_core/libbm1684x_kernel_module.so");

  bm_device_mem_t device_mem;
  ASSERT_SUCCESS({bm_malloc_device_byte(bm_handle, &device_mem, 128)});

  float data[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15};
  float output_data[16] = {0};
  size_t data_size = sizeof(float) * 16;
  ASSERT_SUCCESS(
      {bm_memcpy_s2d_partial(bm_handle, device_mem, (void *)data, data_size)});

  tpu_kernel_api_const_binary_t params = {0};
  params.input_global_offset = device_mem.u.device.device_addr;
  params.output_global_offset = device_mem.u.device.device_addr;
  params.dims = 2;
  params.shape[0] = 4;
  params.shape[1] = 4;
  params.binary_type = 0;
  params.dtype = DT_FP32;
  params.const_value = 1;

  func_id = tpu_kernel_get_function(bm_handle, tpu_module,
                                    "tpu_kernel_api_const_binary");
  auto lret = tpu_kernel_launch(bm_handle, func_id, &params, sizeof(params));

  ASSERT_SUCCESS({bm_memcpy_d2s_partial(bm_handle, (void *)output_data,
                                        device_mem, data_size)});

  std::cout << func_id << std::endl;
  std::cout << "launch" << lret << std::endl;
  bm_free_device(bm_handle, device_mem);
  tpu_kernel_unload_module(bm_handle, tpu_module);
  bm_dev_free(bm_handle);
}

void tpu_kernel_atomic_example() {

  bm_handle_t bm_handle;
  auto ret = bm_dev_request(&bm_handle, 0);
  tpu_kernel_module_t tpu_module;
  tpu_kernel_function_t func_id;

  tpu_module = tpu_kernel_load_module_file(
      bm_handle,
      "/workspace/atomic_exec/build/firmware_core/libbm1684x_kernel_module.so");

  bm_device_mem_t data_mem, weight_mem, output_mem;
  ASSERT_SUCCESS({bm_malloc_device_byte(bm_handle, &data_mem, 128)});

  u64 ctx_size = 8196; //

  // u64 addr;
  // addr = ctx->bm_dev->bm_device_alloc_mem(pmem->size);
  u64 global_offset = 84938752;

  weight_mem = bm_mem_from_device(4294967296, 4 * sizeof(float));
  data_mem = bm_mem_from_device(4294971392, 16 * sizeof(float));
  output_mem = bm_mem_from_device(4294975488, 9 * sizeof(float));

  float data[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  float weight[4] = {0, 1, 2, 3};
  float output_data[9] = {0};

  bm_memcpy_s2d(bm_handle, weight_mem, (void *)weight);
  bm_memcpy_s2d(bm_handle, data_mem, (void *)data);

  sg_api_atomic_t params = {0};
  func_id = tpu_kernel_get_function(bm_handle, tpu_module,
                                    "tpu_kernel_launch_atomic_gdma");
  auto lret = tpu_kernel_launch(bm_handle, func_id, &params, sizeof(params));

  ASSERT_SUCCESS({bm_memcpy_d2s(bm_handle, (void *)output_data, output_mem)});
  std::cout << func_id << std::endl;
  for (int i = 0; i < 9; i++) {
    std::cout << output_data[i] << ',';
  }
  std::cout << std::endl;
  std::cout << "launch" << lret << std::endl;
  tpu_kernel_unload_module(bm_handle, tpu_module);
  // bm_dev_free(bm_handle);
}

int main() {
  // device_example();
  tpu_kernel_atomic_example();
  std::cout << "hello world" << std::endl;
  return 0;
}
