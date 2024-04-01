cimport cython
import numpy as np

# this should have been enum but special keys are not supported
Mode = {
    'reflect': 1,
    'constant': 2,
    'nearest': 3,
    'mirror': 4,
    'wrap': 5,
   'grid-mirror': 1,
    'grid-constant': 2,
    'grid-wrap': 5
}

ctypedef fused int_t:
    cython.int
    cython.uint
    cython.ulong
    cython.long
    cython.longlong
    cython.ulonglong
    cython.short
    cython.ushort

ctypedef fused numeric_t:
    cython.float
    cython.double
    cython.int
    cython.uint
    cython.ulong
    cython.long
    cython.longlong
    cython.ulonglong
    cython.char
    cython.uchar
    cython.short
    cython.ushort
    cython.longdouble

cdef extern from "_rank_filter_1d.cpp" nogil:
    void rank_filter[T](T* in_arr, int rank, int arr_len, int win_len, T* out_arr, int mode, T cval, int origin)

def rank_filter_1d_cpp_api(numeric_t[:] in_arr, int rank, int win_len, numeric_t[:] out_arr, str mode="reflect", numeric_t cval=0, int origin=0):
    rank_filter(&in_arr[0], rank, in_arr.shape[0], win_len, &out_arr[0], Mode[mode], cval, origin)
    return out_arr

def _rank_filter_1d(x: np.array, rank: int_t, size: int_t, mode="reflect", cval=0, origin: int_t=0):
    cval = x.dtype.type(cval)
    x_out = np.empty_like(x)
    rank_filter_1d_cpp_api(in_arr=x, rank=rank, win_len=size, out_arr=x_out, mode=mode, cval=cval, origin=origin)
    return x_out