#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "myMalloc.hpp"

/* Due to the way assert() prints error messges we use out own assert function
 * for deteminism when testing assertions
 */
inline void MallocHeap::mAssert(int e) {
  if (!e) {
    fprintf(stderr, "Assertion Failed!\n");
    exit(1);
  }
}

// Helper functions for getting and storing size and state from Header
// Since the size is a multiple of 8, the last 3 bits are always 0s.
// Therefore we use the 3 lowest bits to store the state of the object.
// This is going to save 8 bytes in all objects.

inline std::size_t MallocHeap::get_block_size(MallocHeap::Header * h) {
     return h->size_and_state & ~0x3;
}

inline void MallocHeap::set_block_size(MallocHeap::Header * h, std::size_t size) {
     h->size_and_state = size | (h->size_and_state & 0x3);
}

inline enum MallocHeap::state MallocHeap::get_block_state(MallocHeap::Header *h) {
     return (enum state) (h->size_and_state & 0x3);
}

inline void MallocHeap::set_block_state(MallocHeap::Header * h, enum state s) {
     h->size_and_state = (h->size_and_state & ~0x3) | s;
}

inline void MallocHeap::set_block_size_and_state(MallocHeap::Header * h, std::size_t size, enum state s) {
     h->size_and_state=(size & ~0x3)|(s &0x3);
}

/**
 * @brief Set the ith bit in the bitmap
 *
 * @param i The bit to set
 */
inline void MallocHeap::set_bit(std::size_t i) {
  freelist_bitmap[i >> 3] |= (1 << (i & 7));
}

/**
 * @brief Unset the ith bit in the bitmap
 *
 * @param i The bit to unset
 */
inline void MallocHeap::unset_bit(std::size_t i) {
  freelist_bitmap[i >> 3] &= ~(1 << (i & 7));
}

/**
 * @brief Retrieve the ith bit from the bitmap
 *
 * @param i The bit to get
 */
inline bool MallocHeap::get_bit(std::size_t i) {
  return (freelist_bitmap[i >> 3] >> (i & 7)) & 1;
}

/**
 * @brief get the first bit >= i which it set
 *
 * @param i Bit to start checking at
 *
 * @return The index of the first bit satisfying the requirements
 */
inline std::size_t MallocHeap::get_next_set_bit(std::size_t i) {
  for (;i < n_lists - 1; i++) {
    if (get_bit(i)) {
      return i;
    }
  }
  return i;
}

/**
 * @brief Convert a request size to an freelist index
 *
 * @param size The request size to convert
 *
 * @return The index of the freelist corrospoding to a request of size size
 */
inline std::size_t MallocHeap::size_to_index(std::size_t size) {
  std::size_t i = ((size - getAllocHeaderSize() /*sizeof(MallocHeap::Header)*/) >> 3) - 1;
  return i >= n_lists ? n_lists - 1 : i;
}

/**
 * @brief Helper function to retrieve a Header pointer from a pointer and an
 *        offset
 *
 * @param ptr base pointer
 * @param off number of bytes from base pointer where Header is located
 *
 * @return a pointer to a Header offset bytes from pointer
 */
inline MallocHeap::Header * MallocHeap::get_header_from_offset(void * ptr, ptrdiff_t off) {
  return (MallocHeap::Header *)((char *) ptr + off);
}

/**
 * @brief Helper function to get the Header to the right of a given Header
 *
 * @param h original Header
 *
 * @return Header to the right of h
 */
MallocHeap::Header * MallocHeap::get_right_header(MallocHeap::Header * h) {
	return get_header_from_offset(h, get_block_size(h));
}

/**
 * @brief Helper function to get the Header to the left of a given Header
 *
 * @param h original Header
 *
 * @return Header to the right of h
 */
inline MallocHeap::Header * MallocHeap::get_left_header(MallocHeap::Header * h) {
  return get_header_from_offset(h, -h->left_size);
}

/**
 * @brief Fenceposts are marked as always allocated and may need to have
 * a left object size to ensure coalescing happens properly
 *
 * @param fp a pointer to the Header being used as a fencepost
 * @param left_size the size of the object to the left of the fencepost
 */
inline void MallocHeap::initialize_fencepost(MallocHeap::Header * fp, std::size_t left_size) {
	set_block_state(fp, state_fencepost);
	set_block_size(fp,getAllocHeaderSize());// sizeof(MallocHeap::Header));
	fp->left_size = left_size;
}

