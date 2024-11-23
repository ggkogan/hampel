import numpy as np
from . import _robust_filters_1d


def hampel(input: np.ndarray, win_size: int, thresh=1):
    """
    TBD
    """
    if input.dtype in (np.int64, np.float64, np.float32):
        x = input
    elif input.dtype == np.float16:
        x = input.astype('float32')
    elif np.result_type(input, np.int64) == np.int64:
        x = input.astype('int64')
    elif input.dtype.kind in 'biu':
        # cast any other boolean, integer or unsigned type to int64
        x = input.astype('int64')
    else:
        raise RuntimeError('Unsupported array type')
    x_out = np.empty_like(x)
    median = np.empty_like(x)
    mad = np.empty_like(x)
    thresh = x.dtype.type(thresh)
    win_size = np.int64(win_size)
    _robust_filters_1d.hampel_filter(x, win_size, median, mad, x_out, thresh)
    if input.dtype not in (np.int64, np.float64, np.float32):
        x_out = x_out.astype(input.dtype)
        median = median.astype(input.dtype)
        mad = mad.astype(input.dtype)
    return x_out, median, mad
