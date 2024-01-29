from heap import maxSortUp, maxSortDown, minSortDown, minSortUp


def get_mad(mediator_top, mediator_bottom, median, wind_len, half_wind_len_p, half_win_m):
    """
    Estimates Median Absolute Deviation
    :param mediator_top: mediator of the top order
    :param mediator_bottom: mediator of the bottom order
    :param median: median value
    :param wind_len: length of the window
    :param half_wind_len_p: 1 order above median
    :param half_win_m: 1 order below median
    :return: Median Absolute Deviation
    """
    median_2 = median * 2
    while True:
        top_order_value = mediator_top['data'][mediator_top['heap'][0]]
        bottom_order_value = mediator_bottom['data'][mediator_bottom['heap'][0]]
        # the condition for the choice of the top/bottom order value for the calculation of the MAD
        # is producing a larger deviation.
        mad_cond = top_order_value + bottom_order_value >= median_2
        if mad_cond:
            # value smaller than bottom order
            bottom_order_value_1 = mediator_bottom['data'][mediator_bottom['heap'][-1]]
            mad_cond_1 = median_2 < top_order_value + bottom_order_value_1
            # condition for no ability to reduce the order of the median-heaps
            if mediator_top['order'] == half_wind_len_p:
                mad = median - bottom_order_value_1 if mad_cond_1 else top_order_value - median
                return mad
            bottom_order_value_2 = mediator_bottom['data'][mediator_bottom['heap'][-2]]
            mad_cond_2 = median_2 < top_order_value + bottom_order_value_2
            # condition for reduction of the order of both median heaps
            if mad_cond_1 or mad_cond_2:
                # updating orders
                mediator_top['order'] -= 1
                mediator_bottom['order'] -= 1

                # sorting the median heaps to adapt to the new order
                minSortUp(mediator_bottom['data'], mediator_bottom['heap'], mediator_bottom['pos'],
                          wind_len - mediator_bottom['order'] - 1)
                maxSortDown(mediator_bottom['data'], mediator_bottom['heap'], mediator_bottom['pos'],
                            mediator_bottom['order'], -1)
                minSortUp(mediator_top['data'], mediator_top['heap'], mediator_top['pos'],
                          wind_len - mediator_top['order'] - 1)
                maxSortDown(mediator_top['data'], mediator_top['heap'], mediator_top['pos'],
                            mediator_top['order'], -1)
            else:
                # no need to modify the orders, reducing the max deviation
                mad = top_order_value - median
                return mad
        else:
            # value larger than top order
            top_order_value_1 = mediator_top['data'][mediator_top['heap'][1]]
            mad_cond_1 = top_order_value_1 + bottom_order_value < median_2
            if mediator_bottom['order'] == half_win_m:
                mad = top_order_value_1 - median if mad_cond_1 else median - bottom_order_value
                return mad
            top_order_value_2 = mediator_top['data'][mediator_top['heap'][2]]
            mad_cond_2 = top_order_value_2 + bottom_order_value < median_2
            # condition for elevation of the order of both median heaps
            if mad_cond_1 or mad_cond_2:
                # updating orders
                mediator_top['order'] += 1
                mediator_bottom['order'] += 1

                # sorting the median heaps to adapt to the new order
                maxSortUp(mediator_top['data'], mediator_top['heap'],
                          mediator_top['pos'], -mediator_top['order'])
                minSortDown(mediator_top['data'], mediator_top['heap'], mediator_top['pos'],
                            wind_len - mediator_top['order'] - 1, 1)
                maxSortUp(mediator_bottom['data'], mediator_bottom['heap'],
                          mediator_bottom['pos'], -mediator_bottom['order'])
                minSortDown(mediator_bottom['data'], mediator_bottom['heap'], mediator_bottom['pos'],
                            wind_len - mediator_bottom['order'] - 1, 1)
            else:
                # no need to modify the orders, reducing the max deviation
                mad = median - bottom_order_value
                return mad
