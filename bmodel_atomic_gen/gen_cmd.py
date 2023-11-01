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
    for index, cmd in enumerate(atomic_mlir.create_cmdlist(), start=1):
        print(f"// {index}")
        if cmd.cmd_type == cmd.cmd_type.dma:
            # print("move", cmd.operands[0].address, cmd.results[0].address, cmd.operands[0])
            write_gdma(cmd.buf)
        else:
            write_bdc(cmd.buf)
            pass
