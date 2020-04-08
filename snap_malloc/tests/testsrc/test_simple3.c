
#include <stdlib.h>
#include <stdio.h>
#include "testing.h"

int
main( int argc, char **argv )
{
  initialize_test(__FILE__);

  printf("Before any allocation\n");
  mPrinter->tags_print(mHeap, mPrinter->print_object);

  //test designed to coalesce from both sides of a block
  int * mem1 = (int *) mallocing( 8, mPrinter->print_status, false );

  printf("mem1 = malloc(8)\n");
  mPrinter->tags_print(mHeap, mPrinter->print_object);

  int * mem2 = (int *) mallocing( 8, mPrinter->print_status, false );
  
  printf("mem2 = malloc(8)\n");
  mPrinter->tags_print(mHeap, mPrinter->print_object);

  int * mem3 = (int *) mallocing( 8, mPrinter->print_status, false );

  printf("mem3 = malloc(8)\n");
  mPrinter->tags_print(mHeap, mPrinter->print_object);

  freeing(mem2, 8, mPrinter->print_status, false);
  printf("free(mem2))\n");
  mPrinter->tags_print(mHeap, mPrinter->print_object);

  finalize_test();
}
