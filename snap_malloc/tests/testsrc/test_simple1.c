
#include <stdlib.h>
#include <stdio.h>
#include "myMalloc.hpp"
#include "printing.hpp"
#include "testing.h"

int
main( int argc, char **argv )
{
  initialize_test(__FILE__);

  printf("Before any allocation\n");
  mPrinter->tags_print(mHeap, mPrinter->print_object);

  //test designed to coalesce from both sides of a block
  int * mem1 = (int *) mallocing( 8, mPrinter->print_status, false );
  *mem1 = 10;

  printf("mem1 = malloc(8)\n");
  mPrinter->tags_print(mHeap, mPrinter->print_object);

  finalize_test();
}
