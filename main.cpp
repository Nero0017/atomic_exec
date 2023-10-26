#include <iostream>
#include "sg_api_struct.h"
#include "tpu_defs.h"
#include "bmlib_runtime.h"
#include "support.h"

void device_example()
{

    bm_handle_t bm_handle;
    auto ret = bm_dev_request(&bm_handle, 0);

    bm_device_mem_t device_mem;
    size_t data_size = 5;
    char data[] = {0, 1, 2, 3, 4};

    char *d_data = (char *)malloc(data_size);

    ASSERT_SUCCESS({bm_malloc_device_byte(bm_handle, &device_mem, data_size)});
    ASSERT_SUCCESS({bm_memcpy_s2d_partial(bm_handle, device_mem, (void *)data, data_size)});
    ASSERT_SUCCESS({bm_memcpy_d2s_partial(bm_handle, (void *)d_data, device_mem, data_size)});
    ASSERT_SUCCESS({bm_malloc_device_byte(bm_handle, &device_mem, data_size)});

    for (int i = 0; i < data_size; i++)
    {
        if (d_data[i] != data[i])
        {
            std::cerr << "err cmp " << std::endl;
        }
    }
    delete d_data;
    bm_free_device(bm_handle, device_mem);
    bm_dev_free(bm_handle);
}

void tpu_kernel_launch_example()
{

    bm_handle_t bm_handle;
    auto ret = bm_dev_request(&bm_handle, 0);
    tpu_kernel_module_t tpu_module;
    tpu_kernel_function_t func_id;

    tpu_module = tpu_kernel_load_module_file(bm_handle, "/workspace/atomic_exec/build/firmware_core/libbm1684x_kernel_module.so");

    bm_device_mem_t device_mem;
    ASSERT_SUCCESS({bm_malloc_device_byte(bm_handle, &device_mem, 128)});

    float data[16] = {
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        8,
        9,
        11,
        12,
        13,
        14,
        15};
    float output_data[16] = {0};
    size_t data_size = sizeof(float) * 16;
    ASSERT_SUCCESS({bm_memcpy_s2d_partial(bm_handle, device_mem, (void *)data, data_size)});

    tpu_kernel_api_const_binary_t params = {0};
    params.input_global_offset = device_mem.u.device.device_addr;
    params.output_global_offset = device_mem.u.device.device_addr;
    params.dims = 2;
    params.shape[0] = 4;
    params.shape[1] = 4;
    params.binary_type = 0;
    params.dtype = DT_FP32;
    params.const_value = 1;

    func_id = tpu_kernel_get_function(bm_handle, tpu_module, "tpu_kernel_api_const_binary");
    auto lret = tpu_kernel_launch(bm_handle, func_id, &params, sizeof(params));

    ASSERT_SUCCESS({bm_memcpy_d2s_partial(bm_handle, (void *)output_data, device_mem, data_size)});
    std::cout << func_id << std::endl;
    std::cout << "launch" << lret << std::endl;
    bm_free_device(bm_handle, device_mem);
    tpu_kernel_unload_module(bm_handle, tpu_module);
    bm_dev_free(bm_handle);
}

int main()
{
    // device_example();
    tpu_kernel_launch_example();
    std::cout << "hello world" << std::endl;
    return 0;
}