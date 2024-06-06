#include <stdlib.h>
#include <stdio.h>
#include <Kernel/math.h>
#include <iostream>
#include <fstream>
using namespace std;

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
   while (i>0 && mmCmpExch(data, m, i, i/2)) {i/=2;};
   return (i==0);
}

//maintains maxheap property for all items above i, including rank
//returns true if rank changed
template <typename T>
inline int maxSortUp(T* data, Mediator* m, int i)
{
   while (i<0 && mmCmpExch(data, m, i/2, i)) {i/=2;};
   return (i==0);
}

inline void promoteIndex(Mediator* m)
{
   m->idx++;
   if (m->idx == m->N)
   {
        m->idx = 0;
   }
}
/*--- Public Interface ---*/

//creates new Mediator: to calculate `nItems` running rank.
Mediator* MediatorNew(int nItems, int rank, bool buffer = false)
{
   Mediator* m =  (Mediator*)malloc(sizeof(Mediator));
   int size = buffer ? 2 * nItems - 1 : nItems;
   m->pos = (int*)malloc(sizeof(int) * size);
   m->heap = (int*)malloc(sizeof(int) * size);
   if ((m == nullptr)||(m->pos == nullptr)||(m->heap == nullptr)){printf("out of memory\n"); exit(1);}
   m->heap += buffer ? rank + (nItems - 1) / 2 :rank; //points to rank
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

   // can use the same index for all mediators - later optimization
   promoteIndex(m);
   promoteIndex(m_top);
   promoteIndex(m_bottom);

   sortHeap(data, m, v, p, old);
   sortHeap(data, m_top, v, p_top, old);
   sortHeap(data, m_bottom, v, p_bottom, old);
}

//Replaces item, maintains rank in O(lg nItems)
template <typename T>
void MediatorReplaceRank(T* data, Mediator* m, T v)
{
   int p = m->pos[m->idx];
   T old = data[m->idx];
   data[m->idx] = v;
   promoteIndex(m);
   sortHeap(data, m, v, p, old);
}

// Mediator rank = rank - 1; in O(lg nItems)
template <typename T>
void rank_minus_1(Mediator* m, T* data)
{
   m->minCt++;
   m->maxCt--;
   m->heap[m->minCt] = m->heap[-m->maxCt - 1];
   m->pos[m->heap[m->minCt]] = m->pos[m->heap[-m->maxCt - 1]];
   if (minSortUp(data, m, m->minCt) && mmCmpExch(data, m, 0, -1)) { maxSortDown(data, m, -1); }
}

template <typename T>
void print_mediator(Mediator* m, T* data)
{
   for (int temp = -m->maxCt; temp <= m->minCt; temp++)
   {
      printf("%d: %f \n", temp, data[m->heap[temp]]);
   }
   fflush(stdout);
}

// Mediator rank = rank + 1; in O(lg nItems)
template <typename T>
void rank_plus_1(Mediator* m, T* data)
{
   m->minCt--;
   m->maxCt++;
   m->heap[-m->maxCt] = m->heap[m->minCt + 1];
   m->pos[m->heap[-m->maxCt]] = m->pos[m->heap[m->minCt + 1]];
   if (maxSortUp(data, m, -m->maxCt) && mmCmpExch(data, m, 1, 0)) { minSortDown(data, m, 1); }
}

template <typename T>
T get_mad(Mediator* m_top, Mediator* m_bottom, T median, T* data, const int win_len){
   T top_order_value = data[m_top->heap[0]];
   T bottom_order_value = data[m_bottom->heap[0]];
   T mad, top_diff, bottom_diff;
   while (top_order_value - median > median - data[m_bottom->heap[-1]])
   {
      if (m_bottom->maxCt == 1)
      {
         bottom_diff = median - data[m_bottom->heap[-1]];
         top_diff = data[m_top->heap[-1]] - median;
         mad = std::max(bottom_diff, top_diff);
         return mad;
      }
      rank_minus_1(m_top, data);
      rank_minus_1(m_bottom, data);
      top_order_value = data[m_top->heap[0]];
      bottom_order_value = data[m_bottom->heap[0]];
   }
   while (median - bottom_order_value > data[m_top->heap[1]] - median)
   {  
      if (m_top->minCt == 1)
         {
            bottom_diff = median - data[m_bottom->heap[1]];
            top_diff = data[m_top->heap[1]] - median;
            mad = std::max(bottom_diff, top_diff);
            return mad;
         }
         rank_plus_1(m_top, data);
         rank_plus_1(m_bottom, data);
         top_order_value = data[m_top->heap[0]];
         bottom_order_value = data[m_bottom->heap[0]];
   }
   top_diff = top_order_value - median;
   bottom_diff = median - bottom_order_value;
   mad = std::max(top_diff, bottom_diff);
   return mad;
}

