import ctypes
import numpy as np
from numpy.ctypeslib import ndpointer


def median_filter(x: np.array, kernel_size: int):
    # cc -fPIC -shared -o median_filter.so _median_filter.c
    lib = ctypes.cdll.LoadLibrary("_median_filter.so")
    median_filter_c = lib.median_filter
    median_filter_c.restype = None
    #float* in, float* out, int win_len, int arr_len, int order
    median_filter_c.argtypes = [
        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"),
        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"),
        ctypes.c_size_t,
        ctypes.c_int,
        ctypes.c_int
    ]
    x_out = np.empty_like(x)
    median_filter_c(x, x_out, x.size, kernel_size, kernel_size // 2 + 1)
    return x_out


if __name__ == 'main':
    pass
