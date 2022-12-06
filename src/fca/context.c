/*
 * context.c
 *
 *  Created on: 22.11.2022
 *      Author: bs
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "context.h"
#include "../bitset/bitset.h"

Context* create_context() {
	Context *c = calloc(1, sizeof(Context));
	assert(c != NULL);

	c->size = 0;
	c->a = NULL;

	c->mask = NULL;

	return(c);
}

void init_context(Context* c, int size) {
	c->size = size;
	c->a = (BitSet**) calloc(size, sizeof(BitSet*));
	assert(c->a != NULL);
	int i;
	for (i = 0; i < size; ++i) {
		c->a[i] = create_bitset(size);
	}

	c->mask = (BitSet**) calloc(size, sizeof(BitSet*));
	assert(c->mask != NULL);
	int j;
	for (i = 0; i < size; ++i) {
		c->mask[i] = create_bitset(size);
		for (j = 0; j < size - i - 1 ; ++j)
			SET_BIT(c->mask[i], j);
	}
}

void print_context(Context* c) {
	int i;

	for (i = 0; i < c->size; ++i) {
		print_bitset(c->a[i]);
		printf("\n");
	}

	printf("\nMask:\n");
	for (i = 0; i < c->size; ++i) {
		print_bitset(c->mask[i]);
		printf("\n");
	}
}

void double_prime_attr_obj(Context* c, BitSet* bs, BitSet* r) {
	int i;

	// First fill r
	// TODO: Improve efficiency?
	for (i = 0; i < r->size; ++i)
		SET_BIT(r, i);

	for (i = 0; i < c->size; ++i) {
		if (is_subset(bs, c->a[i])) {
			intersection(r, c->a[i], r);
		}
	}
}

Context* negate_context(Context*c ){
	Context* nc = create_context();
	init_context(nc, c->size);

	int i;
	for (i = 0; i < c->size; ++i)
		negate_bitset(c->a[i], nc->a[i]);

	return(nc);
}