/**
 * @brief Helper function to maintain list of chunks from the OS for debugging
 *
 * @param hdr the first fencepost in the chunk allocated by the OS
 */
inline void MallocHeap::insert_os_chunk(MallocHeap::Header * hdr) {
  osChunkList[numOsChunks++] = hdr;
}

/**
 * @brief given a chunk of memory insert fenceposts at the left and
 * right boundaries of the block to prevent coalescing outside of the
 * block
 *
 * @param raw_mem a void pointer to the memory chunk to initialize
 * @param size the size of the allocated chunk
 */
inline void MallocHeap::insert_fenceposts(void * raw_mem, std::size_t size) {
  // Convert to char * before performing operations
  char * mem = (char *) raw_mem;

  // Insert a fencepost at the left edge of the block
  MallocHeap::Header * leftFencePost = (MallocHeap::Header *) mem;
  initialize_fencepost(leftFencePost, getAllocHeaderSize());

  // Maintain list of chunks for debugging
  //insert_os_chunk(leftFencePost);

  // Insert a fencepost at the right edge of the block
  MallocHeap::Header * rightFencePost = get_header_from_offset(mem, size - getAllocHeaderSize()); //sizeof(MallocHeap::Header));
  initialize_fencepost(rightFencePost, size - 2 * getAllocHeaderSize()); //sizeof(MallocHeap::Header));
}

/**
 * @brief Allocate another chunk from the OS and prepare to insert it
 * into the free list
 *
 * @param size The size to allocate from the OS
 *
 * @return A pointer to the allocable block in the chunk (just after the
 * first fencpost)
 */
MallocHeap::Header * MallocHeap::allocate_chunk(std::size_t size) {
  void * mem = sbrk(size);
  if (mem == (void *) -1) {
    return NULL;
  }

  insert_fenceposts(mem, size);
  MallocHeap::Header * hdr = (MallocHeap::Header *) ((char *)mem + getAllocHeaderSize()); //sizeof(MallocHeap::Header));
  set_block_state(hdr, state_unallocated);
  set_block_size(hdr,size - 2 * getAllocHeaderSize()); //sizeof(MallocHeap::Header));
  hdr->left_size = getAllocHeaderSize(); //sizeof(MallocHeap::Header);
  return hdr;
}

/**
 * @brief Given an allocation size return the freelist sentinel
 *        for that allocation size
 *
 * @param size The size to allocate
 *
 * @return The sentinel of the list corrosponding to the allocation size
 */
MallocHeap::Header * MallocHeap::get_freelist_sentinel(std::size_t size) {
  // Calculate list index
  std::size_t i = (((size | 7) - getAllocHeaderSize() /*sizeof(MallocHeap::Header)*/) >> 3);
  // Truncate list index to largest available list
  i = i >= n_lists ? n_lists - 1 : i - 1;
  // Return list
  return &freelistSentinels[i];
}

/**
 * @brief Given an allocation size return the first nonemtpty freelist sentinel
 *
 * @param size The size to allocate
 *
 * @return The sentinel which best fits the allocation request
 */
MallocHeap::Header * MallocHeap::get_populated_freelist_sentinel(std::size_t size) {
  return &freelistSentinels[get_next_set_bit(size_to_index(size))];
}

/**
 * @brief Split the block into two parts allocating the right portion to the
 *        user
 *
 * @param block block to allocate from
 * @param size amount to allocate
 *
 * @return a pointer to the allocated region of the block
 */
inline MallocHeap::Header * MallocHeap::split_block(MallocHeap::Header * block, std::size_t size) {
  MallocHeap::Header * remainder = block;
  std::size_t oldSize = get_block_size(remainder);
  MallocHeap::Header * right = get_header_from_offset(block, get_block_size(block));
  MallocHeap::Header * ret = get_header_from_offset(block, get_block_size(block) - size);

  // Update the remainder's size fields
  set_block_size(remainder, get_block_size(block) - size);

  // Update the block's size fields
  set_block_size(ret, size);
  ret->left_size = get_block_size(remainder);

  // Update the right block's size fields
  right->left_size = get_block_size(ret);

  move_coalesced(remainder, oldSize);

  return ret;
}

/**
 * @brief helper functions to remove a block from the freelist
 *
 * @param block block to remove
 */
