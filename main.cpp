#include <iostream>
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

int main()
{
    device_example();
    std::cout << "hello world" << std::endl;
    return 0;
}