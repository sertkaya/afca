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

#include "../fca/context.h"
#include "../bitset/bitset.h"

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
