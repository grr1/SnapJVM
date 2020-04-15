#include <stdlib.h>
#include <stdio.h>

#include "myMalloc.hpp"
#include "testing.h"

int main() {
  initialize_test(__FILE__);

  printf("Mallocing larger than the maximum possible allocation\n");
  void * ptr = mHeap->my_malloc(mHeap->arena_size - 3 * sizeof(MallocHeap::Header) + 1);

  fflush(stdout);
  if (ptr == NULL) {
    perror("Allocation too large");
  } else {
    fprintf(stderr, "Expected an error for an allocation larger than the maximum allocable amount\n");
  }
  puts("");

  printf("Verify that malloc still works after invalid request\n");
  void * ptr2 = mallocing(8, mPrinter->print_status, false);
  freeing(ptr2, 8, mPrinter->print_status, false);

  finalize_test();
}
