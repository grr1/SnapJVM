#ifndef PRINTING_H
#define PRINTING_H
#include "myMalloc.hpp"

class MallocPrinter {
	friend class MallocHeap;
	friend class MallocTester;

//	private:
	public:
		/* Define printFormatter to be a function pointer type taking a sinle parameter
		 * (a Header pointer) and returning void
		 *
		 * This allows the print freelist and tags functions to take various printing
		 * functions depending on the kind of output desired
		 *
		 * https://www.cprogramming.com/tutorial/function-pointers.html
		 */
		typedef void (*printFormatter)(MallocHeap*, MallocHeap::Header *);

//	public:
		/* Functions to print the freelist and boundary tag data structures taking a
		 * one of the above printing functions as a function pointer
		 */
		void freelist_print(MallocHeap* mHeap, MallocPrinter::printFormatter pf);
		void tags_print(MallocHeap* mHeap, MallocPrinter::printFormatter pf);

//	private:
		static constexpr char* malloc_color = "MALLOC_DEBUG_COLOR";

		/* Functions defining a format to print */
		void basic_print(MallocHeap* mHeap, MallocHeap::Header * block);
		void print_list(MallocHeap* mHeap, MallocHeap::Header * block);
		static const char* allocated_to_string(char allocated);
		static bool check_color();
		static void clear_color();
		static inline bool is_sentinel(MallocHeap* mHeap, void* p);
		static void print_color(MallocHeap* mHeap, MallocHeap::Header* block);
		static void print_object(MallocHeap* mHeap, MallocHeap::Header * block);
		static void print_status(MallocHeap* mHeap, MallocHeap::Header * block);

		/* Helpers */
		void print_sublist(MallocHeap* mHeap, MallocPrinter::printFormatter pf, MallocHeap::Header * start, MallocHeap::Header * end);
		static void print_pointer(MallocHeap* mHeap, void * p);
};

#endif // PRINTING_H
