#include <stdlib.h>

#include "testing.h"

int main() {
  initialize_test(__FILE__);

  mallocing(1, mPrinter->print_status, false);
  for (int i = 1; i <= mHeap->n_lists; i++) {
    size_t s = 8 * (i + 1);
    void * ptr = mallocing(s, mPrinter->print_status, false); 

    // Malloc a gap in the freelist
    mallocing(s + 8, mPrinter->print_status, false);

    freeing(ptr, s, mPrinter->print_status, false);
  }

  finalize_test();
}
