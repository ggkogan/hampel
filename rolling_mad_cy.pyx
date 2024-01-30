import numpy as np
from heap_cy import MediatorNew, MediatorInsert
from mad_cy import get_mad


def rolling_mad(x: np.array, wind_len: int):
    """
    provides the rolling median absolute deviation using structures similar to median heaps with adaptive center
    location
    :param x: signal
    :param wind_len: length of the window (assumed to be odd)
    :return: median absolute deviation and rolling median
    """
    if np.mod(wind_len, 2) == 0:
        return None
    # initiation of orders
    order_median = wind_len // 2
    order_top = wind_len - 2
    order_bottom = order_top - order_median
    # creation of the median heaps
    mediator = MediatorNew(wind_len, order=order_median)
    mediator_top = MediatorNew(wind_len, order_top)
    mediator_bottom = MediatorNew(wind_len, order_bottom)
    # padding the edges to maintain the edge conditions - can be later optimized
    x_input = np.concatenate([x[:wind_len][::-1], x, x[-wind_len:][::-1]])

    median = np.empty_like(x_input)
    mad = np.empty_like(x_input)

    half_wind_len_p = (wind_len + 1) // 2
    half_wind_len_m = (wind_len - 3) // 2
    for i in range(x_input.size):
        # insert new values into the median-like heaps
        MediatorInsert(mediator, x_input[i])
        MediatorInsert(mediator_top, x_input[i])
        MediatorInsert(mediator_bottom, x_input[i])
        # main calculations
        median[i] = mediator['data'][mediator['heap'][0]]
        mad[i] = get_mad(mediator_top, mediator_bottom, median[i], wind_len, half_wind_len_p, half_wind_len_m)

    # chopping to get centered window with a window length delay - consistent with previous implementations
    median = median[wind_len * 3 // 2:-wind_len // 2]
    mad = mad[wind_len * 3 // 2:-wind_len // 2]

    return mad, median