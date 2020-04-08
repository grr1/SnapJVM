#include "testing.h"
#include "malloc.h"

int main() {
  initialize_test(__FILE__);

  mallocing(mHeap->arena_size - 3 * mHeap->getAllocHeaderSize(), mPrinter->print_status, false);
  mallocing(8, mPrinter->print_status, false);

  finalize_test();
}
