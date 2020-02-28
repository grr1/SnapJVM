#ifndef PRINTING_H
#define PRINTING_H
#include "myMalloc.hpp"

class mallocPrinter : private mallocHeap {
	public:
		/* Functions to print the freelist and boundary tag data structures taking a
		 * one of the above printing functions as a function pointer
		 */
		void freelist_print(printFormatter pf);
		void tags_print(printFormatter pf);

	private:
		/* Define printFormatter to be a function pointer type taking a sinle parameter
		 * (a header pointer) and returning void
		 *
		 * This allows the print freelist and tags functions to take various printing
		 * functions depending on the kind of output desired
		 *
		 * https://www.cprogramming.com/tutorial/function-pointers.html
		 */
		typedef void (*printFormatter)(header *);

		/* Functions defining a format to print */
		void basic_print(mallocHeap* mHeap, header * block);
		void print_list(mallocHeap* mHeap, header * block);
		const char* allocated_to_string(char allocated);
		bool check_color();
		void clear_color();
		inline bool is_sentinel(mallocHeap* mHeap, void* p);
		void print_color(mallocHeap* mHeap, header* block);
		void print_object(mallocHeap* mHeap, header * block);
		void print_status(mallocHeap* mHeap, header * block);

		/* Helpers */
		void print_sublist(printFormatter pf, header * start, header * end);
		void print_pointer(mallocHeap* mHeap, void * p);
};

#endif // PRINTING_H
