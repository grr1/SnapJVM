#include <stdlib.h>
#include <bits/stc++.h>
#include "myMalloc.hpp"

int main() {
	mallocHeap* mHeap;
	std::vector<void*> mallocHeaders;

	for (int i = 0; i < 5; i++) {
		mallocHeaders.push_back(mHeap->my_malloc(64));
	}

	printf("Freelists before any freeing:\n");
	mHeap->freelist_print(print_object);
	printf("Tags before any freeing:\n");
	mHeap->tags_print(print_object);

	// creating a situation before coalescing with two neighbors
	mHeap->my_free(mallocHeaders.at(1));
	mHeap->my_free(mallocHeaders.at(3));

	printf("Freelists before double coalesce:\n");
	mHeap->freelist_print(print_object);
	printf("Tags before double coalesce:\n");
	mHeap->tags_print(print_object);

	mHeap->my_free(mallocHeaders.at(2));

	printf("Final state of freelists:\n");
	mHeap->freelist_print(print_object);
	printf("Final state of tags:\n");
	mHeap->tags_print(print_object);
}
