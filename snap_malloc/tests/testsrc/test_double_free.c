#include "testing.h"

int main() {
  initialize_test(__FILE__);

  void * ptr = mallocing(8, mPrinter->print_status, false);
  freeing(ptr, 8, mPrinter->print_status, false);
  mHeap->my_free(ptr);

  finalize_test();
}