inline void MallocHeap::freelist_remove(MallocHeap::Header * block) {
  // If the final node in the given freelist then update the bitmap
  if (block->next->next == block) {
    std::size_t index = ((std::size_t)block->next - (std::size_t)freelistSentinels) / sizeof(MallocHeap::Header);
    unset_bit(index);
  }
  block->prev->next = block->next;
  block->next->prev = block->prev;
}

/**
 * @brief General helper for inserting a block between two other blocks
 *
 * @param block block to insert
 * @param prev block previous to the block to insert
 * @param next block after the block to insert
 */
inline void MallocHeap::freelist_insert_between(MallocHeap::Header * block, MallocHeap::Header * prev,
                                           MallocHeap::Header * next) {
  block->next = next;
  block->prev = prev;
  next->prev = block;
  prev->next = block;
}

/**
 * @brief Helper to insert to the front of the free list
 *
 * @param block block to insert
 */
inline void MallocHeap::freelist_insert(MallocHeap::Header * block) {
	set_bit(size_to_index(get_block_size(block)));
	MallocHeap::Header * freelist = get_freelist_sentinel(get_block_size(block));
	freelist_insert_between(block, freelist, freelist->next);
}

/**
 * @brief Iterate over the free list in a first fit manor and
 *        allocate a block if possible
 *
 * @param size requested size of the allocation
 *
 * @return A block satisfying the request or NULL if no such block could be found
 */
inline MallocHeap::Header * MallocHeap::free_list_get_allocable(std::size_t size) {
  MallocHeap::Header * freelist = get_populated_freelist_sentinel(size);

  for (MallocHeap::Header * cur = freelist->next; cur != freelist; cur = cur->next) {
	  if (get_block_size(cur) >= size) {
		  if (get_block_size(cur) >= size + sizeof(MallocHeap::Header)) {
			  return split_block(cur, size);
		  } else {
			  freelist_remove(cur);
			  return cur;
		  }
	  }
  }
  return NULL;
}

/**
 * @brief Helper allocate an object given a raw request size from the user
 *
 * @param raw_size number of bytes the user needs
 *
 * @return A block satisfying the user's request
 */
inline MallocHeap::Header * MallocHeap::allocate_object(std::size_t raw_size) {
  if (raw_size == 0) {
    return NULL;
  }

  std::size_t size = (raw_size + getAllocHeaderSize() + 7) & ~7; //sizeof(MallocHeap::Header) + 7) & ~7;
  if (size < sizeof(MallocHeap::Header)) {
    size = sizeof(MallocHeap::Header);
  }

  // First try
  MallocHeap::Header * h = free_list_get_allocable(size);
  if (h != NULL) {
    return h;
  }

  while(h == NULL) {
    // Allocate more memory
    MallocHeap::Header * newChunk = allocate_chunk(arena_size);
    if (newChunk == NULL) {
      return NULL;
    }

    /* Check if next to another chunk from the OS */
    MallocHeap::Header * prevFencePost = get_header_from_offset(newChunk, 2 * -getAllocHeaderSize());
    MallocHeap::Header * rfp = get_header_from_offset(newChunk, get_block_size(newChunk));
    if (prevFencePost == lastFencePost) {
      // Move Header to fencepost
	    set_block_size(prevFencePost, get_block_size(newChunk) + 2 * getAllocHeaderSize());

      // Update left size of the right fencepost
	    rfp->left_size = get_block_size(prevFencePost);

      // Free node to coalesce
	    set_block_state(prevFencePost, state_unallocated);
      free_list_deallocate(prevFencePost);
    } else {
      // Insert chunk
      freelist_insert(newChunk);
      insert_os_chunk(get_header_from_offset(newChunk, -getAllocHeaderSize()));
    }
    lastFencePost = rfp;

    // Try again
    h = free_list_get_allocable(size);
  }

  return h;
}

/**
 * @brief Helper to get the Header from a pointer allocated with malloc
 *
 * @param p pointer to the data region of the block
 *
 * @return A pointer to the Header of the block
 */
inline MallocHeap::Header * MallocHeap::ptr_to_header(void * p) {
  return (MallocHeap::Header *)((char *) p - getAllocHeaderSize()); //sizeof(MallocHeap::Header));
}

inline void MallocHeap::move_coalesced(MallocHeap::Header * block, std::size_t oldSize) {
	if ((get_block_size(block) >> 3 != oldSize >> 3) &&
	    ((get_block_size(block) >> 3 < n_lists) || (oldSize >> 3 < n_lists))) {
    // Change lists
    freelist_remove(block);
    freelist_insert(block);
  }
}

