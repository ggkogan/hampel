import numpy as np
from _rank_filter_1d import rank_filter

# this should have been enum but special keys are not supported
Mode = {
    'reflect': 2,
    'constant': 4,
    'nearest': 0,
    'mirror': 3,
    'wrap': 1,
}

def _rank_filter_1d_warp(x: np.array, rank: int, size: int, mode="reflect", cval=0,
                         origin: int=0):
    type = x.dtype.name
    flag = type not in ['int64', 'float64', 'float32']
    if flag:
        x = x.astype('int64')
    cval = x.dtype.type(cval)
    x_out = np.empty_like(x)
    rank_filter(x, rank, x.size, size, x_out, Mode[mode], cval, origin)
    if flag:
        x_out = x_out.astype(type)
    return x_out