template <typename T>
void _hampel_filter(T* in_arr, int arr_len, int win_len, T* median, T* mad, T* out_arr, T scale)
{
   if (win_len % 2 == 0)
   {
      printf("Window length must be odd\n");
      exit(1);
   }
   const int rank = (win_len - 1) / 2;
   const int arr_len_thresh = arr_len - 1;
   const int lim = (win_len - 1) / 2;
   int rank_top = win_len - 2;
   int rank_bottom = rank_top - rank;
   int i;
   int i_shift = 0;
   Mediator* m = MediatorNew(win_len, rank);
   Mediator* m_top = MediatorNew(win_len, rank_top, true);
   Mediator* m_bottom = MediatorNew(win_len, rank_bottom, true);
   T* data = (T*)malloc(sizeof(T) * win_len);
   T input_, median_, mad_;
   int temp;
   for (i=win_len - lim; i > 0; i--)
   {
      MediatorReplaceHampel(data, m, m_bottom, m_top, in_arr[0]);
      get_mad(m_top, m_bottom, data[m->heap[0]], data, win_len);
   }
   for (i=0; i < lim; i++)
   {
      MediatorReplaceHampel(data, m, m_bottom, m_top, in_arr[i]);
      get_mad(m_top, m_bottom, data[m->heap[0]], data, win_len);
   }
   for (i=lim; i < arr_len; i++)
   {
      input_ = in_arr[i];
      MediatorReplaceHampel(data, m, m_bottom, m_top, in_arr[i]);
      median[i_shift] = median_ = data[m->heap[0]];
      mad[i_shift] = mad_ = get_mad(m_top, m_bottom, median_, data, win_len);
      out_arr[i_shift] = abs(input_ - median_) > scale * mad_ ? median_ : input_;
      i_shift++;
   }
   for (i=arr_len - lim; i < arr_len; i++)
   {
      input_ = in_arr[arr_len_thresh];
      MediatorReplaceHampel(data, m, m_bottom, m_top, input_);
      median[i] = median_ = data[m->heap[0]];
      mad[i] = mad_ = get_mad(m_top, m_bottom, median_, data, win_len);
      out_arr[i] = abs(input_ - median_) > scale * mad_ ? median_ : input_;
   }
   free(data);
   data = nullptr;
   MediatorFree(m);
   //MediatorFree(m_top);
   //MediatorFree(m_bottom);
}

int main(int argc, char* argv[])
{
   if (argc != 2)
   {
      printf("Usage: %s <window_size>\n", argv[0]);
      exit(1);
   }
   int fs = 20000;
   double freq = 13.47;
   double impulse_intervals = 287;
   int time = 2;
   double dc_amp = 3.1;
   int arr_len = fs * time;
   int win_len = std::stoi(argv[1]);

   double t[arr_len], in_arr[arr_len], out_arr[arr_len], median[arr_len], mad[arr_len];
   // seed the random number generator
   srand(0);
   for (int i = 0; i < 40000; i++)
   {
      t[i] = (double)i / fs;
      in_arr[i] = (-1 + t[i]) * dc_amp + sin(t[i] * 2 * M_PI * freq) + 0.1 * ((double)rand() / RAND_MAX - 0.5);
      if (i % (int)impulse_intervals == 0)
      {
         in_arr[i] += 3 * ((double)rand() / RAND_MAX - 0.5);
      }
   }
   //window size taken from user argument
   double scale = 1.0;
   time_t start, end;
   start = clock();
   _hampel_filter(in_arr, arr_len, win_len, median, mad, out_arr, scale);
   end = clock();
   double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
   printf("Time taken: %f miliseconds for window size %d and signal size %d\n", time_taken, win_len, arr_len);

   ofstream output_file;
   //filename as function of window size
   char filename[100];
   snprintf(filename, sizeof(filename), "output%d.csv", win_len);
   output_file.open(filename);
   output_file << "time,input,output,median,MAD" << endl;
   for (int i = 0; i < arr_len; i++)
   {
      output_file << std::setprecision(23) << t[i] << ", " << in_arr[i] << ", " << out_arr[i] << ", " << median[i] << ", " << mad[i] << endl;
   }
   output_file.close();
   printf("Output for window size %d created\n", win_len);
   return 0;
}  
