import struct

import struct
from debugger import disassembler as dis
from debugger import atomic_dialect


def write_gdma(buf):
    print("{")
    print("BEGIN_FAST_GEN_CMD(GDMA)")
    for index, i in enumerate(range(0, len(buf), 16)):
        low = struct.unpack("Q", buf[i : i + 8])[0]
        high = struct.unpack("Q", buf[i + 8 : i + 16])[0]
        print(f"low = {low};")
        print(f"high = {high};")
        print(f"WRITE_CMD_GDMA({index}, high, low);")
    print("END_FAST_GEN_CMD(GDMA, pid_node)")
    print("}")


def write_bdc(buf):
    print("{")
    print("BEGIN_FAST_GEN_CMD(BD)")
    for index, i in enumerate(range(0, len(buf), 16)):
        low = struct.unpack("Q", buf[i : i + 8])[0]
        high = struct.unpack("Q", buf[i + 8 : i + 16])[0]
        print(f"low = {low};")
        print(f"high = {high};")

        print(f"WRITE_CMD_BD({index}, high, low);")
    print("END_FAST_GEN_CMD(BD, pid_node)")
    print("}")


if __name__ == "__main__":
    bmodel = dis.BModel("./conv_workspace/conv_f32/compilation.bmodel")
    atomic_mlir = atomic_dialect.BModel2MLIR(bmodel)
    print(struct.unpack("ffff", bmodel.net[0].parameter[0].coeff_mem.data[:16]))
    print(str(atomic_mlir))

    reserve = 0
    for index, cmd in enumerate(atomic_mlir.create_cmdlist(), start=1):
        print(f"// {index}")
        if cmd.cmd_type == cmd.cmd_type.dma:
            # print("move", cmd.operands[0].address, cmd.results[0].address, cmd.operands[0])
            print("src_start_addr_h8", int(cmd.cmd.src_start_addr_h8))
            print("src_start_addr_l32", int(cmd.cmd.src_start_addr_l32))
            print("dst_start_addr_h8", int(cmd.cmd.dst_start_addr_h8))
            print("dst_start_addr_l32", int(cmd.cmd.dst_start_addr_l32))
            if cmd.cmd.src_start_addr_h8 == 1:
                cmd.cmd.src_start_addr_l32 += reserve

            if cmd.cmd.dst_start_addr_h8 == 1:
                cmd.cmd.dst_start_addr_l32 += reserve

            write_gdma(bytes(cmd.cmd))
        else:
            write_bdc(cmd.buf)
            pass
    # print("cmd = [0]  * 4")
    # for index, cmd in enumerate(atomic_mlir.create_cmdlist(), start=1):
    #     print(f"# {index}, {type(cmd.cmd)}")
    #     if cmd.cmd_type == cmd.cmd_type.dma:
    #         # print("move", cmd.operands[0].address, cmd.results[0].address, cmd.operands[0])
    #         # print("src_start_addr_h8", int(cmd.cmd.src_start_addr_h8))
    #         # print("src_start_addr_l32", int(cmd.cmd.src_start_addr_l32))
    #         # print("dst_start_addr_h8", int(cmd.cmd.dst_start_addr_h8))
    #         # print("dst_start_addr_l32", int(cmd.cmd.dst_start_addr_l32))
    #         if cmd.cmd.src_start_addr_h8 == 1:
    #             cmd.cmd.src_start_addr_l32 += reserve

    #         if cmd.cmd.dst_start_addr_h8 == 1:
    #             cmd.cmd.dst_start_addr_l32 += reserve

    #     print(
    #         f"cmd[{index-1}] = {cmd.buf} # {cmd.cmd.cmd_id}, {cmd.cmd.cmd_id_dep} {bin(struct.unpack('I',cmd.buf[:4])[0])}"
    #     )
    #     if cmd.cmd_type == cmd.cmd_type.dma:
    #         print(
    #             f"lib.launch_single_cmd(ctypes.create_string_buffer(cmd), 1, len(cmd))"
    #         )
    #     else:
    #         print(
    #             f"lib.launch_single_cmd(ctypes.create_string_buffer(cmd), 0, len(cmd))"
    #         )
