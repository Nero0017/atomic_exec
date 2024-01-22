from cmodel import *
from base_cmd import *
import numpy as np
import ctypes
from debugger.target_common import get_target_context


context = get_target_context("BM1684X")
runner = context.get_runner(0)

memory = runner.memory

weight = np.arange(4).astype(np.float32).view(np.uint8)
weight_addr = 4294967296
data = np.arange(16).astype(np.float32).reshape(1, 1, 4, 4)
data_addr = 4294971392
output = np.zeros(9).astype(np.float32)
output_address = 4294975488


weight_buffer = np.zeros(4096, dtype=np.uint8)
weight_buffer[: weight.size] = weight


memory.set_data_to_address(weight_addr, weight_buffer)
memory.set_data_to_address(data_addr, data)

cmd = DMA_tensor_0x000__reg.from_buffer_copy(cmds[0])
runner.dma_compute(cmd)
cmd = DMA_tensor_0x000__reg.from_buffer_copy(cmds[1])
runner.dma_compute(cmd)
cmd = sCONV_reg.from_buffer_copy(cmds[2])
runner.tiu_compute(cmd)
cmd = DMA_tensor_0x000__reg.from_buffer_copy(cmds[3])
runner.dma_compute(cmd)


memory.get_data_from_address(output_address, output)


# lib.chip_d2s
print(output)
