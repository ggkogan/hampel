cimport cython

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

def rank_filter_1d(numeric_t[:] in_arr, int rank, int win_len, numeric_t[:] out_arr, str mode="reflect", numeric_t cval=0, int origin=0):
    rank_filter(&in_arr[0], rank, in_arr.shape[0], win_len, &out_arr[0], Mode[mode], cval, origin)
    return out_arr