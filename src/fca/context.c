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

struct index_value {
	int index; // the index of the object
	double value; // number of crosses for this object
};

int cmp(const void *v1, const void *v2) {
	if ((((struct index_value*) v1)-> value) > (((struct index_value*) v2)-> value))
		return(1);
	else if ((((struct index_value*) v2)-> value) > (((struct index_value*) v1)-> value))
		return(-1);
	else
		return(0);
}

Context* sort_context(Context *c) {
	Context *sc = create_context();
	init_context(sc, c->size);

	// find the number of crosses for each object, store in a struct
	struct index_value *index_value_pairs = calloc(c->size, sizeof(struct index_value));
	assert(index_value_pairs != NULL);

	int i,j;

	// number of attacks
	/*
	for (i = 0; i < c->size; ++i) {
		index_value_pairs[i].index = i;
		index_value_pairs[i].value = bitset_get_length(c->a[i]);
	}
	*/


	// number of attacked_by
	for (i = 0; i < c->size; ++i) {
		index_value_pairs[i].index = i;
		index_value_pairs[i].value = 0;
		for (j = 0; j < c->size; ++j)
			if (TEST_BIT(c->a[j], i))
				++index_value_pairs[i].value;
	}

	/*
	int attacks_count = 0, attacked_by_count = 0;
	for (i = 0; i < c->size; ++i) {
		index_value_pairs[i].index = i;
		attacks_count = bitset_get_length(c->a[i]);
		attacked_by_count = 0;
		for (j = 0; j < c->size; ++j)
			if (TEST_BIT(c->a[j], i))
				++attacked_by_count;

		if (attacks_count == 0)
			attacks_count = 1;
		// index_value_pairs[i].value = ((double) attacks_count) / (0.5 * attacked_by_count);
		index_value_pairs[i].value = ((double) attacked_by_count) / attacks_count;
	}
	*/

	// sort the index-value pairs according to value
	qsort(index_value_pairs, c->size, sizeof(index_value_pairs[0]), cmp);


	// fill in the new context sorted
	for (i = 0; i < c->size; ++i) {
		for (j = 0; j < c->size; ++j) {
			if (TEST_BIT(c->a[index_value_pairs[i].index], index_value_pairs[j].index))
				SET_BIT(sc->a[i], j);
		}
	}

	// for (i = 0; i < c->size; ++i)
	//  	printf("%d %d %lf\n", i, index_value_pairs[i].index, index_value_pairs[i].value);

	return(sc);
}

/*
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
*/


