/*
Started working on https://ideone.com/8VVEa (Copyright (c) 2011 ashelly.myopenid.com
under <http://www.opensource.org/licenses/mit-license>),
I optimized by restriction of cases and proper initialization,
also adapted for rank filter rather than the original median filter.
Allowed different boundary conditions.
Moved to C++ for polymorphism and added C-Numpy API.
*/

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include "Python.h"
#include "numpy/arrayobject.h"

#include <stdlib.h>
#include <stdio.h>


struct Mediator//this is used for rank keeping
{
   int*  pos;   //index into `heap` for each value
   int*  heap;  //max/rank/min heap holding indexes into `data`.
   int   N;     //allocated size.
   int   idx;   //position in circular queue
   int   minCt; //count of items in min heap
   int   maxCt; //count of items in max heap
};

typedef enum {
   NEAREST = 0,
   WRAP = 1,
   REFLECT = 2,
   MIRROR = 3,
   CONSTANT = 4,
} Mode;

/*--- Helper Functions ---*/

Template <typename T>
inline T filter_mad(T v, T median, T mad, T scale)
{
   if abs(v - median) > scale * mad
   {
      return median;
   }
   else
   {
      return v;
   }
}

//returns 1 if heap[i] < heap[j]
template <typename T>
inline int mmless(T* data, Mediator* m, int i, int j)
{
   return (data[m->heap[i]] < data[m->heap[j]]);
}

//swaps items i&j in heap, maintains indexes
int mmexchange(Mediator* m, int i, int j)
{
   int t = m->heap[i];
   m->heap[i] = m->heap[j];
   m->heap[j] = t;
   m->pos[m->heap[i]] = i;
   m->pos[m->heap[j]] = j;
   return 1;
}

//swaps items i & j if i < j;  returns true if swapped
template <typename T>
inline int mmCmpExch(T* data, Mediator* m, int i, int j)
{
   return (mmless(data, m,i,j) && mmexchange(m,i,j));
}

//maintains minheap property for all items below i.
template <typename T>
void minSortDown(T* data, Mediator* m, int i)
{
   for (i*=2; i <= m->minCt; i*=2)
   {  if (i < m->minCt && mmless(data, m, i+1, i)) { ++i; }
      if (!mmCmpExch(data, m, i, i/2)) { break; }
   }
}

//maintains maxheap property for all items below i. (negative indexes)
template <typename T>
void maxSortDown(T* data, Mediator* m, int i)
{
   for (i*=2; i >= -m->maxCt; i*=2)
   {  if (i > -m->maxCt && mmless(data, m, i, i-1)) { --i;}
      if (!mmCmpExch(data, m, i/2, i)) { break; }
   }
}

//maintains minheap property for all items above i, including the rank
//returns true if rank changed
template <typename T>
inline int minSortUp(T* data, Mediator* m, int i)
{
   while (i>0 && mmCmpExch(data, m, i, i/2)) i/=2;
   return (i==0);
}

//maintains maxheap property for all items above i, including rank
//returns true if rank changed
template <typename T>
inline int maxSortUp(T* data, Mediator* m, int i)
{
   while (i<0 && mmCmpExch(data, m, i/2, i))  i/=2;
   return (i==0);
}

inline void promoteIndex(Mediator* m)
{
   m->idx++;
   if (m->idx == m->N) {m->idx = 0;}
}
/*--- Public Interface ---*/

//creates new Mediator: to calculate `nItems` running rank.
Mediator* MediatorNew(int nItems, int rank)
{
   Mediator* m =  (Mediator*)malloc(sizeof(Mediator));
   m->pos = (int*)malloc(sizeof(int) * nItems);
   m->heap = (int*)malloc(sizeof(int) * nItems);
   if ((m == nullptr)||(m->pos == nullptr)||(m->heap == nullptr)){printf("out of memory\n"); exit(1);}
   m->heap += rank; //points to rank
   m->N = nItems;
   m->idx = 0;
   m->minCt = nItems - rank - 1;
   m->maxCt = rank;
   while (nItems--)
   {
      m->pos[nItems]= nItems - rank;
      m->heap[m->pos[nItems]]=nItems;
   }
   return m;
}

