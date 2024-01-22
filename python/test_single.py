from cmodel import *
from base_cmd import *
import numpy as np
import ctypes

lib_fn = "/workspace/atomic_exec/build/libatomic_exec.so"
kernel_fn = "/workspace/atomic_exec/firmware_core/build/libbm1684x_kernel_module.so"


lib = lib_wrapper(open_lib(lib_fn))
init_handle = lib.init_handle
init_handle.restype = ctypes.c_void_p
runner = init_handle(ctypes.create_string_buffer(kernel_fn.encode()), 0)
runner = ctypes.c_void_p(runner)

reserved_offset = lib.get_reserved_mem(runner)
print(reserved_offset)
weight = np.arange(4).astype(np.float32) + 10
weight_addr = 4294967296 + reserved_offset
data = np.arange(16).astype(np.float32) + 10
data_addr = 4294971392 + reserved_offset
output = np.zeros(9).astype(np.float32) + 10
output_address = 4294975488 + reserved_offset


s2d_ret = lib.chip_s2d(
    runner,
    ctypes.c_uint64(weight_addr),
    weight.size * 4,
    weight.ctypes.data_as(ctypes.c_void_p),
)
check_data(weight, weight_addr, lib, runner)
s2d_ret = lib.chip_s2d(
    runner,
    ctypes.c_uint64(data_addr),
    data.size * 4,
    data.ctypes.data_as(ctypes.c_void_p),
)
check_data(data, data_addr, lib, runner)


def modify_address(cmd: DMA_tensor_0x000__reg):
    if cmd.src_start_addr_h8 == 1:
        print("src_start_addr_h8", int(cmd.src_start_addr_h8))
        print("src_start_addr_l32", int(cmd.src_start_addr_l32))
        print(f"  src addr {(cmd.src_start_addr_h8 << 32) + cmd.src_start_addr_l32}")
        cmd.src_start_addr_l32 += reserved_offset
        print("src_start_addr_h8", int(cmd.src_start_addr_h8))
        print("src_start_addr_l32", int(cmd.src_start_addr_l32))
        print(f"fixed addr {(cmd.src_start_addr_h8 << 32) + cmd.src_start_addr_l32}")

    if cmd.dst_start_addr_h8 == 1:
        print("dst_start_addr_h8", int(cmd.dst_start_addr_h8))
        print("dst_start_addr_l32", int(cmd.dst_start_addr_l32))
        print(
            f"      dst addr {(cmd.dst_start_addr_h8 << 32) + cmd.dst_start_addr_l32}"
        )
        cmd.dst_start_addr_l32 += reserved_offset
        print("dst_start_addr_h8", int(cmd.dst_start_addr_h8))
        print("dst_start_addr_l32", int(cmd.dst_start_addr_l32))
        print(
            f"fixed dst addr {(cmd.dst_start_addr_h8 << 32) + cmd.dst_start_addr_l32}"
        )
    print()


def modify_cmd_id(cmd: cmd_base_reg):
    cmd.cmd_id = 1
    cmd.cmd_id_dep = 0


# 1
cmd = DMA_tensor_0x000__reg.from_buffer_copy(cmds[0])
modify_address(cmd)
modify_cmd_id(cmd)
cmd_bytes = bytes(cmd)
print(cmd_bytes)
lib.launch_single_cmd(runner, ctypes.create_string_buffer(cmd_bytes), 1, len(cmd_bytes))
# 2
cmd = DMA_tensor_0x000__reg.from_buffer_copy(cmds[1])
modify_address(cmd)
modify_cmd_id(cmd)
cmd_bytes = bytes(cmd)
print(cmd_bytes)
lib.launch_single_cmd(runner, ctypes.create_string_buffer(cmd_bytes), 1, len(cmd_bytes))
# 3
cmd = sCONV_reg.from_buffer_copy(cmds[2])
modify_cmd_id(cmd)
cmd_bytes = bytes(cmd)
print(cmd_bytes)
lib.launch_single_cmd(runner, ctypes.create_string_buffer(cmd_bytes), 0, len(cmd_bytes))
# 4
cmd = DMA_tensor_0x000__reg.from_buffer_copy(cmds[3])
modify_address(cmd)
modify_cmd_id(cmd)
cmd_bytes = bytes(cmd)
print(cmd_bytes)
lib.launch_single_cmd(runner, ctypes.create_string_buffer(cmd_bytes), 1, len(cmd_bytes))

d2s_ret = lib.chip_d2s(
    runner,
    ctypes.c_uint64(output_address),
    output.size * output.dtype.itemsize,
    output.ctypes.data_as(ctypes.c_void_p),
)
# lib.chip_d2s
print(output)

# lib.deini)
lib.deinit(runner)
