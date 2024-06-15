#include <Kernel/math.h>
#include <iostream>
#include <fstream>

#include "hampel.h"

using namespace std;

template <typename T> extern
void _hampel_filter(T* in_arr, int arr_len, int win_len, T* median, T* mad, T* out_arr, T scale);

int main(int argc, char* argv[])
{
   if (argc != 3)
   {
      printf("Usage: %s <window_size> <scale>\n", argv[0]);
      exit(1);
   }
   int fs = 20000;
   double freq = 13.47;
   double impulse_intervals = 287;
   int time = 2;
   double dc_amp = 3.1;
   int arr_len = fs * time;
   int win_len = std::stoi(argv[1]);
   double scale = std::stod(argv[2]);

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
   time_t start, end;
   start = clock();
   _hampel_filter(in_arr, arr_len, win_len, median, mad, out_arr, scale);
   end = clock();
   double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
   printf("Time taken: %f miliseconds for window size %d and signal size %d, used scale %f \n", time_taken, win_len, arr_len, scale);

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