void MediatorFree(Mediator* m)
{
   free(m->pos);
   m->pos = nullptr;
   m->heap -= m->maxCt;
   free(m->heap);
   m->heap = nullptr;
   free(m);
   m = nullptr;
}

//maintains rank in O(lg nItems)
template <typename T>
inline void sortHeap(T* data, Mediator* m, T v, int p, T old)
{
   if (p > 0) //new item is in minHeap
   {  if (v > old) { minSortDown(data, m, p); return; }
      if (minSortUp(data, m, p) && mmCmpExch(data, m, 0, -1)) { maxSortDown(data, m,-1); }
   }
   else if (p < 0) //new item is in maxheap
   {  if ( v < old) {maxSortDown(data, m, p); return; }
      if (maxSortUp(data, m, p) && mmCmpExch(data, m, 1, 0)) { minSortDown(data, m, 1); }
   }
   else //new item is at rank
   {  if (maxSortUp(data, m, -1)) { maxSortDown(data, m, -1); }
      if (minSortUp(data, m, 1)) { minSortDown(data, m, 1); }
   }
}

//Replaces item, maintains rank in O(lg nItems)
template <typename T>
void MediatorReplaceHampel(T* data, Mediator* m, Mediator* m_bottom, Mediator* m_top, T v)
{
   int p = m->pos[m->idx];
   int p_top = m_top->pos[m_top->idx];
   int p_bottom = m_bottom->pos[m_bottom->idx];
   
   T old = data[m->idx];
   data[m->idx] = v;

   promoteIndex(Mediator* m);
   promoteIndex(Mediator* m_top);
   promoteIndex(Mediator* m_bottom);

   sortHeap(T* data, Mediator* m, T v, int p, T old);
   sortHeap(T* data, Mediator* m_top, T v, int p_top, T old);
   sortHeap(T* data, Mediator* m_bottom, T v, int p_bottom, T old);
}

//Replaces item, maintains rank in O(lg nItems)
template <typename T>
void MediatorReplaceRank(T* data, Mediator* m, T v)
{
   int p = m->pos[m->idx];
   T old = data[m->idx];
   data[m->idx] = v;
   promoteIndex(Mediator* m);
   sortHeap(T* data, Mediator* m, T v, int p, T old);
}

// Mediator rank = rank - 1; in O(lg nItems)
void rank_minus_1(Mediator* m)
{
   m->heap[m->minCt + 1] = m->heap[-m->maxCt];
   m->pos[m->heap[m->minCt + 1]] = m->minCt + 1;
   m->minCt++;
   m->maxCt--;
   if (minSortUp(data, m, m->minCt + 1) && mmCmpExch(data, m, 0, -1)) {maxSortDown(data, m,-1);}
}

// Mediator rank = rank + 1; in O(lg nItems)
void rank_plus_1(Mediator* m)
{
   m->heap[-m->maxCt - 1] = m->heap[m->minCt + 1];
   m->pos[m->heap[-m->maxCt - 1]] = -m->maxCt - 1;
   m->minCt--;
   m->maxCt++;
   if (maxSortUp(data, m, -m->maxCt - 1) && mmCmpExch(data, m, 1, 0)) { minSortDown(data, m, 1); }
}

template <typename T>
T get_mad(Mediator* m_top, Mediator* m_bottom, T median, const int win_len, const int half_wind_len_p_1, const int half_win_len_m_1)
{
   T top_order_value, bottom_order_value, bottom_order_value_1, bottom_order_value_2;
   T median_2 = 2 * median;
   bool mad_cond_1, mad_cond_2;
   while true
   {
      top_order_value = data[m_top->heap[0]];
      bottom_order_value = data[m_bottom->heap[0]];
      // the condition for the choice of the top/bottom order value for the calculation
      // of the MAD is producing a larger deviation.
      if top_order_value + bottom_order_value >= median_2
      {
         bottom_order_value_1 = data[m_bottom->heap[-1]];
         mad_cond_1 = median_2 < top_order_value + bottom_order_value_1;
         //condition for no ability to reduce the order of the median-heaps
         if m_top->maxCt == half_wind_len_p
         {
            mad = mad_cond_1 ? median - bottom_order_value_1 : top_order_value - median;
            return mad;
         }
         bottom_order_value_2 = data[m_bottom->heap[-2]];
         mad_cond_2 = median_2 < top_order_value + bottom_order_value_2;
         //condition for reduction of the order of both median heaps
         if mad_cond_1 || mad_cond_2
         {
            rank_minus_1(m_top);
            rank_minus_1(m_bottom);
         }
         else
         {
            mad = top_order_value - median;
            return mad;
         }
      }
      else
      {
         // value larger than top order
         top_order_value_1 = data[m_top->heap[1]];
         mad_cond_1 = top_order_value_1 + bottom_order_value < median_2;
         if m_bottom->maxCt == half_win_m
         {
            mad = mad_cond_1 ? top_order_value_1 : median - bottom_order_value;
            return mad;
         }
         top_order_value_2 = data[m_top->heap[2]];
         mad_cond_2 = top_order_value_2 + bottom_order_value < median_2;
         // condition for elevation of the order of both median heaps
         if mad_cond_1 || mad_cond_2
         {
            rank_plus_1(m_top);
            rank_plus_1(m_bottom);
         }
         else
         {
            mad = median - bottom_order_value;
            return mad;
         }
      }
   }
}

