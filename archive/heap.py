import numpy as np
"""
based on 2011 ashelly.myopenid.com under <http:#www.opensource.org/licenses/mit-license>
I have slightly modified it to reduce the computation. Now it requires some initiation but it should run faster
and the code is simpler

"""


# *--- Helper Functions ---*/
def mmless(data, heap, i, k):
    return data[heap[i]] < data[heap[k]]


# swaps items i&k in heap, maintains indexes
def mmexchange(heap, pos, i, k):
    t = heap[i]
    heap[i] = heap[k]
    heap[k] = t
    pos[heap[i]] = i
    pos[heap[k]] = k
    return 1


# swaps items i&k if i<k  returns true if swapped
def mmCmpExch(data, heap, pos, i, k):
    return bool((mmless(data, heap, i, k) and mmexchange(heap, pos, i, k)))


# maintains min-heap property for all items below np.fix(i / 2).
def minSortDown(data, heap, pos, heapSize, i):
    while i <= heapSize:
        if 1 < i < heapSize and mmless(data, heap, i + 1, i):
            i += 1
        if not mmCmpExch(data, heap, pos, i, int(np.fix(i / 2))):
            break
        i *= 2


# maintains max-heap property for all items below np.fix(i / 2)2. (negative indexes)
def maxSortDown(data, heap, pos, heapSize, i):
    while i >= -heapSize:
        if -1 > i > -heapSize and mmless(data, heap, i, i - 1):
            i -= 1
        if not mmCmpExch(data, heap, pos, int(np.fix(i / 2)), i):
            break
        i *= 2


# maintains min-heap property for all items above i, including median
# returns true if median changed
def fminSortUp(data, heap, pos, i):
    while i > 0 and mmCmpExch(data, heap, pos, i, int(np.fix(i / 2))):
        i = int(np.fix(i / 2))
    return i == 0


# maintains max-heap property for all items above i, including median
# returns true if median changed
def maxSortUp(data, heap, pos, i):
    while i < 0 and mmCmpExch(data, heap, pos, int(np.fix(i / 2)), i):
        i = int(np.fix(i / 2))
    return i == 0


# creates new Mediator: to calculate `nItems` running median.
# mallocs single block of memory, caller must free.
def MediatorNew(wind_len, order):
    """
    creates new mediator
    :param wind_len: number of items in the window
    :param order: half of the window len (for median filter)
    :return:
    data: values
    heap: pointer from the data to the heap
    pos: pointer from the heap to the data
    index: pointer to the input value (circular buffer)
    """
    mediator = {}
    mediator['heap'] = np.empty(wind_len, dtype=int)
    mediator['heap'][-order:] = np.arange(-order, 0).astype(int)
    mediator['heap'][:wind_len - order] = np.arange(wind_len - order).astype(int)
    mediator['pos'] = mediator['heap'].copy()
    mediator['data'] = np.empty(wind_len)
    mediator['data'][-order:] = -np.inf
    mediator['data'][:wind_len - order] = np.inf
    mediator['idx'] = 0
    mediator['order'] = order
    return mediator


# Inserts item, maintains median in O(lg nItems)
def MediatorInsert(mediator, v):
    pos = mediator['pos']
    data = mediator['data']
    heap = mediator['heap']
    p = pos[mediator['idx']]
    old = data[mediator['idx']]
    order = mediator['order']

    data[mediator['idx']] = v

    mediator['idx'] += 1
    if mediator['idx'] == mediator['data'].size:
        mediator['idx'] = 0

    if p > 0:  # new item is in minHeap
        if old < v:
            minSortDown(data, heap, pos, data.size - order - 1, p * 2)
        elif minSortUp(data, heap, pos, p):
            maxSortDown(data, heap, pos, order, -1)
    elif p < 0:  # new item is in max-heap
        if v < old:
            maxSortDown(data, heap, pos, order, p * 2)
        elif maxSortUp(data, heap, pos, p):
            minSortDown(data, heap, pos, data.size - order - 1, 1)
    else:  # new item is at median
        maxSortDown(data, heap, pos, order, -1)
        minSortDown(data, heap, pos, data.size - order - 1, 1)
