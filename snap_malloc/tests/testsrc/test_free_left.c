#include "testing.h"

int main() {
  initialize_test(__FILE__);

  void * ptr = mallocing(99, mPrinter->print_status, false);
  freeing(ptr, 99, mPrinter->print_status, false);

  finalize_test();
}
