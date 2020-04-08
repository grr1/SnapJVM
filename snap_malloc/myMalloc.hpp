#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <stdbool.h>
#include <sys/types.h>
#include <cstddef>

class MallocHeap {
	friend class MallocPrinter;
	friend class MallocTester;

	public:
		MallocHeap();
		~MallocHeap();

		/* Due to the way assert() prints error messges we use out own assert function
		 * for deteminism when testing assertions
		 */
		inline void mAssert(int e);

		/* Size of the Header for an allocated block
		 *
		 * The size of the normal minus the size of the two free list pointers as
		 * they are only maintained while block is free
		 */
		static int getAllocHeaderSize() {
			return (sizeof(MallocHeap::Header) - (2 * sizeof(MallocHeap::Header *)));
		}

		// Malloc interface
		void * my_malloc(std::size_t size);
		void * my_calloc(std::size_t nmemb, std::size_t size);
		void * my_realloc(void * ptr, std::size_t size);
		void my_free(void * p);

//	private:
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
		  state_unallocated = 0,
		  state_allocated = 1,
		  state_fencepost = 2,
		};

		enum {
			use_relative_pointers = 1,
			arena_size = 4096,
			n_lists = 59,
			min_allocation = 8,		/* The minimum size request the allocator will service */
			max_os_chunks = 1024
		};

		bool verify();

	private:
		// Helper functions for getting and storing size and state from Header
		// Since the size is a multiple of 8, the last 3 bits are always 0s.
		// Therefore we use the 3 lowest bits to store the state of the object.
		// This is going to save 8 bytes in all objects.
		inline size_t get_block_size(MallocHeap::Header * h);
		inline void set_block_size(MallocHeap::Header * h, size_t size);
		inline enum MallocHeap::state get_block_state(MallocHeap::Header *h);
		inline void set_block_state(MallocHeap::Header * h, enum MallocHeap::state s);
		inline void set_block_size_and_state(MallocHeap::Header * h, std::size_t size, enum MallocHeap::state s);

		// Manage accessing and mutatig the freelist bitmap
		inline void set_bit(std::size_t i);
		inline void unset_bit(std::size_t i);
		inline bool get_bit(std::size_t i);
		inline size_t get_next_set_bit(std::size_t i);
		inline size_t size_to_index(std::size_t size);

		// Helper functions for manipulating pointers to Headers
		inline MallocHeap::Header * get_header_from_offset(void * ptr, ptrdiff_t off);
		inline MallocHeap::Header * get_left_header(MallocHeap::Header * h);
		inline MallocHeap::Header * ptr_to_header(void * p);

		// Helper functions for freeing a block
		inline void move_coalesced(MallocHeap::Header * block, std::size_t oldSize);
		inline void coalesce_left(MallocHeap::Header * left, MallocHeap::Header * block, MallocHeap::Header * right);
		inline void coalesce_right(MallocHeap::Header * block, MallocHeap::Header * right);
		inline void coalesce_both(MallocHeap::Header * left, MallocHeap::Header * block, MallocHeap::Header * right);
		inline void free_list_deallocate(MallocHeap::Header * block);
		inline void deallocate_object(void * p);

		// Helper functions for allocating more memory from the OS
		inline void initialize_fencepost(MallocHeap::Header * fp, std::size_t left_size);
		inline void insert_os_chunk(MallocHeap::Header * hdr);
		inline void insert_fenceposts(void * raw_mem, std::size_t size);
		MallocHeap::Header * allocate_chunk(std::size_t size);

		// Helper functions for allocating a block
		MallocHeap::Header * get_freelist_sentinel(std::size_t size);
		MallocHeap::Header * get_populated_freelist_sentinel(std::size_t size);
		inline MallocHeap::Header * split_block(MallocHeap::Header * block, std::size_t size);
		inline MallocHeap::Header * free_list_get_allocable(std::size_t size);
		inline MallocHeap::Header * allocate_object(std::size_t raw_size);

		// Helper functions for manipulating the freelist
		inline void freelist_remove(MallocHeap::Header * block);
		inline void freelist_insert_between(MallocHeap::Header * block, MallocHeap::Header * prev, MallocHeap::Header * next);
		inline void freelist_insert(MallocHeap::Header * block);

		// Helper to find a block's right neighbor
		MallocHeap::Header * get_right_header(MallocHeap::Header * h);

		/*
		 * Mutex to ensure thread safety for the freelist
		 */
		pthread_mutex_t mutex;

		/*
		 * Array of sentinel nodes for the freelists
		 */
		MallocHeap::Header freelistSentinels[n_lists];

		/*
		 * bitmap representing which of the freelists have available blocks. This
		 * allows for an optimized block lookup. By finding the index in the
		 * bitmap corresponding to the current request size and then finding the next
		 * set bit in the bitmap we can determine the location of the block to allocate
		 *
		 * By using the bitmap we only need to load a small number of bytes into cache
		 * to perform this lookup operation instead of checking each of the sentinels.
		 */
		char freelist_bitmap[(n_lists | 7) >> 3];

		/*
		 * Pointer to the second fencepost in the most recently allocated chunk from
		 * the OS. Used for coalescing chunks
		 */
		MallocHeap::Header * lastFencePost;

		/*
		 * Pointer to maintian the base of the heap to allow printing based on the
		 * distance from the base of the heap
		 */
		void * base;

		MallocHeap::Header * osChunkList [max_os_chunks];
		std::size_t numOsChunks;

		// Helper functions for verifying that the data structures are structurally valid
		inline MallocHeap::Header * detect_cycles();
		inline MallocHeap::Header * verify_pointers();
		inline bool verify_freelist();
		inline MallocHeap::Header * verify_chunk(MallocHeap::Header * chunk);
		inline bool verify_tags();

		// Debug list verification
//		bool verify();
};

#endif // MY_MALLOC_H
