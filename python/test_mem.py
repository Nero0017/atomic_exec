from cmodel import *
import numpy as np

import ctypes

lib_fn = "/workspace/atomic_exec/build/libatomic_exec.so"
kernel_fn = "/workspace/atomic_exec/firmware_core/build/libbm1684x_kernel_module.so"

lib = lib_wrapper(open_lib(lib_fn))
lib.init_handle.restype = ctypes.c_void_p
runner = lib.init_handle(ctypes.create_string_buffer(kernel_fn.encode()), 0)
runner = ctypes.c_void_p(runner)

reserved_offset = lib.get_reserved_mem(runner)


address = 4294975488 + reserved_offset

data = (np.random.rand(4096 * 1024) * 10).astype(np.uint8).reshape(1024, 1024, 4)
data = np.ascontiguousarray(data)
output_data = np.zeros_like(data)
print(data)


s2d_ret = lib.chip_s2d(
    runner,
    ctypes.c_uint64(address),
    data.size * data.dtype.itemsize,
    data.ctypes.data_as(ctypes.c_void_p),
)

check_data(data, address, lib, runner)

lib.deinit(runner)
