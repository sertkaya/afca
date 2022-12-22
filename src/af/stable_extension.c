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

char next_cf_closure(Context* not_attacks, Context* attacks, BitSet* s, BitSet* ni) {
	int i,j;
	BitSet* tmp = create_bitset(s->size);

	for (i = not_attacks->size - 1; i >= 0; --i) {
		if (TEST_BIT(s, i))
			RESET_BIT(s, i);
		else {
			intersection(s, attacks->a[i], tmp);
			// check if argument i attacks the set s
			if (!is_emptyset(tmp))
				continue;
			// check if s attacks i
			char flag = 0;
			for (j = 0; j < s->size; ++j) {
				if (TEST_BIT(s,j) && TEST_BIT(attacks->a[j], i)) {
					flag = 1;
					break;
				}
			}
			if (flag)
				continue;


			reset_bitset(tmp);
			SET_BIT(s, i);
			double_prime_attr_obj(not_attacks, s, ni);
			RESET_BIT(s, i);
			// TODO: optimize!
			set_minus(ni, s, tmp);
			flag = 0;
			for (j = 0; j < i; ++j)
				if (TEST_BIT(tmp, j)) {
					flag = 1;
					break;
				}
			if (!flag)
				return(1);
			// if (tmp <= c->singletons[i]) {
			// 	printf("\nyes\n");
			// 	return;
			// }
		}
	}
	return(0);
}


void all_stable_extensions(Context* attacks) {
	BitSet* bs = create_bitset(attacks->size);
	BitSet* ni = create_bitset(attacks->size);
	BitSet* tmp = create_bitset(attacks->size);

	Context* not_attacks = negate_context(attacks);

	printf("Conflict-free closures:\n");
	while (1) {
		if (!next_cf_closure(not_attacks, attacks, bs, ni))
			break;
		prime_obj_attr(not_attacks, ni, tmp);
		if (is_set_equal(ni, tmp)) {
			print_bitset(ni);
			printf("\n");
		}
		copy_bitset(ni, bs);
	}
}
