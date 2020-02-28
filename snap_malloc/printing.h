#ifndef PRINTING_H
#define PRINTING_H
#include "myMalloc.hpp"

class mallocPrinter {
	public:
		/* Functions to print the freelist and boundary tag data structures taking a
		 * one of the above printing functions as a function pointer
		 */
		void freelist_print(printFormatter pf);
		void tags_print(printFormatter pf);

	private:
		/* Define printFormatter to be a function pointer type taking a sinle parameter
		 * (a Header pointer) and returning void
		 *
		 * This allows the print freelist and tags functions to take various printing
		 * functions depending on the kind of output desired
		 *
		 * https://www.cprogramming.com/tutorial/function-pointers.html
		 */
		typedef void (*printFormatter)(Header *);

		/* Functions defining a format to print */
		void basic_print(MallocHeap* mHeap, Header * block);
		void print_list(MallocHeap* mHeap, Header * block);
		const char* allocated_to_string(char allocated);
		bool check_color();
		void clear_color();
		inline bool is_sentinel(MallocHeap* mHeap, void* p);
		void print_color(MallocHeap* mHeap, MallocHeap::Header* block);
		void print_object(MallocHeap* mHeap, Header * block);
		void print_status(MallocHeap* mHeap, Header * block);

		/* Helpers */
		void print_sublist(printFormatter pf, Header * start, Header * end);
		void print_pointer(MallocHeap* mHeap, void * p);
};

#endif // PRINTING_H
