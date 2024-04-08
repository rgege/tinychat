#include <stdio.h>
#include <stdlib.h>

void *tinyMalloc(size_t size) {
	void *p = malloc(size);
	if (p == NULL) {
		perror("memory allocation failed");
		exit(EXIT_FAILURE);
	}
	return p;
}