template <typename T>
void _hampel_filter(T* in_arr, int arr_len, int win_len, T* median, T* mad, T* out_arr, T* scale)
{
   if (win_len % 2 == 0)
   {
      printf("Window length must be odd\n");
      exit(1);
   }
   const int rank = (win_len - 1) / 2;
   const int half_wind_len_p_1 = rank + 1;
   const int half_wind_len_m_1 = rank - 1;
   const int arr_len_thresh = arr_len - 1;
   const int lim = (win_len - 1) / 2;
   int rank_bottom = rank_top - rank;
   int rank_top = win_len - 2;
   int i;
   int i_shift = 0;
   Mediator* m = MediatorNew(win_len, rank);
   Mediator* m_top = MediatorNew(win_len, rank_top);
   Mediator* m_bottom = MediatorNew(win_len, rank_bottom);
   T* data = (T*)malloc(sizeof(T) * win_len);
   for (i=win_len - lim; i > 0; i--)
   {
      MediatorReplaceHampel(data, m, m_bottom, m_top, in_arr[0]);
   }
   for (i=0; i < lim; i++)
   {
      MediatorReplaceHampel(data, m, m_bottom, m_top, in_arr[i]);
   }

   for (i=lim; i < arr_len; i++)
   {
      MediatorReplaceHampel(data, m, m_bottom, m_top, in_arr[i]);
      i_shift++;
      median[i_shift] = data[m->heap[0]];
      mad[i_shift] = get_mad(m_top, m_bottom, median[i_shift], win_len, half_win_len_p, half_win_len_m);
      out_arr[i_shift] = filter_mad(in_arr[i_shift], median[i_shift], mad[i_shift], scale);
   }

   for (i=arr_len - lim; i < arr_len; i++)
   {
      MediatorReplaceHampel(data, m, m_bottom, m_top, in_arr[arr_len_thresh]);
      median[i] = data[m->heap[0]];
      mad[i] = get_mad(m_top, m_bottom, median[i], win_len, half_win_len_p, half_win_len_m);
      out_arr[i] = filter_mad(in_arr[i], median[i], mad[i], scale);
   }

   free(data);
   data = nullptr;
   MediatorFree(m);
   MediatorFree(m_top);
   MediatorFree(m_bottom);
}

