/*
 * context.c
 *
 *  Created on: 22.11.2022
 *      Author: bs
 */

#include <stdlib.h>
#include <assert.h>

#include "context.h"
#include "../bitset/bitset.h"

Context* create_context() {
	Context *c = calloc(1, sizeof(Context));
	assert(c != NULL);

	c->size = 0;
	c->a = NULL;

	return(c);
}

void init_context(Context* c, int size) {
	c->size = size;
	c->a = calloc(size, sizeof(BitSet*));
	assert(c->a != NULL);
	int i;
	for (i = 0; i < size; ++i) {
		c->a[i] = create_bitset(size);
	}
}
