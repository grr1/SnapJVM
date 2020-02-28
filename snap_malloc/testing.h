#ifndef TESTING_H
#define TESTING_H

#include "myMalloc.hpp"

class mallocTester : private MallocHeap {
	MallocHeap* mHeap;

	void initialize_test(const char* name);
	void finalize_test();

	void * malloc_and_clear(size_t size);
	void ** mallocing_loop(void ** array, size_t size, size_t n, printFormatter pf, bool silent);
	void * mallocing(size_t size, printFormatter pf, bool silent);

	void check_and_free(char * p, size_t size);
	void freeing_loop(void ** array, size_t size, size_t n, printFormatter pf, bool silent);
	void freeing(void * p, size_t size, printFormatter pf, bool silent);
};

#endif // TESTING_H
