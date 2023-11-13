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

#include <stdio.h>
#include <inttypes.h>
#include <math.h>

#include "../af.h"
#include "../bitset/bitset.h"

AF* create_argumentation_framework(int size) {
	AF *af = calloc(1, sizeof(AF));
	assert(af != NULL);

	af->size = size;
	af->bitset_base_count = (int) ceilf((double) size / BITSET_BASE_SIZE);

	af->graph = (BitSet**) calloc(size, sizeof(BitSet*));
	assert(af->graph != NULL);

	int i;
	for (i = 0; i < size; ++i) {
		af->graph[i] = create_bitset(af);
	}

	return(af);
}

int free_argumentation_framework(AF* af) {
	int i, freed_bytes = 0;
	for (i = 0; i < af->size; ++i) {
		freed_bytes += free_bitset(af, af->graph[i]);
	}
	free(af->graph);
	freed_bytes += (af->size * sizeof(BitSet*));
	free(af);
	freed_bytes += sizeof(AF);
	return(freed_bytes);
}

int is_conflict_free(Context* attacks, BitSet* x) {
	BitSet* x_attacks = create_bitset(attacks->size);
	BitSet* r = create_bitset(attacks->size);
	int i;
	for (i = 0; i < attacks->size; ++i) {
		if (TEST_BIT(x, i))
			bitset_union(x_attacks, attacks->a[i], x_attacks);
	}
	// printf("x: ");
	// print_bitset(x, stdout);
	// printf("\t");
	// printf("x_attacks: ");
	// print_bitset(x_attacks, stdout);
	// printf("\t");
	bitset_intersection(x, x_attacks, r);
	if (bitset_is_emptyset(r))
		return(1);
	return(0);
}

void print_argumentation_framework(AF* af) {
	int i;

	for (i = 0; i < af->size; ++i) {
		print_bitset(af->graph[i], stdout);
		printf("\n");
	}
}

// Victims of bs (up-arrow)
void get_victims(AF* af, BitSet* bs, BitSet* r) {
	int i;

	// First fill r
	// TODO: Improve efficiency?
	// for (i = 0; i < af->size; ++i)
	// 	SET_BIT(r, i);
	set_bitset(af, r);


	for (i = 0; i < af->size; ++i)
		if (TEST_BIT(bs, i))
			bitset_intersection(r, af->graph[i], r);
}

// Attackers of bs (downarrow)
void get_attackers(AF* af, BitSet* bs, BitSet* r) {
	int i;

	// TODO: Improve efficiency?
	// for (i = 0; i < r->size; ++i)
	// 	RESET_BIT(r, i);
	reset_bitset(af, r);

	for (i = 0; i < af->size; ++i)
		if (bitset_is_subset(bs, af->graph[i]))
			SET_BIT(r, i);
}

AF* complement_argumentation_framework(AF *af ){
	AF* c_af = create_argumentation_framework(af->size);

	int i;
	for (i = 0; i < af->size; ++i)
		complement_bitset(af->graph[i], c_af->graph[i]);

	return(c_af);
}

AF* transpose_argumentation_framework(AF *af) {
	AF* t_af = create_argumentation_framework(af->size)

	int i,j;
	for (i = 0; i < af->size; ++i)
		for (j = 0; j < af->size; ++j)
			if (TEST_BIT(af->graph[i], j))
				SET_BIT(t_af->graph[j], i);
	return(t_af);
}
