#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <stdbool.h>
#include <sys/types.h>
#include <cstddef>

/* Size of the Header for an allocated block
 *
 * The size of the normal minus the size of the two free list pointers as
 * they are only maintained while block is free
 */
// #define ALLOC_HEADER_SIZE (sizeof(Header) - (2 * sizeof(Header *)))

class MallocHeap {
	friend class mallocPrinter;
	friend class mallocTester;

	public:
		MallocHeap();
		~MallocHeap();

	private:
		/*
		 * The Header contains all metadata about a block to be allocated
		 * The size fields allow accessing the neighboring blocks in memory by
		 * calculating their stating and ending addresses.
		 *
		 * When a block is free the free list pointer fields are set to the next
		 * and previous blocks in the free list to allow faster traversal of the
		 * freed blocks
		 *
		 * When a block is allocated the user's data starts by using the 16 bytes
		 * reserved for the freelist pointers
		 *
		 * The zero length array at the end of the struct is the beginning of the
		 * usable data in the block.
		 *
		 * FIELDS ALWAYS PRESENT
		 * size_t size The size of the current block *including metadata*
		 * size_t left_size The size of the block to the left (in memory)
		 *
		 * FIELDS PRESENT WHEN FREE
		 * Header * next The next block in the free list (only valid if free)
		 * Header * prev The previous block in the free list (only valid if free)
		 *
		 * FIELD PRESENT WHEN ALLOCATED
		 * size_t[] canary magic value to detetmine if a block as been corrupted
		 *
		 * char[] data first byte of data pointed to by the list
		 */
		typedef struct Header {
		  size_t size_and_state;
		  size_t left_size;
		  union {
		    // Used when the object is free
		    struct {
		      struct Header * next;
		      struct Header * prev;
		    };
		    // Used when the object is allocated
		    char data[0];
		  };
		} Header;

		/**
		 * @brief enum representing the allocation state of a block
		 *
		 * enums provice a method of specifying a set of named values
		 * http://en.cppreference.com/w/c/language/enum
		 */
		enum state {
		  UNALLOCATED = 0,
		  ALLOCATED = 1,
		  FENCEPOST = 2,
		};

		enum {
			RELATIVE_POINTERS = 1,
			ARENA_SIZE = 4096,
			N_LISTS = 59,
			MIN_ALLOCATION = 8,		/* The minimum size request the allocator will service */
			MAX_OS_CHUNKS 1024,
			/* Size of the Header for an allocated block
			 *
			 * The size of the normal minus the size of the two free list pointers as
			 * they are only maintained while block is free
			 */
			ALLOC_HEADER_SIZE (sizeof(Header) - (2 * sizeof(Header *)))
		};

		// Helper functions for getting and storing size and state from Header
		// Since the size is a multiple of 8, the last 3 bits are always 0s.
		// Therefore we use the 3 lowest bits to store the state of the object.
		// This is going to save 8 bytes in all objects.
		inline size_t get_block_size(Header * h);
		inline void set_block_size(Header * h, size_t size);
		inline enum state get_block_state(Header *h);
		inline void set_block_state(Header * h, enum state s);
		inline void set_block_size_and_state(Header * h, size_t size, enum state s);

		// Manage accessing and mutatig the freelist bitmap
		inline void set_bit(size_t i);
		inline void unset_bit(size_t i);
		inline bool get_bit(size_t i);
		inline size_t get_next_set_bit(size_t i);
		inline size_t size_to_index(size_t size);

		// Helper functions for manipulating pointers to Headers
		inline Header * get_Header_from_offset(void * ptr, ptrdiff_t off);
		inline Header * get_left_Header(Header * h);
		inline Header * ptr_to_Header(void * p);

		// Helper functions for freeing a block
		inline void move_coalesced(Header * block, size_t oldSize);
		inline void coalesce_left(Header * left, Header * block, Header * right);
		inline void coalesce_right(Header * block, Header * right);
		inline void coalesce_both(Header * left, Header * block, Header * right);
		inline void free_list_deallocate(Header * block);
		inline void deallocate_object(void * p);

		// Helper functions for allocating more memory from the OS
		inline void initialize_fencepost(Header * fp, size_t left_size);
		inline void insert_os_chunk(Header * hdr);
		inline void insert_fenceposts(void * raw_mem, size_t size);
		Header * allocate_chunk(size_t size);

		// Helper functions for allocating a block
		Header * get_freelist_sentinel(size_t size);
		Header * get_populated_freelist_sentinel(size_t size);
		inline Header * split_block(Header * block, size_t size);
		inline Header * free_list_get_allocable(size_t size);
		inline Header * allocate_object(size_t raw_size);

		// Helper functions for manipulating the freelist
		inline void freelist_remove(Header * block);
		inline void freelist_insert_between(Header * block, Header * prev, Header * next);
		inline void freelist_insert(Header * block);

		// Helper to find a block's right neighbor
		Header * get_right_Header(Header * h);

		/*
		 * Mutex to ensure thread safety for the freelist
		 */
		pthread_mutex_t mutex;

		/*
		 * Array of sentinel nodes for the freelists
		 */
		Header freelistSentinels[N_LISTS];

		/*
		 * bitmap representing which of the freelists have available blocks. This
		 * allows for an optimized block lookup. By finding the index in the
		 * bitmap corresponding to the current request size and then finding the next
		 * set bit in the bitmap we can determine the location of the block to allocate
		 *
		 * By using the bitmap we only need to load a small number of bytes into cache
		 * to perform this lookup operation instead of checking each of the sentinels.
		 */
		char freelist_bitmap[(N_LISTS | 7) >> 3];

		/*
		 * Pointer to the second fencepost in the most recently allocated chunk from
		 * the OS. Used for coalescing chunks
		 */
		Header * lastFencePost;

		/*
		 * Pointer to maintian the base of the heap to allow printing based on the
		 * distance from the base of the heap
		 */
		void * base;

		Header * osChunkList [MAX_OS_CHUNKS];
		size_t numOsChunks;

		// Helper functions for verifying that the data structures are structurally valid
		inline Header * detect_cycles();
		inline Header * verify_pointers();
		inline bool verify_freelist();
		inline Header * verify_chunk(Header * chunk);
		inline bool verify_tags();

		// Debug list verifitcation
		bool verify();

		// Malloc interface
		void * my_malloc(size_t size);
		void * my_calloc(size_t nmemb, size_t size);
		void * my_realloc(void * ptr, size_t size);
		void my_free(void * p);
};

#endif // MY_MALLOC_H
