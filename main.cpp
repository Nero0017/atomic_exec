#include <iostream>
#include "bmlib_runtime.h"
#include "support.h"

typedef struct
{
    int length;
    char data[8];
} sg_api_atomic_t;

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

    // const unsigned int *p = kernel_module_data;
    // size_t length = sizeof(kernel_module_data);
    // tpu_module = tpu_kernel_load_module(bm_handle, (const char *)p, length);
    sg_api_atomic_t params = {0};
    params.length = 5;

    func_id = tpu_kernel_get_function(bm_handle, tpu_module, "tpu_kernel_launch_atomic");
    std::cout << func_id << std::endl;
    auto lret = tpu_kernel_launch(bm_handle, func_id, &params, sizeof(params));
    std::cout << "launch" << lret << std::endl;

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