/**
 * @brief Helper to handle the case where only the left neighbor is free
 *
 * @param left The block to the left of the block being freed
 * @param block The block being freed
 * @param right The block to the right of the block being freed
 */
inline void MallocHeap::coalesce_left(MallocHeap::Header * left, MallocHeap::Header * block, MallocHeap::Header * right) {
	std::size_t oldSize = get_block_size(left);

  // Update sizes
	set_block_size(left, get_block_size(left) + get_block_size(block));
	right->left_size = get_block_size(left);

  // Pointers do not need to be modified

  move_coalesced(left, oldSize);
}

/**
 * @brief Helper to handle the case where only the right neighbor is free
 *
 * @param block The block being freed
 * @param right The block to the right of the block being freed
 */
inline void MallocHeap::coalesce_right(MallocHeap::Header * block, MallocHeap::Header * right) {
  MallocHeap::Header * rightRight = get_right_header(right);
  std::size_t oldSize = get_block_size(right);

  // Update sizes
  set_block_size(block, get_block_size(block)+get_block_size(right));
  rightRight->left_size = get_block_size(block);

  // Update pointers
  block->next = right->next;
  block->prev = right->prev;
  block->prev->next = block;
  block->next->prev = block;

  move_coalesced(block, oldSize);
}

/**
 * @brief Halper to handle the case where both neighbors are free. The final
 * location of the freed block is where the left neighbor was in the list
 *
 * @param left The block to the left of the block being freed
 * @param block The block being freed
 * @param right The block to the right of the block being freed
 */
inline void MallocHeap::coalesce_both(MallocHeap::Header * left, MallocHeap::Header * block, MallocHeap::Header * right) {
  MallocHeap::Header * rightRight = get_right_header(right);
  std::size_t oldSize = get_block_size(left);

  // Update sizes
  set_block_size(left, get_block_size(left) + get_block_size(block) + get_block_size(right));
  rightRight->left_size = get_block_size(left);

  // Update pointers
  freelist_remove(right);

  move_coalesced(left, oldSize);
}

/**
 * @brief Helper to manage how to return the block to the free list
 *
 * @param block block to return to the free list
 */
inline void MallocHeap::free_list_deallocate(MallocHeap::Header * block) {
  MallocHeap::Header * left = get_left_header(block);
  MallocHeap::Header * right = get_right_header(block);

  if (get_block_state(left) && get_block_state(right)) {
    freelist_insert(block);
  } else if (!get_block_state(left) && !get_block_state(right)) {
    coalesce_both(left, block, right);
  } else if (!get_block_state(left)) {
    coalesce_left(left, block, right);
  } else if (!get_block_state(right)) {
    coalesce_right(block, right);
  }
}


/**
 * @brief Helper to manage deallocation of a pointer returned by the user
 *
 * @param p The pointer returned to the user by a call to malloc
 */
inline void MallocHeap::deallocate_object(void * p) {
  // Allow NULL pointers to be freed
  if (p == NULL) {
    return;
  }

  // Retrieve the block's Header from the pointer and verify it has not be overflowed
  MallocHeap::Header * block = ptr_to_header(p);
  if (get_block_state(block) == state_unallocated) {
    fprintf(stderr, "Double Free Detected\n");
    mAssert(false);
  }

  // Update the block's allocation status
  set_block_state(block, state_unallocated);

  // Return the block to the free list
  free_list_deallocate(block);
}

/**
 * @brief Helper to detect cycles in the free list
 * https://en.wikipedia.org/wiki/Cycle_detection#Floyd's_Tortoise_and_Hare
 *
 * @return One of the nodes in the cycle or NULL if no cycle is present
 */
inline MallocHeap::Header * MallocHeap::detect_cycles() {
  for (int i = 0; i < n_lists; i++) {
    MallocHeap::Header * freelist = &freelistSentinels[i];
    for (MallocHeap::Header * slow = freelist->next, * fast = freelist->next->next;
         fast != freelist;
         slow = slow->next, fast = fast->next->next) {
      if (slow == fast) {
        return slow;
      }
    }
  }
  return NULL;
}

/**
 * @brief Helper to verify that there are no unlinked previous or next pointers
 *        in the free list
 *
 * @return A node whose previous and next pointers are incorrect or NULL if no
 *         such node exists
 */
