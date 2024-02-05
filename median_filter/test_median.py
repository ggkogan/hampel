import numpy as np
from scipy import ndimage
from median_filter import median_filter
import time
import pathlib

import matplotlib.pyplot as plt


if __name__ == "__main__":
    # demonstration of bit exact
    fs = 20000
    freq = 13.47
    impulse_intervals = 287
    time_ = 2
    dc_amp = 3.1
    t = np.arange(fs * time_) / fs

    np.random.seed(0)
    x_test = (-1 + t) * dc_amp + np.sin(t * 2 * np.pi * freq) + np.random.normal(scale=0.1, size=t.size)

    spikes = np.zeros(t.size)
    np.random.seed(0)
    spikes[::impulse_intervals] = np.random.normal(scale=3, size=spikes[::impulse_intervals].size)
    x_test += spikes

    n_tests = 101

    kernel_sizes = np.arange(1, 53, 2)
    ratio = np.empty(kernel_sizes.size)
    for i_size, kernel_size in enumerate(kernel_sizes):
        t = time.time()
        for a in range(n_tests):
            median = median_filter(x_test, kernel_size)
        time_new = time.time() - t

        t = time.time()
        for a in range(n_tests):
            median_ref = ndimage.median_filter(x_test, kernel_size, mode='reflect')
        time_current = time.time() - t
        if not np.all(median_ref == median):
            print(kernel_size)
        ratio[i_size] = time_current / time_new
    plt.figure()
    plt.grid()
    plt.scatter(kernel_sizes, ratio)
    plt.xticks(np.arange(0, (kernel_sizes[-1] // 5 + 1) * 5, 5))
    plt.xlabel('Kernel size')
    plt.title('Computation time ratio [ndimage.median_filter/suggested]')
    plt.axhline(1, linestyle='--', color='black')
    plt.savefig(pathlib.Path(__file__).parent / 'time_ratio.png')
