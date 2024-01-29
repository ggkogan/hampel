import numpy as np
from heap import MediatorNew, MediatorInsert
from scipy import ndimage, stats
from mad import get_mad
import pandas as pd


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


if __name__ == "__main__":
    # demonstration of bit exact
    fs = 20000
    freq = 13.47
    impulse_intervals = 287
    time = 2
    dc_amp = 3.1
    t = np.arange(fs * time) / fs

    wind_len_test = 121
    np.random.seed(0)
    x_test = (-1 + t) * dc_amp + np.sin(t * 2 * np.pi * freq) + np.random.normal(scale=0.1, size=t.size)

    spikes = np.zeros(t.size)
    np.random.seed(0)
    spikes[::impulse_intervals] = np.random.normal(scale=3, size=spikes[::impulse_intervals].size)
    x_test += spikes

    x_med = ndimage.median_filter(x_test, wind_len_test)
    x_mad_local, x_med_local = rolling_mad(x_test, wind_len_test)
    x_mad = pd.Series(np.concatenate([x_test[:wind_len_test][::-1], x_test, x_test[-wind_len_test:][::-1]])).rolling(
        wind_len_test, center=True).apply(stats.median_abs_deviation)[wind_len_test:-wind_len_test]
    assert np.all(x_med_local == x_med)
    assert np.all(x_mad_local == x_mad)
