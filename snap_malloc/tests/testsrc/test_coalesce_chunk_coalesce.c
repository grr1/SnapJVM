#include "testing.h"
#include "malloc.h"

int main() {
  initialize_test(__FILE__);

  void * ptr = mallocing(8, mPrinter->print_status, false);
  mallocing(mHeap->arena_size - 3 * mHeap->getAllocHeaderSize() - 128, mPrinter->print_status, false);
  freeing(ptr, 8, mPrinter->print_status, false);
  mallocing(mHeap->arena_size - 3 * mHeap->getAllocHeaderSize() - 128, mPrinter->print_status, false);

  finalize_test();
}
