//Copyright (c) 2011 ashelly.myopenid.com under <http://w...content-available-to-author-only...e.org/licenses/mit-license>
// Started working on https://ideone.com/8VVEa, I optimized by restriction of cases and proper initialization, 
//also adapted for rank filter rather than the original median filter. Allowed different boundary conditons. 
//Moved to C++ for polymorphism and added C interface for use in Python. 

#include <stdlib.h>
#include <stdio.h>
#include <cstdint>
 
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
 
//Inserts item, maintains rank in O(lg nItems)
template <typename T>
void MediatorInsert(T* data, Mediator* m, T v)
{
   int p = m->pos[m->idx];
   T old = data[m->idx];
   data[m->idx] = v;
   m->idx++;
   if(m->idx == m->N){m->idx = 0; }

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
 
template <typename T>
void rank_filter(T* in_arr, int rank, int arr_len, int win_len, T* out_arr, int mode, T cval, int origin)
{
   int i, arr_len_thresh, lim = (win_len - 1) / 2 - origin;
   int lim2 = arr_len - lim;
   Mediator* m = MediatorNew(win_len, rank);
   T* data = (T*)malloc(sizeof(T) * win_len);

   switch (mode)
   {
      case REFLECT:
      for (i=win_len - lim - 1; i > - 1; i--){MediatorInsert(data, m, in_arr[i]);}
      break;
      case CONSTANT:
      for (i=win_len - lim; i > 0; i--){MediatorInsert(data, m, cval);}
      break;
      case NEAREST:
      for (i=win_len - lim; i > 0; i--){MediatorInsert(data, m, in_arr[0]);}
      break;
      case MIRROR:
      for (i=win_len - lim; i > 0; i--){MediatorInsert(data, m, in_arr[i]);}
      break;
      case WRAP:
      for (i=arr_len - lim - 1 - 2 * origin; i < arr_len; i++){MediatorInsert(data, m, in_arr[i]);}
      break;
   }

   for (i=0; i < lim; i++){MediatorInsert(data, m, in_arr[i]);}
   for (i=lim; i < arr_len; i++)
   {
      MediatorInsert(data, m, in_arr[i]);
      out_arr[i - lim] = data[m->heap[0]];
   }
   switch (mode)
   {
      case REFLECT:
      arr_len_thresh = arr_len - 1;
      for (i=0; i < lim; i++)
      {
      MediatorInsert(data, m, in_arr[arr_len_thresh - i]);
      out_arr[lim2 + i] = data[m->heap[0]];
      }
      break;
      case CONSTANT:
      for (i=0; i < lim; i++)
      {
      MediatorInsert(data, m, cval);
      out_arr[lim2 + i] = data[m->heap[0]];
      }
      break;
      case NEAREST:
      arr_len_thresh = arr_len - 1;
      for (i=0; i < lim; i++)
      {
      MediatorInsert(data, m, in_arr[arr_len_thresh]);
      out_arr[lim2 + i] = data[m->heap[0]];
      }
      break;
      case MIRROR:
      arr_len_thresh = arr_len - 2;
      for (i=0; i < lim + 1; i++)
      {
      MediatorInsert(data, m, in_arr[arr_len_thresh - i]);
      out_arr[lim2 + i] = data[m->heap[0]];
      }
      break;
      case WRAP:
      for (i=0; i < win_len; i++){
      MediatorInsert(data, m, in_arr[i]);
      out_arr[lim2 + i] = data[m->heap[0]];
      }
      break;
   }

   m->heap -= rank;
   free(m->heap);
   m->heap = nullptr;
   free(m->pos);
   m->pos = nullptr;
   free(m);
   m = nullptr;
   free(data);
   data = nullptr;
}
/*
extern "C"
void rank_filter_longdouble(long double* in, long double* out, int arr_len, int win_len, int order, Mode mode, long double cval, int origin){
   rank_filter(in, out, arr_len, win_len, order, mode, cval, origin);
}
extern "C"
void rank_filter_double(double* in, double* out, int arr_len, int win_len, int order, Mode mode, double cval, int origin){
   rank_filter(in, out, arr_len, win_len, order, mode, cval, origin);
}
extern "C"
void rank_filter_float(float* in, float* out, int arr_len, int win_len, int order, Mode mode, float cval, int origin){
   rank_filter(in, out, arr_len, win_len, order, mode, cval, origin);
}
extern "C"
void rank_filter_ulonglong(unsigned long long* in, unsigned long long* out, int arr_len, int win_len, int order, Mode mode, unsigned long long cval, int origin){
   rank_filter(in, out, arr_len, win_len, order, mode, cval, origin);
}
extern "C"
void rank_filter_longlong(long long* in, long long* out, int arr_len, int win_len, int order, Mode mode, long long cval, int origin){
   rank_filter(in, out, arr_len, win_len, order, mode, cval, origin);
}
extern "C"
void rank_filter_int64(int64_t* in, int64_t* out, int arr_len, int win_len, int order, Mode mode, int64_t cval, int origin){
   rank_filter(in, out, arr_len, win_len, order, mode, cval, origin);
}
extern "C"
void rank_filter_uint64(uint64_t* in, uint64_t* out, int arr_len, int win_len, int order, Mode mode, uint64_t cval, int origin){
   rank_filter(in, out, arr_len, win_len, order, mode, cval, origin);
}
extern "C"
void rank_filter_int32(int32_t* in, int32_t* out, int arr_len, int win_len, int order, Mode mode, int32_t cval, int origin){
   rank_filter(in, out, arr_len, win_len, order, mode, cval, origin);
}
extern "C"
void rank_filter_uint32(uint32_t* in, uint32_t* out, int arr_len, int win_len, int order, Mode mode, uint32_t cval, int origin){
   rank_filter(in, out, arr_len, win_len, order, mode, cval, origin);
}
extern "C"
void rank_filter_int(int* in, int* out, int arr_len, int win_len, int order, Mode mode, int cval, int origin){
   rank_filter(in, out, arr_len, win_len, order, mode, cval, origin);
}
extern "C"
void rank_filter_uint16(uint16_t* in, uint16_t* out, int arr_len, int win_len, int order, Mode mode, uint16_t cval, int origin){
   rank_filter(in, out, arr_len, win_len, order, mode, cval, origin);
}
extern "C"
void rank_filter_int16(int16_t* in, int16_t* out, int arr_len, int win_len, int order, Mode mode, int16_t cval, int origin){
   rank_filter(in, out, arr_len, win_len, order, mode, cval, origin);
}
extern "C"
void rank_filter_uint8(uint8_t* in, uint8_t* out, int arr_len, int win_len, int order, Mode mode, uint8_t cval, int origin){
   rank_filter(in, out, arr_len, win_len, order, mode, cval, origin);
}
extern "C"
void rank_filter_int8(int8_t* in, int8_t* out, int arr_len, int win_len, int order, Mode mode, int8_t cval, int origin){
   rank_filter(in, out, arr_len, win_len, order, mode, cval, origin);
}
extern "C"
void rank_filter_bool(bool* in, bool* out, int arr_len, int win_len, int order, Mode mode, bool cval, int origin){
   rank_filter(in, out, arr_len, win_len, order, mode, cval, origin);
}

int main()
{
   int arr_len = 10;
   int win_len = 4;
   int order = 1;
   Mode mode = REFLECT;
   int origin = 0;
   int64_t cval = 0;
   int64_t in[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
   int64_t out[10] = {0};
   for (size_t i = 0; i < 10; i++){
      printf("%ld ", i);
      rank_filter_int64(in_arr, out_arr, arr_len, win_len, order, mode, cval, origin);
   }
   return 0;
}
*/