template <typename T>
void _rank_filter(T* in_arr, int rank, int arr_len, int win_len, T* out_arr, int mode, T cval, int origin)
{
   int i, arr_len_thresh, lim = (win_len - 1) / 2 - origin;
   int lim2 = arr_len - lim;
   Mediator* m = MediatorNew(win_len, rank);
   T* data = (T*)malloc(sizeof(T) * win_len);

   switch (mode)
   {
      case REFLECT:
      for (i=win_len - lim - 1; i > - 1; i--){MediatorReplaceRank(data, m, in_arr[i]);}
      break;
      case CONSTANT:
      for (i=win_len - lim; i > 0; i--){MediatorReplaceRank(data, m, cval);}
      break;
      case NEAREST:
      for (i=win_len - lim; i > 0; i--){MediatorReplaceRank(data, m, in_arr[0]);}
      break;
      case MIRROR:
      for (i=win_len - lim; i > 0; i--){MediatorReplaceRank(data, m, in_arr[i]);}
      break;
      case WRAP:
      for (i=arr_len - lim - 1 - 2 * origin; i < arr_len; i++){MediatorReplaceRank(data, m, in_arr[i]);}
      break;
   }

   for (i=0; i < lim; i++){MediatorReplaceRank(data, m, in_arr[i]);}
   for (i=lim; i < arr_len; i++)
   {
      MediatorReplaceRank(data, m, in_arr[i]);
      out_arr[i - lim] = data[m->heap[0]];
   }
   switch (mode)
   {
      case REFLECT:
      arr_len_thresh = arr_len - 1;
      for (i=0; i < lim; i++)
      {
      MediatorReplaceRank(data, m, in_arr[arr_len_thresh - i]);
      out_arr[lim2 + i] = data[m->heap[0]];
      }
      break;
      case CONSTANT:
      for (i=0; i < lim; i++)
      {
      MediatorReplaceRank(data, m, cval);
      out_arr[lim2 + i] = data[m->heap[0]];
      }
      break;
      case NEAREST:
      arr_len_thresh = arr_len - 1;
      for (i=0; i < lim; i++)
      {
      MediatorReplaceRank(data, m, in_arr[arr_len_thresh]);
      out_arr[lim2 + i] = data[m->heap[0]];
      }
      break;
      case MIRROR:
      arr_len_thresh = arr_len - 2;
      for (i=0; i < lim + 1; i++)
      {
      MediatorReplaceRank(data, m, in_arr[arr_len_thresh - i]);
      out_arr[lim2 + i] = data[m->heap[0]];
      }
      break;
      case WRAP:
      for (i=0; i < win_len; i++){
      MediatorReplaceRank(data, m, in_arr[i]);
      out_arr[lim2 + i] = data[m->heap[0]];
      }
      break;
   }

   free(data);
   data = nullptr;
   MediatorFree(m);
}


// Python wrapper for hampel_filter
static PyObject* hampel_filter(PyObject* self, PyObject* args){
      PyObject *in_arr_obj, *out_arr_obj, *median_obj, *mad_obj, *scale_obj;
      int32_t win_len, arr_len, type;
      if (!PyArg_ParseTuple(args, "OiOOOO", &in_arr_obj, &win_len, &out_arr_obj, &median_obj, &mad_obj, &scale_obj))
      {
          return NULL;
      }
      PyArrayObject *in_arr = (PyArrayObject *)PyArray_FROM_OTF(in_arr_obj, NPY_NOTYPE, NPY_ARRAY_INOUT_ARRAY2);
      PyArrayObject *out_arr = (PyArrayObject *)PyArray_FROM_OTF(out_arr_obj, NPY_NOTYPE, NPY_ARRAY_INOUT_ARRAY2);
      PyArrayObject *median = (PyArrayObject *)PyArray_FROM_OTF(median_obj, NPY_NOTYPE, NPY_ARRAY_INOUT_ARRAY2);
      PyArrayObject *mad = (PyArrayObject *)PyArray_FROM_OTF(mad_obj, NPY_NOTYPE, NPY_ARRAY_INOUT_ARRAY2);
      if (in_arr == NULL || out_arr == NULL || median == NULL || mad == NULL)
      {
          return NULL;
      }
      type = PyArray_TYPE(in_arr);
      arr_len = PyArray_SIZE(in_arr);
      switch (type) { // the considered types are float, double, int64
          case NPY_FLOAT:
          {
              float *c_in_arr = (float *)PyArray_DATA(in_arr);
              float *c_out_arr = (float *)PyArray_DATA(out_arr);
              float *c_median = (float *)PyArray_DATA(median);
              float *c_mad = (float *)PyArray_DATA(mad);
              float c_scale = (float)PyFloat_AsDouble(scale_obj);
              _hampel_filter(c_in_arr, arr_len, win_len, c_median, c_mad, c_out_arr, c_scale);
              break;
          }
          case NPY_DOUBLE:
          {
              double *c_in_arr = (double *)PyArray_DATA(in_arr);
              double *c_out_arr = (double *)PyArray_DATA(out_arr);
              double *c_median = (double *)PyArray_DATA(median);
              double *c_mad = (double *)PyArray_DATA(mad);
              double c_scale = PyFloat_AsDouble(scale_obj);
              _hampel_filter(c_in_arr, arr_len, win_len, c_median, c_mad, c_out_arr, c_scale);
              break;
          }
          case NPY_INT64:
          {
              int64_t *c_in_arr = (int64_t *)PyArray_DATA(in_arr);
              int64_t *c_out_arr = (int64_t *)PyArray_DATA(out_arr);
              int64_t *c_median = (int64_t *)PyArray_DATA(median);
              int64_t *c_mad = (int64_t *)PyArray_DATA(mad);
              int64_t c_scale = PyLong_AsLongLong(scale_obj);
              _hampel_filter(c_in_arr, arr_len, win_len, c_median, c_mad, c_out_arr, c_scale);
              break;
          }
          default:
              PyErr_SetString(PyExc_TypeError, "Unsupported array type");
              break;
      }
      Py_DECREF(in_arr);
      Py_DECREF(out_arr);
      Py_DECREF(median);
      Py_DECREF(mad);
      Py_RETURN_NONE;
}