inline MallocHeap::Header * MallocHeap::verify_pointers() {
  for (int i = 0; i < n_lists; i++) {
    MallocHeap::Header * freelist = &freelistSentinels[i];
    for (MallocHeap::Header * cur = freelist->next; cur != freelist; cur = cur->next) {
      if (cur->next->prev != cur || cur->prev->next != cur) {
        return cur;
      }
    }
  }
  return NULL;
}

/**
 * @brief Verify the structure of the free list is correct by checkin for
 *        cycles and misdirected pointers
 *
 * @return true if the list is valid
 */
inline bool MallocHeap::verify_freelist() {
  MallocHeap::Header * cycle = detect_cycles();
  if (cycle != NULL) {
    fprintf(stderr, "Cycle Detected\n");
    return false;
  }

  MallocHeap::Header * invalid = verify_pointers();
  if (invalid != NULL) {
    fprintf(stderr, "Invalid pointers\n");
    return false;
  }

  return true;
}

/**
 * @brief Helper to verify that the sizes in a chunk from the OS are correct
 *        and that allocated node's canary values are correct
 *
 * @param chunk AREA_SIZE chunk allocated from the OS
 *
 * @return a pointer to an invalid Header or NULL if all Header's are valid
 */
inline MallocHeap::Header * MallocHeap::verify_chunk(MallocHeap::Header * chunk) {
	if (get_block_state(chunk) != state_fencepost) {
    fprintf(stderr, "Invalid fencepost\n");
    return chunk;
  }

	for (; get_block_state(chunk) != state_fencepost; chunk = get_right_header(chunk)) {
	  if (get_block_size(chunk)  != get_right_header(chunk)->left_size) {
      fprintf(stderr, "Invalid sizes\n");
      return chunk;
    }
  }

  return NULL;
}

/**
 * @brief For each chunk allocated by the OS verify that the boundary tags
 *        are consistent
 *
 * @return true if the boundary tags are valid
 */
inline bool MallocHeap::verify_tags() {
  for (std::size_t i = 0; i < numOsChunks; i++) {
    MallocHeap::Header * invalid = verify_chunk(osChunkList[i]);
    if (invalid != NULL) {
      return invalid;
    }
  }

  return NULL;
}

/**
 * @brief Initialize mutex lock and prepare an initial chunk of memory for allocation
 */
MallocHeap::MallocHeap() {
  // Zero additional chunks have been requested of the OS before insert_os_chunk()
  numOsChunks = 0;

  // Initialize mutex for thread safety
  pthread_mutex_init(&mutex, NULL);

  // Manually set printf buffer so it won't call malloc
  setvbuf(stdout, NULL, _IONBF, 0);

  // Allocate the first chunk from the OS
  MallocHeap::Header * block = allocate_chunk(arena_size);

  MallocHeap::Header * prevFencePost = get_header_from_offset(block, -getAllocHeaderSize());
  insert_os_chunk(prevFencePost);

  lastFencePost = get_header_from_offset(block, get_block_size(block) );

  // Set the base pointer to the beginning of the first fencepost in the first
  // chunk from the OS
  base = ((char *) block) - getAllocHeaderSize(); //sizeof(MallocHeap::Header);

  // Initialize freelist sentinels
  for (int i = 0; i < n_lists; i++) {
    MallocHeap::Header * freelist = &freelistSentinels[i];
    freelist->next = freelist;
    freelist->prev = freelist;
  }

  // Insert first chunk into the free list
  freelist_insert(block);
}

/*
 * External interface
 */
void * MallocHeap::my_malloc(std::size_t size) {
  pthread_mutex_lock(&mutex);
  MallocHeap::Header * hdr = allocate_object(size);
  pthread_mutex_unlock(&mutex);
  if (hdr == NULL) {
    return NULL;
  } else {
	  set_block_state(hdr, state_allocated);
	  return hdr->data;
  }
}

void * MallocHeap::my_calloc(std::size_t nmemb, std::size_t size) {
  return memset(my_malloc(size * nmemb), 0, size * nmemb);
}

void * MallocHeap::my_realloc(void * ptr, std::size_t size) {
  void * mem = my_malloc(size);
  memcpy(mem, ptr, size);
  my_free(ptr);
  return mem;
}

void MallocHeap::my_free(void * p) {
  pthread_mutex_lock(&mutex);
  deallocate_object(p);
  pthread_mutex_unlock(&mutex);
}

bool MallocHeap::verify() {
  return verify_freelist() && verify_tags();
}
