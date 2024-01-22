# ==============================================================================
#
# Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
#
# TPU-MLIR is licensed under the 2-Clause BSD License except for the
# third-party components.
#
# ==============================================================================

import os
import shutil
import numpy as np
import ctypes
from functools import lru_cache
from ctypes import Structure, POINTER
from numpy import ndarray
from typing import List
import tempfile


def check_data(gd, address, lib, runner=None):
    actual = np.zeros_like(gd)

    lib.chip_d2s(
        runner,
        ctypes.c_uint64(address),
        actual.size * actual.dtype.itemsize,
        actual.ctypes.data_as(ctypes.c_void_p),
    )
    print((gd == actual).all())


def c_array_to_ndarray(x, shape):
    if isinstance(x, int):
        x = ctypes.c_void_p(x)
    if isinstance(shape, int):
        shape = (shape,)
    try:
        p = ctypes.cast(x, ctypes.POINTER(ctypes.c_uint8))
    except Exception:
        raise Exception(f"unsupported memory access: {x}")
    else:
        return np.ctypeslib.as_array(p, shape=shape)


class local_mem(Structure):
    _fields_ = [
        ("raw_ptr", POINTER(ctypes.c_char)),
        ("mem_arr", POINTER(POINTER(ctypes.c_uint32))),
        ("count", ctypes.c_int32),
        ("size_per_mem", ctypes.c_int32),
        ("align_num", ctypes.c_int32),
        ("need_free", ctypes.c_int32),
    ]


class lib_wrapper:
    __slots__ = ["_lib", "_fntab"]

    def __init__(self, lib):
        self._lib = lib
        self._fntab = {}

    def __getattr__(self, name):
        try:
            return self._fntab[name]
        except KeyError:
            # Lazily wraps new functions as they are requested
            cfn = getattr(self._lib, name)
            wrapped = _lib_fn_wrapper(cfn)
            self._fntab[name] = wrapped
            return wrapped

    @property
    def _name(self):
        return self._lib._name

    @property
    def _handle(self):
        return self._lib._handle


class _lib_fn_wrapper(object):
    __slots__ = ["_cfn"]

    def __init__(self, cfn):
        self._cfn = cfn

    @property
    def argtypes(self):
        return self._cfn.argtypes

    @argtypes.setter
    def argtypes(self, argtypes):
        self._cfn.argtypes = argtypes

    @property
    def restype(self):
        return self._cfn.restype

    @restype.setter
    def restype(self, restype):
        self._cfn.restype = restype

    def __call__(self, *args, **kwargs):
        return self._cfn(*args, **kwargs)


# @contextmanager
def temp_position(file):
    os.makedirs(os.path.expanduser("~/.cache/tpu-mlir"), exist_ok=True)
    tempdirname = tempfile.TemporaryDirectory(
        dir=os.path.expanduser("~/.cache/tpu-mlir")
    ).name
    # make sure
    os.makedirs(tempdirname, exist_ok=True)
    temp_fn = os.path.join(tempdirname, os.path.basename(file))
    shutil.copy(file, temp_fn)

    return temp_fn


def open_lib(lib_name):
    """
    The same library can only be loaded once;
    more precisely as long as you try to load a
    library with the same path it gets only loaded **once**
    in the process.

    see https://stackoverflow.com/questions/55312646/loading-two-dynamic-library-instances-in-python for details

    """

    try:
        if os.path.isfile(lib_name):
            lib_full_name = lib_name
        else:
            lib_path = os.environ["LD_LIBRARY_PATH"]
            lib_full_name = None
            for path in lib_path.split(":"):
                if os.path.isfile(os.path.join(path, lib_name)):
                    lib_full_name = os.path.join(path, lib_name)
                    break
        if not lib_full_name:
            raise OSError
        lib_temp_name = temp_position(lib_full_name)

        return ctypes.CDLL(lib_temp_name)
    except OSError as e:
        msg = f"""Could not find/load shared object file: {lib_name}
     Error was: {e}"""
        raise OSError(msg)
    finally:
        pass


