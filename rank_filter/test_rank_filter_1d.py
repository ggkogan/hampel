import numpy as np
from scipy import ndimage
from rank_filter_1d import rank_filter_1d
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
    x_test = (-1 + t) * dc_amp + np.sin(t * 2 * np.pi * freq)
    x_test += np.random.normal(scale=0.1, size=t.size)

    spikes = np.zeros(t.size)
    np.random.seed(0)
    spikes[::impulse_intervals] = np.random.normal(scale=3, size=spikes[::impulse_intervals].size)
    x_test += spikes

    n_tests = 21
    kernel_sizes = np.arange(7, 51)
    ratio = []
    configurations = []
    for kernel_size in kernel_sizes:
        for order in range(1, kernel_sizes[-1] - 1):
            if order >= kernel_size - 1:
                break
            configurations.append((kernel_size, order))

    for kernel_size, order in configurations:
        t = time.time()
        for a in range(n_tests):
            x_filt = rank_filter_1d(x_test, order, size=kernel_size)
        time_new = time.time() - t

        t = time.time()
        for a in range(n_tests):
            x_filt_ref = ndimage.rank_filter(x_test, order, size=kernel_size)
        time_current = time.time() - t
        ratio.append(time_current / time_new)

        if ratio[-1] < 1:
            print(f"ratio = {ratio[-1]:.2f}, kernel size={kernel_size}, order={order}")

    order_mat, kernel_size_mat = np.meshgrid(
        np.arange(configurations[0][-1], configurations[-1][-1] + 1),
        np.arange(configurations[0][0], configurations[-1][-1] + 1)
    )
    ratio_mat = np.full_like(order_mat, fill_value=np.nan, dtype=np.float64)

    for (kernel_size, order), ratio_val in zip(configurations, ratio):
        ratio_mat[np.equal(order_mat, order) & np.equal(kernel_size_mat, kernel_size)] = ratio_val

    fig, ax = plt.subplots(1, 1)
    c = ax.pcolor(order_mat, kernel_size_mat, ratio_mat, shading='auto', cmap='plasma')
    ax.set_xlabel('Order')
    ax.set_ylabel('Kernel size')
    ax.set_title(f'Running time ratio [ndimage.rank_filter/suggested]\nbased on mean value of {n_tests} runs')
    fig.colorbar(c, ax=ax)
    if n_tests >= 21:
        plt.savefig(pathlib.Path(__file__).parent / 'time_ratio_rank_filter.png')

    testing_cases = []
    for kernel_size in [20, 21]:
        for order in range(1, kernel_size - 1):
            for mode in ['constant', 'nearest', 'mirror', 'wrap']:
                for origin in range(-(kernel_size//2), (kernel_size + 1)//2):
                    testing_cases.append((kernel_size, order, mode, origin))
    for kernel_size, order, mode, origin in testing_cases:
        x_filt_ref = ndimage.rank_filter(x_test, order, size=kernel_size, mode=mode, cval=0, origin=origin)
        x_filt = rank_filter_1d(x_test, order, kernel_size, mode=mode, cval=0, origin=origin)
        if not np.all(x_filt_ref == x_filt):
            print(f"failed {mode} mode for kernel size of {kernel_size}, order of {order}, with origin {origin}")
            break
