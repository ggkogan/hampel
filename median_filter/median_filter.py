import ctypes
import numpy as np
from numpy.ctypeslib import ndpointer
import pathlib
import enum


class Mode(enum.Enum):
    reflect = 1
    constant = 2
    nearest = 3
    mirror = 4
    wrap = 5


def median_filter(x: np.array, kernel_size: int, mode='reflect', cval=0):
    # cc -fPIC -shared -o _median_filter.so _median_filter.c
    lib = ctypes.cdll.LoadLibrary(pathlib.Path(__file__).parent / "_median_filter.so")
    median_filter_c = lib.median_filter
    median_filter_c.restype = None
    median_filter_c.argtypes = [
        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"),
        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"),
        ctypes.c_size_t,
        ctypes.c_int,
        ctypes.c_int,
        ctypes.c_int,
        ctypes.c_double
    ]
    x_out = np.empty_like(x)
    mode_enum = Mode[mode]
    median_filter_c(x, x_out, x.size, kernel_size, kernel_size // 2 + 1, mode_enum.value, cval)
    return x_out


if __name__ == 'main':
    pass
