#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myMalloc.hpp"
#include "printing.h"

#define MALLOC_COLOR "MALLOC_DEBUG_COLOR"

static bool check_env;
static bool use_color;

/**
 * @brief Print just the block's size
 *
 * @param block The block to print
 */
void mallocPrinter::basic_print(MallocHeap* mHeap, Header * block) {
	printf("[%zd] -> ", mHeap->get_block_size(block));
}

/**
 * @brief Print just the block's size
 *
 * @param block The block to print
 */
void mallocPrinter::print_list(MallocHeap* mHeap, Header * block) {
	printf("[%zd]\n", mHeap->get_block_size(block));
}

/**
 * @brief return a string representing the allocation status
 *
 * @param allocated The allocation status field
 *
 * @return A string representing the allocation status
 */
inline const char * mallocPrinter::allocated_to_string(char allocated) {
  switch(allocated) {
    case UNALLOCATED:
      return "false";
    case ALLOCATED:
      return "true";
    case FENCEPOST:
      return "fencepost";
  }
  assert(false);
}

bool mallocPrinter::check_color() {
  if (!check_env) {
    // genenv allows accessing environment varibles
    const char * var = getenv(MALLOC_COLOR);
    use_color = var != NULL && !strcmp(var, "1337_CoLoRs");
    check_env = true;
  }
  return use_color;
}

/**
 * @brief Change the tty color based on the block's allocation status
 *
 * @param block The block to print the allocation status of
 */
void mallocPrinter::print_color(MallocHeap* mHeap, Header * block) {
  if (!check_color()) {
    return;
  }

  switch(mHeap->get_block_state(block)) {
    case UNALLOCATED:
      printf("\033[0;32m");
      break;
    case ALLOCATED:
      printf("\033[0;34m");
      break;
    case FENCEPOST:
      printf("\033[0;33m");
      break;
  }
}

void mallocPrinter::clear_color() {
  if (check_color()) {
    printf("\033[0;0m");
  }
}

inline bool mallocPrinter::is_sentinel(MallocHeap* mHeap, void * p) {
  for (int i = 0; i < N_LISTS; i++) {
    if (&(mHeap->freelistSentinels[i]) == p) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Print the free list pointers if RELATIVE_POINTERS is set to true
 * then print the pointers as an offset from the base of the heap. This allows
 * for determinism in testing.
 * (due to ASLR https://en.wikipedia.org/wiki/Address_space_layout_randomization#Linux)
 *
 * @param p The pointer to print
 */
void mallocPrinter::print_pointer(MallocHeap* mHeap, void * p) {
  if (is_sentinel(mHeap, p)) {
    printf("SENTINEL");
  } else {
    if (RELATIVE_POINTERS) {
      printf("%04zd", (char*)p - (char*)mHeap->base);
    } else {
      printf("%p", p);
    }
  }
}

/**
 * @brief Verbose printing of all of the metadata fields of each block
 *
 * @param block The block to print
 */
void mallocPrinter::print_object(MallocHeap* mHeap, Header * block) {
  print_color(mHeap, block);

  printf("[\n");
  printf("\taddr: ");
  print_pointer(mHeap, block);
  puts("");
  printf("\tsize: %zd\n", get_block_size(block) );
  printf("\tleft_size: %zd\n", block->left_size);
  printf("\tallocated: %s\n", allocated_to_string(get_block_state(block)));
  if (!get_block_state(block)) {
    printf("\tprev: ");
    print_pointer(mHeap, block->prev);
    puts("");

    printf("\tnext: ");
    print_pointer(mHeap, block->next);
    puts("");
  }
  printf("]\n");

  clear_color();
}

/**
 * @brief Simple printer that just prints the allocation status of each block
 *
 * @param block The block to print
 */
void mallocPrinter::print_status(MallocHeap* mHeap, Header * block) {
  print_color(mHeap, block);
  switch(mHeap->get_block_state(block)) {
    case UNALLOCATED:
      printf("[U]");
      break;
    case ALLOCATED:
      printf("[A]");
      break;
    case FENCEPOST:
      printf("[F]");
      break;
  }
  clear_color();
}

/*
static void print_bitmap() {
  printf("bitmap: [");
  for(int i = 0; i < N_LISTS; i++) {
    if ((freelist_bitmap[i >> 3] >> (i & 7)) & 1) {
      printf("\033[32m#\033[0m");
    } else {
      printf("\033[34m_\033[0m");
    }
    if (i % 8 == 7) {
      printf(" ");
    }
  }
  puts("]");
}
*/

/**
 * @brief Print a linked list between two nodes using a provided print function
 *
 * @param pf Function to perform the actual printing
 * @param start Node to start printing at
 * @param end Node to stop printing at
 */
void mallocPrinter::print_sublist(printFormatter pf, Header * start, Header * end) {
  for (Header * cur = start; cur != end; cur = cur->next) {
    pf(cur);
  }
}

/**
 * @brief print the full freelist
 *
 * @param pf Function to perform the Header printing
 */
void mallocPrinter::freelist_print(printFormatter pf) {
  if (!pf) {
    return;
  }

  for (size_t i = 0; i < N_LISTS; i++) {
    Header * freelist = &freelistSentinels[i];
    if (freelist->next != freelist) {
      printf("L%zu: ", i);
      print_sublist(pf, freelist->next, freelist);
      puts("");
    }
    fflush(stdout);
  }
}

/**
 * @brief print the boundary tags from each chunk from the OS
 *
 * @param pf Function to perform the Header printing
 */
void mallocPrinter::tags_print(printFormatter pf) {
  if (!pf) {
    return;
  }

  for (size_t i = 0; i < numOsChunks; i++) {
    Header * chunk = osChunkList[i];
    pf(chunk);
    for (chunk = get_right_Header(chunk);
         get_block_state(chunk) != FENCEPOST;
         chunk = get_right_Header(chunk)) {
        pf(chunk);
    }
    pf(chunk);
    fflush(stdout);
  }
}
