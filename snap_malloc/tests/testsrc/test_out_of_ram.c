#include <sys/resource.h>

#include "testing.h"
#include "malloc.h"

int main() {
  initialize_test(__FILE__);

  // Set the resource limit for the processes address space to 1 byte
  // This causes all future calls to sbrk to fail
  struct rlimit lim;
  getrlimit(RLIMIT_AS, &lim);
  lim.rlim_cur = 1;
  setrlimit(RLIMIT_AS, &lim);

  // Alloce more than allotted memory
  void * p = mallocing(mHeap->arena_size - 3 * mHeap->getAllocHeaderSize(), mPrinter->print_status, false);

  printf("Mallocing an additional 8 bytes, which requires more memory from "
         "sbrk, but sbrk will fail\n");
  void * ptr = malloc(8);

  if (ptr == NULL) {
    perror("SUCCESS: Malloc Failed");
  } else {
    printf("Malloc should have failed due to sbrk returning failure\n");
  }
  puts("");

  printf("Testing that available memory can still be reallocated\n");
  freeing(p, 8, mPrinter->print_status, false);
  mallocing(8, mPrinter->print_status, false);

  finalize_test();
}
