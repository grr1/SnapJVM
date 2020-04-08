#include "testing.h"
#include "malloc.h"

int main() {
  initialize_test(__FILE__);

  mallocing(mHeap->arena_size - 3 * mHeap->getAllocHeaderSize() /*sizeof(header)*/, mPrinter->print_status, false);

  finalize_test();
}
