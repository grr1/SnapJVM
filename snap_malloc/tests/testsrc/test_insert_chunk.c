#include <unistd.h>

#include "testing.h"
#include "malloc.h"

int main() {
  initialize_test(__FILE__);

  mallocing(mHeap->arena_size - 3 * mHeap->getAllocHeaderSize() - 48, mPrinter->print_status, false);
  printf("Calling sbrk to allocate memory between malloc's chunks\n");
  sbrk(1024);
  mallocing(mHeap->arena_size - 3 * mHeap->getAllocHeaderSize() - 48, mPrinter->print_status, false);

  finalize_test();
}
