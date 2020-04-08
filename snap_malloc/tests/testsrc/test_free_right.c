#include "testing.h"

int main() {
  initialize_test(__FILE__);

  void * ptr = mallocing(3, mPrinter->print_status, false);
  void * ptr2 = mallocing(2, mPrinter->print_status, false);
  mallocing(1, mPrinter->print_status, false);
  freeing(ptr, 3, mPrinter->print_status, false);
  freeing(ptr2, 2, mPrinter->print_status, false);

  finalize_test();
}
