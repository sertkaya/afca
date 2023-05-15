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

void all_preferred_extensions_nc(Context* attacks, FILE *outfile) {
	Context* not_attacks = negate_context(attacks);

	BitSet* bs = create_bitset(attacks->size);
	BitSet* ni = create_bitset(attacks->size);
	BitSet* tmp_up = create_bitset(attacks->size);
	BitSet* tmp_down = create_bitset(attacks->size);

	int closure_count = 0, preferred_extension_count = 0;

	while (1) {
		if (!next_closure(not_attacks, attacks, bs, ni))
			break;
		++closure_count;
		// printf("*");
		// print_bitset(ni, stdout);
		// printf("\n");

		up_arrow(not_attacks, ni, tmp_up);
		down_arrow(not_attacks, ni, tmp_down);
		// TODO: add the maximality condition!
		if (bitset_is_subset(tmp_up, tmp_down)) {
			++preferred_extension_count;
			print_bitset(ni, outfile);
			fprintf(outfile, "\n");
		}
		copy_bitset(ni, bs);
	}
	printf("Number of closures generated: %d\n", closure_count);
	printf("Number of preferred extensions: %d\n", preferred_extension_count);

	free_bitset(bs);
	free_bitset(ni);
	free_bitset(tmp_up);
	free_bitset(tmp_down);

	free_context(attacks);
	free_context(not_attacks);
}
