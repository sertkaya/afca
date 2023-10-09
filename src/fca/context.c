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
}

void free_context(Context* c) {
	int i;
	for (i = 0; i < c->size; ++i) {
		free_bitset(c->a[i]);
	}
	free(c->a);
	free(c);
}


void print_context(Context* c) {
	int i;

	for (i = 0; i < c->size; ++i) {
		print_bitset(c->a[i], stdout);
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

Context* transpose_context(Context *c) {
	Context* tc = create_context();
	init_context(tc, c->size);

	int i,j;
	for (i = 0; i < c->size; ++i)
		for (j = 0; j < c->size; ++j)
			if (TEST_BIT(c->a[i], j))
				SET_BIT(tc->a[j], i);
	return(tc);
}
