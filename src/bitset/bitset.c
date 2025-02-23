/*
 * AFCA - argumentation framework using closed sets
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
#include <math.h>
#include <assert.h>

BitSet* create_bitset(SIZE_TYPE size) {

	BitSet* bs = (BitSet*) calloc(1,  sizeof(BitSet));
	assert(bs != NULL);

	bs->size = size;
	bs->base_count = (int) ceilf((double) bs->size / BITSET_BASE_SIZE);
	bs->elements = (BITSET_BASE_TYPE*) calloc(bs->base_count,  sizeof(BITSET_BASE_TYPE));
	assert(bs->elements != NULL);

	return(bs);
}

int free_bitset(BitSet* bs) {
	int freed_bytes = bs->base_count * sizeof(BITSET_BASE_TYPE);
	free(bs->elements);
	free(bs);
	return(freed_bytes);
}

void print_bitset(BitSet* bs, FILE *outfile) {
	for (SIZE_TYPE i = 0; i < bs->size; ++i)
		if (TEST_BIT(bs, i))
			fprintf(outfile, "%d", 1);
		else
			fprintf(outfile, "%d", 0);
}

void print_set(BitSet* bs, FILE *outfile, const char *end) {
	fprintf(outfile, "[ ");
	for (SIZE_TYPE i = 0; i < bs->size; ++i)
		if (TEST_BIT(bs, i))
			fprintf(outfile, "%d ", i+1);
	fprintf(outfile, "]%s", end);
}

// TODO: optimize!
// int bitset_get_length(AF *af, BitSet* bs) {
// 	int i, l = 0;
// 	for (i = 0; i < af->size; ++i)
// 		if (TEST_BIT(bs, i))
// 			++l;
// 	return(l);
// }
