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

dtype_fun_mapping = {
    'float128' : 'rank_filter_longdouble',
    'float64' : 'rank_filter_double',
    'float32' : 'rank_filter_float',
    'longdouble' : 'rank_filter_longdouble',
    'ulonglong': 'rank_filter_ulonglong',
    'longlong': 'rank_filter_long_long',
    'uint': 'rank_filter_unsigned_int',
    'int64': 'rank_filter_int64',
    'int32': 'rank_filter_int32',
    'uint64': 'rank_filter_uint64',
    'uint32': 'rank_filter_uint32',
    'uint16': 'rank_filter_uint16',
    'int16': 'rank_filter_int16',
    'uint8': 'rank_filter_uint8',
    'int8': 'rank_filter_int8',
    'bool_': 'rank_filter_bool',
}

dtype_numpy_c_mapping = {
    'float128': ctypes.c_longdouble,
    'float64': ctypes.c_double,
    'float32': ctypes.c_float,
    'longdouble': ctypes.c_longdouble,
    'ulonglong': ctypes.c_ulonglong,
    'longlong': ctypes.c_longlong,
    'uint': ctypes.c_uint,
    'uint64': ctypes.c_ulong,
    'int64': ctypes.c_long,
    'int32': ctypes.c_int,
    'uint32': ctypes.c_uint,
    'uint16': ctypes.c_ushort,
    'int16': ctypes.c_short,
    'uint8': ctypes.c_ubyte,
    'int8': ctypes.c_byte,
    'bool_': ctypes.c_bool,
}

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
def rank_filter_1d_(x: np.array, order:int, size: int, mode='reflect', cval=0, origin=0):
    # cc -O1 -fPIC -shared -o _rank_filter_1d_cpp.so _rank_filter_1d.cpp
    lib = ctypes.cdll.LoadLibrary(pathlib.Path(__file__).parent / "_rank_filter_1d_cpp.so")
    rank_filter = lib.__getattr__(dtype_fun_mapping[x.dtype.name])
    rank_filter.restype = None
    rank_filter.argtypes = [
        ndpointer(dtype_numpy_c_mapping[x.dtype.name], flags="C_CONTIGUOUS"),
        ndpointer(dtype_numpy_c_mapping[x.dtype.name], flags="C_CONTIGUOUS"),
        ctypes.c_size_t,
        ctypes.c_int,
        ctypes.c_int,
        ctypes.c_int,
        dtype_numpy_c_mapping[x.dtype.name],
        ctypes.c_int
    ]
    x_out = np.empty_like(x)
    mode_enum = Mode[mode]
    rank_filter(x, x_out, x.size, size, order, mode_enum.value, cval, origin)
    return x_out

if __name__ == 'main':
    pass
