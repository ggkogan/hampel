import numpy as np
from rank_filter_1d_cython import rank_filter_1d as _rank_filter_1d

def rank_filter_1d(x: np.array, rank:int, size: int, mode="reflect", cval=0, origin=0):
    cval = x.dtype.type(cval)
    x_out = np.empty_like(x)
    size = int(size)
    rank = int(rank)
    origin = int(origin)
    _rank_filter_1d(in_arr=x, rank=rank, win_len=size, out_arr=x_out, mode=mode, cval=cval, origin=origin)
    return x_out