class cmd_base_reg(ctypes.Structure):
    OP_NAME: str
    length: int

    cmd_id: int
    cmd_id_dep: int
    buf: memoryview
    # extra params
    subnet_id: int  # injected in decode_cmdgroup
    core_id: int

    def __repr__(self):
        return str(dict(self))

    def __iter__(self):
        for field in self._fields_:
            yield (field[0], getattr(self, field[0]))

    def __getitem__(self, key):
        return getattr(self, key)

    def __setitem__(self, key, value):
        setattr(self, key, value)

    @classmethod
    def from_values(cls, values: List[int]) -> "cmd_base_reg":
        res = cls()
        assert len(values) == len(cls._fields_), f"{len(values)} != {len(cls._fields_)}"
        for (key, *_), val in zip(cls._fields_, values):
            setattr(res, key, val)
        return res


class DMA_tensor_0x000__reg(cmd_base_reg):
    OP_NAME = "DMA_tensor（0x000）"
    _fields_ = [
        ("intr_en", ctypes.c_uint64, 1),
        ("stride_enable", ctypes.c_uint64, 1),
        ("nchw_copy", ctypes.c_uint64, 1),
        ("cmd_short", ctypes.c_uint64, 1),
        ("decompress_enable", ctypes.c_uint64, 1),
        ("cmd_id_en", ctypes.c_uint64, 4),
        ("cmd_id", ctypes.c_uint64, 20),
        ("Reserved", ctypes.c_uint64, 3),
        ("cmd_type", ctypes.c_uint64, 4),
        ("cmd_special_function", ctypes.c_uint64, 3),
        ("fill_constant_en", ctypes.c_uint64, 1),
        ("src_data_format", ctypes.c_uint64, 3),
        ("reserved", ctypes.c_uint64, 21),
        ("cmd_id_dep", ctypes.c_uint64, 20),
        ("reserved", ctypes.c_uint64, 12),
        ("constant_value", ctypes.c_uint64, 32),
        ("src_nstride", ctypes.c_uint64, 32),
        ("src_cstride", ctypes.c_uint64, 32),
        ("src_hstride", ctypes.c_uint64, 32),
        ("src_wstride", ctypes.c_uint64, 32),
        ("dst_nstride", ctypes.c_uint64, 32),
        ("dst_cstride", ctypes.c_uint64, 32),
        ("dst_hstride", ctypes.c_uint64, 32),
        ("dst_wstride", ctypes.c_uint64, 32),
        ("src_nsize", ctypes.c_uint64, 16),
        ("src_csize", ctypes.c_uint64, 16),
        ("src_hsize", ctypes.c_uint64, 16),
        ("src_wsize", ctypes.c_uint64, 16),
        ("dst_nsize", ctypes.c_uint64, 16),
        ("dst_csize", ctypes.c_uint64, 16),
        ("dst_hsize", ctypes.c_uint64, 16),
        ("dst_wsize", ctypes.c_uint64, 16),
        ("src_start_addr_l32", ctypes.c_uint64, 32),
        ("src_start_addr_h8", ctypes.c_uint64, 8),
        ("reserved", ctypes.c_uint64, 24),
        ("dst_start_addr_l32", ctypes.c_uint64, 32),
        ("dst_start_addr_h8", ctypes.c_uint64, 8),
        ("reserved", ctypes.c_uint64, 24),
        ("Reserved", ctypes.c_uint64, 32),
        ("Reserved", ctypes.c_uint64, 32),
        ("localmem_mask_l32", ctypes.c_uint64, 32),
        ("localmem_mask_h32", ctypes.c_uint64, 32),
    ]

    intr_en: int
    stride_enable: int
    nchw_copy: int
    cmd_short: int
    decompress_enable: int
    cmd_id_en: int
    cmd_id: int
    Reserved: int
    cmd_type: int
    cmd_special_function: int
    fill_constant_en: int
    src_data_format: int
    reserved: int
    cmd_id_dep: int
    reserved: int
    constant_value: int
    src_nstride: int
    src_cstride: int
    src_hstride: int
    src_wstride: int
    dst_nstride: int
    dst_cstride: int
    dst_hstride: int
    dst_wstride: int
    src_nsize: int
    src_csize: int
    src_hsize: int
    src_wsize: int
    dst_nsize: int
    dst_csize: int
    dst_hsize: int
    dst_wsize: int
    src_start_addr_l32: int
    src_start_addr_h8: int
    reserved: int
    dst_start_addr_l32: int
    dst_start_addr_h8: int
    reserved: int
    Reserved: int
    Reserved: int
    localmem_mask_l32: int
    localmem_mask_h32: int

    length: int = 768


