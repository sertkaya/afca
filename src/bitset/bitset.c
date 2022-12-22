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

BitSet* create_bitset(int size) {

	int base_count = (size / BITSET_BASE_SIZE);
	if (size % BITSET_BASE_SIZE)
		++base_count;

	BitSet* bs = (BitSet*) calloc(1,  sizeof(BitSet));
	assert(bs != NULL);

	bs->base_count = base_count;
	bs->size = size;
	bs->a = (BITSET_BASE_TYPE*) calloc(base_count,  BITSET_BASE_SIZE / 8);
	assert(bs->a != NULL);

	return(bs);
}

void print_bitset(BitSet* bs) {
	int i;
	for (i = 0; i < bs->size; ++i)
		if (TEST_BIT(bs, i))
			printf("%d", 1);
		else
			printf("%d", 0);
}

char is_subset(BitSet* bs1, BitSet* bs2) {
	int i;

	for (i = 0; i < bs1->base_count; ++i)
		if (bs1->a[i] != (bs1->a[i] & bs2->a[i]))
			return(0);
	return(1);
}

char is_set_equal(BitSet* bs1, BitSet* bs2) {
	int i;

	for (i = 0; i < bs1->base_count; ++i)
		if (bs1->a[i] != bs2->a[i])
			return(0);
	return(1);
}

void intersection(BitSet* bs1, BitSet* bs2, BitSet* r) {
	int i;
	for (i = 0; i < bs1->base_count; ++i)
		r->a[i] = bs1->a[i] & bs2->a[i];
}

void negate_bitset(BitSet* bs, BitSet* r) {
	int i;
	for (i = 0; i < bs->base_count; ++i)
		r->a[i] = ~(bs->a[i]);
}

void set_minus(BitSet* bs1, BitSet* bs2, BitSet* r) {
	int i;
	for (i = 0; i < bs1->base_count; ++i)
		r->a[i] = bs1->a[i] & ~(bs2->a[i]);
}

void copy_bitset(BitSet* bs1, BitSet* bs2) {
	int i;
	for (i = 0; i < bs1->base_count; ++i)
		bs2->a[i] = bs1->a[i];
}

// TODO: optimize!
char is_fullset(BitSet* bs) {
	int i;
	for (i = 0; i < bs->size; ++i)
		if (!(TEST_BIT(bs, i)))
			return 0;
	return(1);
}

char is_emptyset(BitSet* bs) {
	int i;

	for (i = 0; i < bs->base_count; ++i)
		if (bs->a[i] != 0UL)
			return(0);
	return(1);
}

void reset_bitset(BitSet* bs) {
	int i;

	for (i = 0; i < bs->base_count; ++i)
		bs->a[i] = 0UL;
}
