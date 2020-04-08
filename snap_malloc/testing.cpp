#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "myMalloc.hpp"
#include "testing.h"

MallocHeap* mHeap;
MallocPrinter* mPrinter;

/**
 * @brief Print the name of the test file and the initial state of
 *        the data structures
 *
 * @param name the path to the test file
 */
void initialize_test(const char * name) {
  mHeap = new MallocHeap();
  mPrinter = new MallocPrinter();

  const char * filename = strrchr(name, '/');
  printf("TEST: %s\n", filename ? filename+1 : name);

  printf("INTIAL STATE\n\n");
  printf("FREELIST\n");
  mPrinter->freelist_print(mHeap, MallocPrinter::print_object);
  printf("TAGS\n");
  mPrinter->tags_print(mHeap, MallocPrinter::print_object);
}

/**
 * @brief print the final state of the data structures and verify their
 *        validity
 */
void finalize_test() {
  printf("FINAL STATE\n\n");

  printf("FREELIST\n");
  mPrinter->freelist_print(mHeap, MallocPrinter::print_object);
  printf("TAGS\n");
  mPrinter->tags_print(mHeap, MallocPrinter::print_object);
  mHeap->verify();
}

/**
 * @brief Allocate size byte of memory and zero the memory
 *
 * @param size Number of bytes to allocate
 *
 * @return The pointer returned by th malloc call
 */
void * malloc_and_clear(size_t size) {
  // Allocate memory
  void * mem = mHeap->my_malloc(size);

  // Fill memory with 0 bytes
  memset(mem, 0, size);

  return mem;
}

/**
 * @brief Malloc n allocations of size bytes
 *
 * @param array Array to hold the pointers returned by malloc
 * @param size The size of each allocation
 * @param n The number of allocations
 * @param pf The formatter to determine printing
 * @param silent If true don't print
 *
 * @return The array of pointers to allocated memory
 */
void ** mallocing_loop(void ** array, size_t size, size_t n, MallocPrinter::printFormatter pf, bool silent) {
  if (!silent) {
    if (n == 1) {
      printf("mallocing %zu bytes\n", size);
    } else {
      printf("mallocing %zu bytes in %zu allocations\n", size, n);
    }
  }
  for (size_t i = 0; i < n; i++) {
    void * v = malloc_and_clear(size);
    if (array) {
      array[i] = v;
    }
  }
  if (!silent) {
    mPrinter->tags_print(mHeap, pf);
    puts("");
  }
  mHeap->verify();
  return array;
}

/**
 * @brief Malloc only a single allocaion of size size
 *
 * @param size the number of bytes to allocate
 * @param pf the formatter to use for printing
 * @param silent if true don't print
 *
 * @return The pointer returned by the malloc call
 */
void * mallocing(size_t size, MallocPrinter::printFormatter pf, bool silent) {
  void * p;
  return *mallocing_loop(&p, size, 1, pf, silent);
}

/**
 * @brief check that the memory is still zeroed out and free it
 *
 * @param p The pointer to free
 * @param size The size of the allocation p points to
 */
void check_and_free(char * p, size_t size) {
  // Verify memory is still zeroed out from allocation
  for (size_t i = 0; i < size; i++) {
    if(p[i] != 0) {
      fprintf(stderr, "Memory Corruption Detected\n");
      break;
    }
  }

  // Free pointer
  mHeap->my_free(p);
}

/**
 * @brief Free an array of pointers returned by malloc
 *
 * @param array The array of pointers to free
 * @param size The size of each allocation
 * @param n The number of allocations
 * @param pf The formatter to use for printing
 * @param silent If true don't print anything
 */
void freeing_loop(void ** array, size_t size, size_t n, MallocPrinter::printFormatter pf, bool silent) {
  if (!silent) {
    if (n == 1) {
      printf("freeing %zu bytes (", size);
      mPrinter->print_pointer(mHeap, (char *) *array - sizeof(MallocHeap::Header));
      puts(")");
    } else {
      printf("freeing %zu bytes from %zu allocations\n", size, n);
    }
  }
  for (size_t i = 0; i < n; i++) {
    check_and_free((char*)array[i], size);
  }
  if (!silent) {
    mPrinter->tags_print(mHeap, pf);
    puts("");
  }
  mHeap->verify();
}

/**
 * @brief Free a single pointer allocated by malloc
 *
 * @param p The pointer returned by malloc
 * @param size The size of the allocation
 * @param pf The formatter to use for printing
 * @param silent If true don't print anything
 */
void freeing(void * p, size_t size, MallocPrinter::printFormatter pf, bool silent) {
  freeing_loop(&p, size, 1, pf, silent);
}