class sCONV_reg(cmd_base_reg):
    OP_NAME = "sCONV"
    _fields_ = [
        ("cmd_short", ctypes.c_uint64, 1),
        ("cmd_id", ctypes.c_uint64, 20),
        ("cmd_id_dep", ctypes.c_uint64, 20),
        ("tsk_typ", ctypes.c_uint64, 4),
        ("tsk_eu_typ", ctypes.c_uint64, 5),
        ("opd0_sign", ctypes.c_uint64, 1),
        ("opd1_sign", ctypes.c_uint64, 1),
        ("opd2_sign", ctypes.c_uint64, 1),
        ("pad_mode", ctypes.c_uint64, 2),
        ("cmd_id_en", ctypes.c_uint64, 4),
        ("pwr_step", ctypes.c_uint64, 4),
        ("res_add", ctypes.c_uint64, 1),
        ("kernel_rotate", ctypes.c_uint64, 1),
        ("res0_prec", ctypes.c_uint64, 3),
        ("opd0_prec", ctypes.c_uint64, 3),
        ("opd1_const", ctypes.c_uint64, 1),
        ("opd2_const", ctypes.c_uint64, 1),
        ("opd0_up_pad", ctypes.c_uint64, 4),
        ("opd0_dn_pad", ctypes.c_uint64, 4),
        ("opd0_lf_pad", ctypes.c_uint64, 4),
        ("opd0_rt_pad", ctypes.c_uint64, 4),
        ("res_op_x_str", ctypes.c_uint64, 4),
        ("res_op_y_str", ctypes.c_uint64, 4),
        ("opd3_const", ctypes.c_uint64, 1),
        ("opd0_str", ctypes.c_uint64, 3),
        ("res0_addr", ctypes.c_uint64, 26),
        ("intr_en", ctypes.c_uint64, 1),
        ("opd0_x_ins0", ctypes.c_uint64, 4),
        ("opd0_y_ins0", ctypes.c_uint64, 4),
        ("opd1_x_ins0", ctypes.c_uint64, 4),
        ("opd1_y_ins0", ctypes.c_uint64, 4),
        ("opd0_n_str", ctypes.c_uint64, 16),
        ("res0_n", ctypes.c_uint64, 16),
        ("res0_c", ctypes.c_uint64, 16),
        ("res0_h", ctypes.c_uint64, 16),
        ("res0_w", ctypes.c_uint64, 16),
        ("opd0_c", ctypes.c_uint64, 16),
        ("opd0_h", ctypes.c_uint64, 16),
        ("opd0_w", ctypes.c_uint64, 16),
        ("opd1_h", ctypes.c_uint64, 16),
        ("opd1_w", ctypes.c_uint64, 16),
        ("opd0_c_str", ctypes.c_uint64, 16),
        ("opd0_addr", ctypes.c_uint64, 32),
        ("opd1_addr", ctypes.c_uint64, 32),
        ("opd2_addr", ctypes.c_uint64, 32),
        ("res1_addr", ctypes.c_uint64, 32),
        ("opd3_addr", ctypes.c_uint64, 32),
        ("opd0_h_str", ctypes.c_uint64, 16),
        ("opd0_w_str", ctypes.c_uint64, 16),
    ]

    cmd_short: int
    cmd_id: int
    cmd_id_dep: int
    tsk_typ: int
    tsk_eu_typ: int
    opd0_sign: int
    opd1_sign: int
    opd2_sign: int
    pad_mode: int
    cmd_id_en: int
    pwr_step: int
    res_add: int
    kernel_rotate: int
    res0_prec: int
    opd0_prec: int
    opd1_const: int
    opd2_const: int
    opd0_up_pad: int
    opd0_dn_pad: int
    opd0_lf_pad: int
    opd0_rt_pad: int
    res_op_x_str: int
    res_op_y_str: int
    opd3_const: int
    opd0_str: int
    res0_addr: int
    intr_en: int
    opd0_x_ins0: int
    opd0_y_ins0: int
    opd1_x_ins0: int
    opd1_y_ins0: int
    opd0_n_str: int
    res0_n: int
    res0_c: int
    res0_h: int
    res0_w: int
    opd0_c: int
    opd0_h: int
    opd0_w: int
    opd1_h: int
    opd1_w: int
    opd0_c_str: int
    opd0_addr: int
    opd1_addr: int
    opd2_addr: int
    res1_addr: int
    opd3_addr: int
    opd0_h_str: int
    opd0_w_str: int

    length: int = 512
