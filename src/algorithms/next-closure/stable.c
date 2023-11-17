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

#include <stdio.h>
#include <inttypes.h>

#include "../af/af.h"
#include "../bitset/bitset.h"

// Compute the next conflict-free closure coming after "current" and store it in "next"
char next_closure(AF* not_attacks, AF* attacks, BitSet* current, BitSet* next) {
	int i,j;
	BitSet* tmp = create_bitset(attacks->size);

	for (i = not_attacks->size - 1; i >= 0; --i) {
		if (TEST_BIT(current, i))
			RESET_BIT(current, i);
		else {
			// check if i attacks i
			if (CHECK_ARG_ATTACKS_ARG(attacks,i, i))
				continue;

			// check if argument i attacks the set current
			if (CHECK_ARG_ATTACKS_SET(attacks, i, current))
				continue;

			// check if current attacks i
			if (check_set_attacks_arg(attacks, current, i))
				continue;

			// TODO: Compare what is more efficient:
			// The three checks above or adding "i" to "current" and checking
			// whether the result is conflict-free

			reset_bitset(tmp);
			SET_BIT(current, i);
			printf("current: ");
			print_bitset(current, stdout);
			printf("\n");

			// compute next
			down_up_arrow(not_attacks, current, next);
			printf("next: ");
			print_bitset(next, stdout);
			printf("\n");
			RESET_BIT(current, i);

			// TODO: optimize!
			bitset_set_minus(next, current, tmp);
			char canonicity_test_passed = 1;
			for (j = 0; j < i; ++j)
				// check if next \ current contains a bit larger than i
				if (TEST_BIT(tmp, j)) {
					canonicity_test_passed = 0;
					break;
				}
			if (canonicity_test_passed) {
				free_bitset(tmp);
				return(1);
			}
		}
	}
	free_bitset(tmp);
	return(0);
}

void stable_extensions_nc(AF *attacks, FILE *outfile) {

	AF *not_attacks = complement_argumentation_framework(attacks);

	BitSet* tmp = create_bitset(attacks->size);
	BitSet* c = create_bitset(attacks->size);
	// up-arrow of c
	BitSet* c_up = create_bitset(attacks->size);

	int concept_count = 0, stable_extension_count = 0;

	print_argumentation_framework(not_attacks);
	printf("\n");
	while (1) {
		print_bitset(c, stdout);
		printf("-\n");
		if (!next_closure(not_attacks, attacks, tmp, c))
			break;
		++concept_count;
		// up-arrow of c
		up_arrow(not_attacks, c, c_up);

		if (bitset_is_equal(c, c_up)) {
			++stable_extension_count;
			print_bitset(c, outfile);
			fprintf(outfile, "\n");
		}
		copy_bitset(c, tmp);
	}
	printf("Number of concepts generated: %d\n", concept_count);
	printf("Number of stable extensions: %d\n", stable_extension_count);

	free_bitset(tmp);
	free_bitset(c);
	free_bitset(c_up);

	free_argumentation_framework(not_attacks);
}

void one_stable_extension_nc(AF* attacks, FILE *outfile) {

	AF* not_attacks = complement_argumentation_framework(attacks);

	BitSet* tmp = create_bitset(attacks->size);
	BitSet* c = create_bitset(attacks->size);
	BitSet* c_up = create_bitset(attacks->size);

	int concept_count = 0;

	while (1) {
		if (!next_closure(not_attacks, attacks, tmp, c))
			break;
		++concept_count;

		up_arrow(not_attacks, c, c_up);

		if (bitset_is_equal(c, c_up)) {
			print_bitset(c, outfile);
			fprintf(outfile, "\n");
			break;
		}
		copy_bitset(c, tmp);
	}
	printf("Number of concepts generated: %d\n", concept_count);

	free_bitset(tmp);
	free_bitset(c);
	free_bitset(c_up);

	free_argumentation_framework(not_attacks);
}
