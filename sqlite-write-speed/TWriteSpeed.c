/* This is a simple speed test. A program executes a large number of 
   function calls (1 + 2 x repeat1 + repeat1 x repeat2). The default values
   for repeat1 and repeat2 are 10 and 1000. They can be changed as argv[1] and
   argv[2] on the command line.
   
   The function calls can be instrumented with the -finstrument-functions compilation
   option and logged in different ways (e.g. with lttng-ust versus to a sqlite database).
   
   At the end, the program prints Total, the number of function calls (including the call to main)
   and Result, an arbitrary checksum computed through the calls, to insure that no call is optimised out.
 */
   
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

unsigned long nb_calls = 0;

unsigned long function_c(unsigned long i, unsigned long j)
{
  nb_calls++;
  return i + (j % 3);
}

unsigned long function_b(unsigned long i, unsigned long j)
{
  nb_calls++;
  return i + j;
}

unsigned long function_a(unsigned long i, unsigned long j)
{
  nb_calls++;
  for(int k = 0; k < j; k++) i = function_c(i, j + k);
  return (j % 2 == 0) ? i + 1: i + 2;
}

int main (int argc, char *argv[])
{
  unsigned long total = 0, repeat1 = 10, repeat2 = 1000;

  nb_calls++;
  if(argc > 1) repeat1 = atoi(argv[1]);
  if(argc > 2) repeat2 = atoi(argv[2]);
  
  for(int i = 0; i < repeat1; i++) {
    total = function_a(total, repeat2);
    total = function_b(total, repeat2 % 4);
  }
  printf("Total: %lu\nResult: %lu\n", nb_calls, total);
}
