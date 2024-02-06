//Copyright (c) 2011 ashelly.myopenid.com under <http://w...content-available-to-author-only...e.org/licenses/mit-license>
// I optimized by restriction of cases and proper initialization

#include <stdlib.h>
#include <stdio.h>
#define inline
 
typedef double Item;
typedef struct Mediator_t
{
   Item* data;  //circular queue of values
   int*  pos;   //index into `heap` for each value
   int*  heap;  //max/median/min heap holding indexes into `data`.
   int   N;     //allocated size.
   int   idx;   //position in circular queue
   int   minCt; //count of items in min heap
   int   maxCt; //count of items in max heap
} Mediator;

typedef enum {
   REFLECT = 1,
   CONSTANT = 2,
   NEAREST = 3,
   MIRROR = 4,
   WRAP = 5
} Mode;
 
/*--- Helper Functions ---*/
 
//returns 1 if heap[i] < heap[j]
inline int mmless(Mediator* m, int i, int j)
{
   return (m->data[m->heap[i]] < m->data[m->heap[j]]);
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
inline int mmCmpExch(Mediator* m, int i, int j)
{
   return (mmless(m,i,j) && mmexchange(m,i,j));
}
 
//maintains minheap property for all items below i.
void minSortDown(Mediator* m, int i)
{
   for (i*=2; i <= m->minCt; i*=2)
   {  if (i < m->minCt && mmless(m, i+1, i)) { ++i; }
      if (!mmCmpExch(m,i,i/2)) { break; }
   }
}
 
//maintains maxheap property for all items below i. (negative indexes)
void maxSortDown(Mediator* m, int i)
{
   for (i*=2; i >= -m->maxCt; i*=2)
   {  if (i > -m->maxCt && mmless(m, i, i-1)) { --i; }
      if (!mmCmpExch(m,i/2,i)) { break; }
   }
}
 
//maintains minheap property for all items above i, including median
//returns true if median changed
inline int minSortUp(Mediator* m, int i)
{
   while (i>0 && mmCmpExch(m,i,i/2)) i/=2;
   return (i==0);
}
 
//maintains maxheap property for all items above i, including median
//returns true if median changed
inline int maxSortUp(Mediator* m, int i)
{
   while (i<0 && mmCmpExch(m,i/2,i))  i/=2;
   return (i==0);
}
 
/*--- Public Interface ---*/
 
 
//creates new Mediator: to calculate `nItems` running median. 
//mallocs single block of memory, caller must free.
Mediator* MediatorNew(int nItems, int order)
{
   int size = sizeof(Mediator) + nItems * (sizeof(Item) + sizeof(int) * 2);
   Mediator* m =  malloc(size);
   m->data= (Item*)(m+1);
   m->pos = (int*) (m->data+nItems);
   m->heap = m->pos+nItems + (nItems/2); //points to middle of storage.
   m->N = nItems;
   m->idx = 0;
   m->minCt = order - 1;
   m->maxCt = nItems - order;
   while (nItems--)  //set up initial heap fill pattern: median,max,min,max,...
   {
      m->data[nItems] = 0;
      m->pos[nItems]= ((nItems+1)/2) * ((nItems&1)?-1:1);
      m->heap[m->pos[nItems]]=nItems;
   }
   return m;
}
 
 
//Inserts item, maintains median in O(lg nItems)
void MediatorInsert(Mediator* m, Item v)
{
   int p = m->pos[m->idx];
   Item old = m->data[m->idx];
   m->data[m->idx] = v;
   m->idx++;
   if(m->idx == m->N){m->idx = 0; }

   if (p > 0)         //new item is in minHeap
   {  if (v>old) { minSortDown(m,p); return; }
      if (minSortUp(m,p) && mmCmpExch(m,0,-1)) { maxSortDown(m,-1); }
   }
   else if (p < 0)   //new item is in maxheap
   {  if (v<old) { maxSortDown(m,p); return; }
      if (maxSortUp(m,p) && mmCmpExch(m,1,0)) { minSortDown(m,1); }
   }
   else //new item is at median
   {  if (maxSortUp(m,-1)) { maxSortDown(m,-1); }
      if (minSortUp(m, 1)) { minSortDown(m, 1); }
   }
}
 

void median_filter(double* in, double* out, int arr_len, int win_len, int order, Mode mode, double cval)
{
   int i, arr_len_thresh, lim = (win_len - 1) / 2;
   int lim2 = arr_len - lim;
   double value;
   Mediator* m = MediatorNew(win_len, order);

   switch (mode)
   {
      case REFLECT:
      for (i=win_len - 1; i >-1; i--){MediatorInsert(m, in[i]);}
      break;
      case CONSTANT:
      for (i=win_len - 1; i >-1; i--){MediatorInsert(m, cval);}
      break;
      case NEAREST:
      for (i=win_len - 1; i >-1; i--){MediatorInsert(m, in[0]);}
      break;
      case MIRROR:
      for (i=win_len - 1; i > 0; i--){MediatorInsert(m, in[i]);}
      break;
      case WRAP:
      for (i=arr_len - win_len; i < arr_len; i++){MediatorInsert(m, in[i]);}
      break;
   }

   for (i=0; i < lim; i++){MediatorInsert(m, in[i]);}
   for (i=lim; i<arr_len; i++)
   {
      value = in[i];
      MediatorInsert(m, in[i]);
      out[i - lim] = m->data[m->heap[0]];
   }
   switch (mode)
   {
      case REFLECT:
      arr_len_thresh = arr_len - 1;
      for (i=0; i<lim; i++)
      {
      MediatorInsert(m, in[arr_len_thresh - i]);
      out[lim2 + i] = m->data[m->heap[0]];
      }
      break;
      case CONSTANT:
      for (i=0; i<lim; i++)
      {
      MediatorInsert(m, cval);
      out[lim2 + i] = m->data[m->heap[0]];
      }
      break;
      case NEAREST:
      arr_len_thresh = arr_len - 1;
      for (i=0; i<lim; i++)
      {
      MediatorInsert(m, in[arr_len_thresh]);
      out[lim2 + i] = m->data[m->heap[0]];
      }
      break;
      case MIRROR:
      arr_len_thresh = arr_len - 2;
      for (i=0; i<lim + 1; i++)
      {
      MediatorInsert(m, in[arr_len_thresh - i]);
      out[lim2 + i] = m->data[m->heap[0]];
      }
      break;
      case WRAP:
      for (i=0; i < win_len; i++){
      MediatorInsert(m, in[i]);
      out[lim2 + i] = m->data[m->heap[0]];
      }
      break;
   }


   free(m);
}

int main()
{
   double in[] = {-3.05559757, -2.99335285, -2.99335285, -2.89569811, -2.86275085};
   double out[] = {0, 0, 0, 0, 0};
   int arr_len = 5;
   int win_len = 3;
   int order = 1;
   Mode mode = MIRROR;
   double cval = 0;
   median_filter(in, out, arr_len, win_len, order, mode, cval);
   return 1;
}