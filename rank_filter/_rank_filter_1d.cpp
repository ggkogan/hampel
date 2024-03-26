//Copyright (c) 2011 ashelly.myopenid.com under <http://w...content-available-to-author-only...e.org/licenses/mit-license>
// I optimized by restriction of cases and proper initialization, also adapted for rank filter rather than the original
//median filter.

#include <stdlib.h>
#include <stdio.h>
#define inline
 
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
   REFLECT = 1,
   CONSTANT = 2,
   NEAREST = 3,
   MIRROR = 4,
   WRAP = 5
} Mode;
 
/*--- Helper Functions ---*/
 
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
   while (i>0 && mmCmpExch(data, m,i,i/2)) i/=2;
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
 
/*--- Public Interface ---*/
 
 
//creates new Mediator: to calculate `nItems` running rank.
//mallocs single block of memory, caller must free.
Mediator* MediatorNew(int nItems, int order)
{
   Mediator* m =  (Mediator*)malloc(sizeof(Mediator));
   m->pos = (int*)malloc(sizeof(int) * nItems);
   m->heap = (int*)malloc(sizeof(int) * nItems);
   m->heap += order + 1; //points to rank
   m->N = nItems;
   m->idx = 0;
   m->minCt = nItems - order - 1;
   m->maxCt = order;
   while (nItems--)
   {
      m->pos[nItems]= nItems - order;
      m->heap[m->pos[nItems]]=nItems;
   }
   return m;
}
 
 
//Inserts item, maintains rank in O(lg nItems)
template <typename T>
void MediatorInsert(T* data, Mediator* m, T v)
{
   int p = m->pos[m->idx];
   T old = data[m->idx];
   data[m->idx] = v;
   m->idx++;
   if(m->idx == m->N){m->idx = 0; }

   if (p > 0)         //new item is in minHeap
   {  if (v>old) { minSortDown(data, m, p); return; }
      if (minSortUp(data, m, p) && mmCmpExch(data, m, 0, -1)) { maxSortDown(data, m,-1); }
   }
   else if (p < 0)   //new item is in maxheap
   {  if (v<old) {maxSortDown(data, m, p); return; }
      if (maxSortUp(data, m, p) && mmCmpExch(data, m, 1, 0)) { minSortDown(data, m, 1); }
   }
   else //new item is at rank
   {  if (maxSortUp(data, m, -1)) { maxSortDown(data, m, -1); }
      if (minSortUp(data, m, 1)) { minSortDown(data, m, 1); }
   }
}
 
template <typename T>
void rank_filter(T* in, T* out, int arr_len, int win_len, int order, Mode mode, T cval, int origin)
{
   int i, arr_len_thresh, lim = (win_len - 1) / 2 - origin;
   int lim2 = arr_len - lim;
   T value;
   Mediator* m = MediatorNew(win_len, order);
   T* data = (T*)malloc(sizeof(T) * win_len);

   switch (mode)
   {
      case REFLECT:
      for (i=win_len - lim - 1; i > - 1; i--){MediatorInsert(data, m, in[i]);}
      break;
      case CONSTANT:
      for (i=win_len - lim; i > 0; i--){MediatorInsert(data, m, cval);}
      break;
      case NEAREST:
      for (i=win_len - lim; i > 0; i--){MediatorInsert(data, m, in[0]);}
      break;
      case MIRROR:
      for (i=win_len - lim; i > 0; i--){MediatorInsert(data, m, in[i]);}
      break;
      case WRAP:
      for (i=arr_len - lim - 1 - 2 * origin; i < arr_len; i++){MediatorInsert(data, m, in[i]);}
      break;
   }

   for (i=0; i < lim; i++){MediatorInsert(data, m, in[i]);}
   for (i=lim; i < arr_len; i++)
   {
      value = in[i];
      MediatorInsert(data, m, in[i]);
      out[i - lim] = data[m->heap[0]];
   }
   switch (mode)
   {
      case REFLECT:
      arr_len_thresh = arr_len - 1;
      for (i=0; i < lim; i++)
      {
      MediatorInsert(data, m, in[arr_len_thresh - i]);
      out[lim2 + i] = data[m->heap[0]];
      }
      break;
      case CONSTANT:
      for (i=0; i < lim; i++)
      {
      MediatorInsert(data, m, cval);
      out[lim2 + i] = data[m->heap[0]];
      }
      break;
      case NEAREST:
      arr_len_thresh = arr_len - 1;
      for (i=0; i < lim; i++)
      {
      MediatorInsert(data, m, in[arr_len_thresh]);
      out[lim2 + i] = data[m->heap[0]];
      }
      break;
      case MIRROR:
      arr_len_thresh = arr_len - 2;
      for (i=0; i < lim + 1; i++)
      {
      MediatorInsert(data, m, in[arr_len_thresh - i]);
      out[lim2 + i] = data[m->heap[0]];
      }
      break;
      case WRAP:
      for (i=0; i < win_len; i++){
      MediatorInsert(data, m, in[i]);
      out[lim2 + i] = data[m->heap[0]];
      }
      break;
   }

   m->heap -= order + 1;
   free(m->heap);
   free(m->pos);
   free(m);
   free(data);
}

extern "C"
void rank_filter_double(double* in, double* out, int arr_len, int win_len, int order, Mode mode, double cval, int origin){
   rank_filter(in, out, arr_len, win_len, order, mode, cval, origin);
}
extern "C"
void rank_filter_float(float* in, float* out, int arr_len, int win_len, int order, Mode mode, float cval, int origin){
   rank_filter(in, out, arr_len, win_len, order, mode, cval, origin);
}
int main()
{
   double in_double[] = {
      2.36856227, -3.05559757, -2.99335285, -2.86275085, -2.89569811, 
      -3.17579574, -2.97867353, -3.08443298, -3.07523455, -3.01946883
      };
   float in_float[] = {
      2.36856227, -3.05559757, -2.99335285, -2.86275085, -2.89569811,
      -3.17579574, -2.97867353, -3.08443298, -3.07523455, -3.01946883
      };
   int16_t in_int[] = {
      236, -305, -299, -286, -289,
      -317, -297, -308, -307, -301
      };
   double out_double[10];
   float out_float[10];
   int16_t out_int[10];
   int arr_len = 10;
   int win_len = 5;
   int order = 1;
   int origin = 1;
   Mode mode = WRAP;
   double cval = 0;
   rank_filter_double(in_double, out_double, arr_len, win_len, order, mode, cval, origin);
   //rank_filter(in_int, out_int, arr_len, win_len, order, mode, (int)cval, origin);
   //for (int i = 0; i < arr_len; i++){printf("%d\n", out_int[i]);}
   return 1;
}