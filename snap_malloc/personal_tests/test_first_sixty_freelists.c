#include <stdlib.h>
#include <bits/stdc++.h>
#include "myMalloc.hpp"

int main() {
	mallocHeap* mHeap;
	std::vector<void*> mallocHeaders;
	int numLists = 60;

	for (int i = 0; i < numLists*8; i+=8) {
		printf("Mallocing %d bytes...\n", i);
		mallocHeaders.push_back(mHeap->my_malloc(i)); // first three blocks should be 32 bytes
	}

	printf("Final state of freelist:\n");
	mHeap->freelist_print(print_object);
	printf("Final tags:\n");
	mHeap->tags_print(print_object);

	for (int i = 0; i < numLists; i++) {
		mHeap->my_free(mallocHeaders.at(i));
	}
}
