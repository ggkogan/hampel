import numpy as np
from scipy import ndimage, stats
from rolling_mad_cy import rolling_mad
import pandas as pd
import time


if __name__ == "__main__":
    # demonstration of bit exact
    fs = 20000
    freq = 13.47
    impulse_intervals = 287
    time_ = 2
    dc_amp = 3.1
    t = np.arange(fs * time_) / fs

    wind_len_test = 21
    np.random.seed(0)
    x_test = (-1 + t) * dc_amp + np.sin(t * 2 * np.pi * freq) + np.random.normal(scale=0.1, size=t.size)

    ndimage.median_filter(x_test, wind_len_test)

    spikes = np.zeros(t.size)
    np.random.seed(0)
    spikes[::impulse_intervals] = np.random.normal(scale=3, size=spikes[::impulse_intervals].size)
    x_test += spikes

    x_med = ndimage.median_filter(x_test, wind_len_test)
    t1 = time.time()
    x_mad_local, x_med_local = rolling_mad(x_test, wind_len_test)
    print(time.time() - t1)
    x_mad = pd.Series(np.concatenate([x_test[:wind_len_test][::-1], x_test, x_test[-wind_len_test:][::-1]])).rolling(
        wind_len_test, center=True).apply(stats.median_abs_deviation)[wind_len_test:-wind_len_test]
    assert np.all(x_med_local == x_med)
    assert np.all(x_mad_local == x_mad)
