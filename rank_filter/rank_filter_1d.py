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
    # grid-mirror = 1
    # grid-constant = 2
    # grid-wrap = 5


def rank_filter_1d(x: np.array, order:int, size: int, mode='reflect', cval=0, origin=0):
    # cc -fPIC -shared -o _rank_filter_1d.so _rank_filter_1d.c
    lib = ctypes.cdll.LoadLibrary(pathlib.Path(__file__).parent / "_rank_filter_1d.so")
    rank_filter_c = lib.rank_filter
    rank_filter_c.restype = None
    rank_filter_c.argtypes = [
        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"),
        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"),
        ctypes.c_size_t,
        ctypes.c_int,
        ctypes.c_int,
        ctypes.c_int,
        ctypes.c_double,
        ctypes.c_int
    ]
    x_out = np.empty_like(x)
    mode_enum = Mode[mode]
    rank_filter_c(x, x_out, x.size, size, order, mode_enum.value, cval, origin)
    return x_out


def rank_filter_1d_double(x: np.array, order:int, size: int, mode='reflect', cval=0, origin=0):
    # cc -fPIC -shared -o _rank_filter_1d_cpp.so _rank_filter_1d.cpp
    lib = ctypes.cdll.LoadLibrary(pathlib.Path(__file__).parent / "_rank_filter_1d_cpp.so")
    rank_filter_double = lib.rank_filter_double
    rank_filter_double.restype = None
    rank_filter_double.argtypes = [
        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"),
        ndpointer(ctypes.c_double, flags="C_CONTIGUOUS"),
        ctypes.c_size_t,
        ctypes.c_int,
        ctypes.c_int,
        ctypes.c_int,
        ctypes.c_double,
        ctypes.c_int
    ]
    x_out = np.empty_like(x)
    mode_enum = Mode[mode]
    rank_filter_double(x, x_out, x.size, size, order, mode_enum.value, cval, origin)
    return x_out

def rank_filter_1d_float(x: np.array, order:int, size: int, mode='reflect', cval=0, origin=0):
    # cc -fPIC -shared -o _rank_filter_1d_cpp.so _rank_filter_1d.cpp
    lib = ctypes.cdll.LoadLibrary(pathlib.Path(__file__).parent / "_rank_filter_1d_cpp.so")
    rank_filter_float = lib.rank_filter_float
    rank_filter_float.restype = None
    rank_filter_float.argtypes = [
        ndpointer(ctypes.c_float, flags="C_CONTIGUOUS"),
        ndpointer(ctypes.c_float, flags="C_CONTIGUOUS"),
        ctypes.c_size_t,
        ctypes.c_int,
        ctypes.c_int,
        ctypes.c_int,
        ctypes.c_float,
        ctypes.c_int
    ]
    x_out = np.empty_like(x)
    mode_enum = Mode[mode]
    rank_filter_float(x, x_out, x.size, size, order, mode_enum.value, cval, origin)
    return x_out


if __name__ == 'main':
    pass