// Python wrapper for rank_filter
static PyObject* rank_filter(PyObject* self, PyObject* args)
{
    PyObject *in_arr_obj, *out_arr_obj, *cval_obj;
    int32_t rank, arr_len, win_len, mode, origin, type;
    if (!PyArg_ParseTuple(args, "OiiOiOi", &in_arr_obj, &rank, &win_len, &out_arr_obj, &mode, &cval_obj, &origin))
    {
        return NULL;
    }
    PyArrayObject *in_arr = (PyArrayObject *)PyArray_FROM_OTF(in_arr_obj, NPY_NOTYPE, NPY_ARRAY_INOUT_ARRAY2);
    PyArrayObject *out_arr = (PyArrayObject *)PyArray_FROM_OTF(out_arr_obj, NPY_NOTYPE, NPY_ARRAY_INOUT_ARRAY2);
    if (in_arr == NULL || out_arr == NULL)
    {
        return NULL;
    }
    arr_len = PyArray_SIZE(in_arr);
    type = PyArray_TYPE(in_arr);
    switch (type) { // the considered types are float, double, int64
        case NPY_FLOAT:
        {
            float *c_in_arr = (float *)PyArray_DATA(in_arr);
            float *c_out_arr = (float *)PyArray_DATA(out_arr);
            float cval = (float)PyFloat_AsDouble(cval_obj);
            _rank_filter(c_in_arr, rank, arr_len, win_len, c_out_arr, mode, cval, origin);
            break;
        }
        case NPY_DOUBLE:
        {
            double *c_in_arr = (double *)PyArray_DATA(in_arr);
            double *c_out_arr = (double *)PyArray_DATA(out_arr);
            double cval = PyFloat_AsDouble(cval_obj);
            _rank_filter(c_in_arr, rank, arr_len, win_len, c_out_arr, mode, cval, origin);
            break;
        }
        case NPY_INT64:
        {
            int64_t *c_in_arr = (int64_t *)PyArray_DATA(in_arr);
            int64_t *c_out_arr = (int64_t *)PyArray_DATA(out_arr);
            int64_t cval = PyLong_AsLongLong(cval_obj);
            _rank_filter(c_in_arr, rank, arr_len, win_len, c_out_arr, mode, cval, origin);
            break;
        }
        default:
            PyErr_SetString(PyExc_TypeError, "Unsupported array type");
            break;
    }
    Py_DECREF(in_arr);
    Py_DECREF(out_arr);
    Py_RETURN_NONE;
}

//define the module methods
static PyMethodDef myMethods[] = {
    {"rank_filter", rank_filter, METH_VARARGS, "1D rank filter"},
    {"hampel_filter", hampel_filter, METH_VARARGS, "Hampel filter"}
    {NULL, NULL, 0, NULL}};

//define the module
static struct PyModuleDef _robust_filters_1d = {
    PyModuleDef_HEAD_INIT,
    "_robust_filters_1d",
    "1D robust filters",
    -1,
    myMethods};

//init the module
PyMODINIT_FUNC PyInit__robust_filters_1d(void)
{
    import_array();
    return PyModule_Create(&_robust_filters_1d);
}

