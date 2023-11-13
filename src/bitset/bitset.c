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

#include "bitset.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <assert.h>

BitSet* create_bitset(AF *af) {

	BitSet* bs = (BitSet*) calloc(1,  sizeof(BitSet));
	assert(bs != NULL);

	bs->elements = (BITSET_BASE_TYPE*) calloc(af->bitset_base_count,  sizeof(BITSET_BASE_TYPE));
	assert(bs->elements != NULL);

	return(bs);
}

int free_bitset(AF* af, BitSet* bs) {
	int freed_bytes = af->bitset_base_count * sizeof(BITSET_BASE_TYPE);
	free(bs->elements);
	free(bs);
}

void print_bitset(AF *af, BitSet* bs, FILE *outfile) {
	int i;
	for (i = 0; i < af->size; ++i)
		if (TEST_BIT(bs, i))
			fprintf(outfile, "%d", 1);
		else
			fprintf(outfile, "%d", 0);
}

char bitset_is_subset(AF *af, BitSet* bs1, BitSet* bs2) {
	int i;

	for (i = 0; i < af->bitset_base_count; ++i)
		if (bs1->elements[i] != (bs1->elements[i] & bs2->elements[i]))
			return(0);
	return(1);
}

char bitset_is_equal(AF *af, BitSet* bs1, BitSet* bs2) {
	int i;

	for (i = 0; i < af->bitset_base_count; ++i)
		if (bs1->elements[i] != bs2->elements[i])
			return(0);
	return(1);
}

void bitset_intersection(AF *af, BitSet* bs1, BitSet* bs2, BitSet* r) {
	int i;
	for (i = 0; i < af->bitset_base_count; ++i)
		r->elements[i] = bs1->elements[i] & bs2->elements[i];
}

void bitset_union(AF *af, BitSet* bs1, BitSet* bs2, BitSet* r) {
	int i;
	for (i = 0; i < af->bitset_base_count; ++i)
		r->elements[i] = bs1->elements[i] | bs2->elements[i];
}

/*
void negate_bitset(BitSet* bs, BitSet* r) {
}
*/
void complement_bitset(AF *af, BitSet* bs, BitSet* r) {
	// TODO: temporarily
	// int i;
	// for (i = 0; i < bs->size; ++i)
	// 	if (TEST_BIT(bs,i))
	// 		RESET_BIT(r,i);
	// 	else
	// 		SET_BIT(r, i);
	int i;
	for (i = 0; i < af->bitset_base_count; ++i)
		r->elements[i] = ~(bs->elements[i]);
}

void bitset_set_minus(AF *af, BitSet* bs1, BitSet* bs2, BitSet* r) {
	int i;
	reset_bitset(r);
	for (i = 0; i < af->bitset_base_count; ++i)
	 	r->elements[i] = bs1->elements[i] & ~(bs2->elements[i]);
}

void copy_bitset(AF *af, BitSet* bs1, BitSet* bs2) {
	int i;
	for (i = 0; i < af->bitset_base_count; ++i)
		bs2->elements[i] = bs1->elements[i];
}

// TODO: optimize!
char bitset_is_fullset(AF *af, BitSet* bs) {
	int i;
	for (i = 0; i < af->size; ++i)
		if (!(TEST_BIT(bs, i)))
			return 0;
	return(1);
}

char bitset_is_emptyset(AF *af, BitSet* bs) {
	int i;

	for (i = 0; i < af->bitset_base_count; ++i)
		if (bs->elements[i] != 0UL)
			return(0);
	return(1);
}

void reset_bitset(AF *af, BitSet* bs) {
	int i;

	for (i = 0; i < af->bitset_base_count; ++i)
		bs->elements[i] = 0UL;
}

void set_bitset(AF *af, BitSet* bs) {
	int i;

	for (i = 0; i < af->bitset_base_count; ++i)
		bs->elements[i] = 1UL;
}

// TODO: optimize!
// int bitset_get_length(AF *af, BitSet* bs) {
// 	int i, l = 0;
// 	for (i = 0; i < af->size; ++i)
// 		if (TEST_BIT(bs, i))
// 			++l;
// 	return(l);
}
