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

#include "context.h"
#include "../bitset/bitset.h"

void next_intent(Context* c, BitSet* s, BitSet* ni) {
	int i,j;
	BitSet* tmp = create_bitset(s->size);

	for (i = c->size - 1; i >= 0; --i) {
		if (TEST_BIT(s, i))
			RESET_BIT(s, i);
		else {
			SET_BIT(s, i);
			down_up_arrow(c, s, ni);
			RESET_BIT(s, i);
			// TODO: optimize!
			bitset_set_minus(ni, s, tmp);
			char flag = 0;
			for (j = 0; j < i; ++j)
				if (TEST_BIT(tmp, j)) {
					flag = 1;
					break;
				}
			if (!flag)
				return;
		}
	}
}


void all_intents(Context* c) {
	BitSet* bs = create_bitset(c->size);
	BitSet* ni = create_bitset(c->size);

	while (!bitset_is_fullset(bs)) {
		next_intent(c, bs, ni);
		print_bitset(ni, stdout);
		copy_bitset(ni, bs);
		printf("\n");
	}
}
