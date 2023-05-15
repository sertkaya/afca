/*
 * AFCA - argumentation framework using closed sets
 *
 * Copyright (C) Baris Sertkaya (sertkaya@fb2.fra-uas.de)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

	c->singletons = NULL;

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

	c->singletons = (BitSet**) calloc(size, sizeof(BitSet*));
	assert(c->singletons != NULL);
	for (i = 0; i < size; ++i) {
		c->singletons[i] = create_bitset(size);
		SET_BIT(c->singletons[i], i);
	}
}

void free_context(Context* c) {
	int i;
	for (i = 0; i < c->size; ++i) {
		free_bitset(c->a[i]);
		free_bitset(c->singletons[i]);
	}
	free(c->a);
	free(c->singletons);
	free(c);
}


void print_context(Context* c) {
	int i;

	for (i = 0; i < c->size; ++i) {
		print_bitset(c->a[i], stdout);
		printf("\n");
	}

	printf("\nSingletons:\n");
	for (i = 0; i < c->size; ++i) {
		print_bitset(c->singletons[i], stdout);
		printf("\n");
	}
}

void down_up_arrow(Context* c, BitSet* bs, BitSet* r) {
	int i;

	// First fill r
	// TODO: Improve efficiency?
	for (i = 0; i < r->size; ++i)
		SET_BIT(r, i);

	for (i = 0; i < c->size; ++i) {
		if (bitset_is_subset(bs, c->a[i])) {
			bitset_intersection(r, c->a[i], r);
		}
	}
}

void up_arrow(Context* c, BitSet* bs, BitSet* r) {
	int i;

	// First fill r
	// TODO: Improve efficiency?
	for (i = 0; i < r->size; ++i)
		SET_BIT(r, i);

	for (i = 0; i < c->size; ++i)
		if (TEST_BIT(bs, i))
			bitset_intersection(r, c->a[i], r);
}

// downarrow
void down_arrow(Context* c, BitSet* bs, BitSet* r) {
	int i;

	// First reset r
	// TODO: Improve efficiency?

	for (i = 0; i < r->size; ++i)
		RESET_BIT(r, i);

	for (i = 0; i < c->size; ++i)
		if (bitset_is_subset(bs, c->a[i]))
			SET_BIT(r, i);
}

Context* negate_context(Context *c ){
	Context* nc = create_context();
	init_context(nc, c->size);

	int i;
	for (i = 0; i < c->size; ++i)
		negate_bitset(c->a[i], nc->a[i]);

	return(nc);
}

void reducible_objects(Context *c) {
	int i,j,k;
	BitSet* tmp = create_bitset(c->size);


	printf("Reducible objects:\n");
	for (i = 0; i < c->size; ++i) {
		// First fill tmp
		// TODO: Improve efficiency?
		for (k = 0; k < tmp->size; ++k)
			SET_BIT(tmp, k);

		for (j = 0; (j < c->size) && (i != j); ++j) {
			if (bitset_is_subset(c->a[i], c->a[j])) {
				bitset_intersection(tmp, c->a[j], tmp);
			}
		}
		if (bitset_is_equal(tmp, c->a[i]))
			printf("%d reducible\n", i);
	}
}



