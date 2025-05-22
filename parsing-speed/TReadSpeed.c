/* This is a simple speed test. We read a file assuming that it is a trace
   with millions of 27 bytes long events. The byte tells us the type which tells us
   the length of the payload to skip to reach the next event. The file is memory mapped for
   hopefully greater speed. */
   
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>

int main (int argc, char *argv[])
{
  unsigned event_types[256], event_counts[256], total;
  int fd;
  unsigned file_length;
  uint8_t *mf, *cursor;
  
  fd = open(argv[1], O_RDONLY);
  file_length = atoi(argv[2]);
  cursor = mf = (uint8_t *) mmap(NULL, 4294967296, PROT_READ, MAP_PRIVATE, fd, 0);
  
  for(int i = 0; i < 256; i++) {
    event_counts[i] = 0;
    event_types[i] = 27 + ((i % 2) ? -2 : 2);
  }
  
  while(cursor < mf + file_length) {
    event_counts[*cursor]++;
    cursor += event_types[*cursor];
  }

  total = 0;
  printf("Event counts: {");
  for(int i = 0; i < 256; i++) { total += event_counts[i]; printf(" %d", event_counts[i]); }
  printf("}\n Event length: {");  
  for(int i = 0; i < 256; i++) printf(" %d", event_types[i]);
  printf("}\nTotal count: %d\n", total);
}
