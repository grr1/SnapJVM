#include "testing.h"

#define NALLOCS 20000

int main() {
  initialize_test(__FILE__);

  void * ptrs[NALLOCS];
  mallocing_loop(ptrs, 16384, NALLOCS, mPrinter->print_status, false);

  mPrinter->tags_print(mHeap, mPrinter->print_object);

  finalize_test();
}
