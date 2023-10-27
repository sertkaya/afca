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
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "../fca/context.h"
#include "../bitset/bitset.h"
#include "stable_extensions_nourine.h"
#include "af.h"

/*
// In our case implications are unit implications.
// lhs is a bitset, rhs is an index
struct unit_implication {
	BitSet *lhs;
	int rhs;
};

typedef struct unit_implication UnitImplication;

// Implication set is just an array of implications
struct implication_set {
	int size;
	UnitImplication **elements;
};

typedef struct implication_set ImplicationSet;
*/

UnitImplication *create_unit_implication(BitSet *lhs, int rhs) {
	UnitImplication *imp = (UnitImplication*) calloc(1, sizeof(UnitImplication));
	assert(imp != NULL);
	imp->lhs = lhs;
	imp->rhs = rhs;

	return(imp);
}

void free_implication(UnitImplication *imp) {
	free_bitset(imp->lhs);
	free(imp);
}

void print_implication(UnitImplication *imp) {
	print_bitset(imp->lhs, stdout);
	printf("-> %d\n", imp->rhs);
}

void print_implication_set(ImplicationSet *imps) {
	int i;
	for (i = 0; i < imps->size; ++i)
		print_implication(imps->elements[i]);
}

ImplicationSet *create_implication_set() {
	ImplicationSet *imps = (ImplicationSet*) calloc(1, sizeof(ImplicationSet));
	assert(imps != NULL);
	imps->size = 0;
	imps->elements = NULL;

	return(imps);
}

void add_implication(UnitImplication *imp, ImplicationSet *imps) {
	UnitImplication** tmp;
	tmp = realloc(imps->elements, (imps->size + 1) * sizeof(UnitImplication*));
	assert(tmp != NULL);
	imps->elements = tmp;
	imps->elements[imps->size] = imp;
	++imps->size;
}

ImplicationSet *attacks_to_implications(Context *attacks) {
	ImplicationSet *imps = create_implication_set();
	Context *attacked_by = transpose_context(attacks);

	int i, j;
	for (i = 0; i < attacks->size; ++i) {
		for (j = 0; j < attacks->size; ++j) {
			if (TEST_BIT(attacks->a[i], j) && !TEST_BIT(attacks->a[j], i)) {
				BitSet *lhs = create_bitset(attacks->size);
				copy_bitset(attacked_by->a[i], lhs);

				UnitImplication *imp = create_unit_implication(lhs, j);
				add_implication(imp, imps);
				//

				/*
				int lhs_was_empty = bitset_is_emptyset(lhs);
				int k;
				for (k = 0; k < attacks->size; ++k) {
					if (TEST_BIT(attacks->a[i], k)) {
						RESET_BIT(lhs, k);
					}
				}
				UnitImplication *imp = create_unit_implication(lhs, j);
				if (lhs_was_empty) {
					add_implication(imp, imps);
				}
				else
					if (bitset_is_emptyset(lhs)) {
						continue;
					}
					else {
						add_implication(imp, imps);
					}
					*/
			}
		}
	}

	return(imps);
}

// Compute closure of x under imps and store in c
void naive_closure(BitSet *x, ImplicationSet *imps, BitSet *c) {
	int i;
	// TODO: optimize!
	copy_bitset(x, c);
	BitSet *tmp = create_bitset(x->size);
	do {
		copy_bitset(c, tmp);
		for (i = 0; i < imps->size; ++i) {
			if (bitset_is_subset(imps->elements[i]->lhs, c)) {
				SET_BIT(c, imps->elements[i]->rhs);
			}
		}
	} while (!bitset_is_equal(tmp, c));
}

void all_stable_extensions_nourine(Context* attacks, FILE *outfile) {
	Context* not_attacks = negate_context(attacks);

	BitSet* c = create_bitset(attacks->size);
	// c closure
	BitSet* cc = create_bitset(attacks->size);
	// c closure complement
	BitSet* ccc = create_bitset(attacks->size);
	// c closure complement up
	BitSet* ccc_up = create_bitset(attacks->size);
	BitSet* tmp = create_bitset(attacks->size);

	ImplicationSet *imps = attacks_to_implications(attacks);
	print_implication_set(imps);
	printf("\nImps size: %d\n\n", imps->size);

	int i, j, closure_count = 0, stable_extension_count = 0;

	// closure of the empty set
	naive_closure(c, imps, cc);
	copy_bitset(cc, c);

	complement_bitset(cc, ccc);
	up_arrow(not_attacks, ccc, ccc_up);

	if (bitset_is_equal(ccc, ccc_up)) {
		++stable_extension_count;
		print_bitset(ccc, outfile);
		fprintf(outfile, "\n");
	}

	while (!bitset_is_fullset(c)) {
		// print_bitset(c, stdout);
		// printf("\n");
		++closure_count;


		for (i = attacks->size - 1; i >= 0; --i) {
			if (TEST_BIT(c, i))
				RESET_BIT(c, i);
			else {
				reset_bitset(tmp);
				SET_BIT(c, i);

				// compute closure
				naive_closure(c, imps, cc);

				RESET_BIT(c, i);

				// TODO: optimize!
				bitset_set_minus(cc, c, tmp);
				char canonicity_test_passed = 1;
				for (j = 0; j < i; ++j) {
					// check if next \ current contains a bit larger than i
					if (TEST_BIT(tmp, j)) {
						canonicity_test_passed = 0;
						break;
					}
				}
				if (canonicity_test_passed) {
					complement_bitset(cc, ccc);
					up_arrow(not_attacks, ccc, ccc_up);

					if (bitset_is_equal(ccc, ccc_up)) {
						++stable_extension_count;
						print_bitset(ccc, outfile);
						fprintf(outfile, "\n");
						// printf("cut\n");
						// printf("\n");
						++closure_count;
						continue;
					}
					else if (is_conflict_free(attacks, ccc)) {
						// printf("cut\n");
						continue;
					}
					else
						break;
				}
			}
		}
		copy_bitset(cc, c);
	}

	printf("Number of closures generated: %d\n", closure_count);
	printf("Number of stable extensions: %d\n", stable_extension_count);

	free_bitset(tmp);
	free_bitset(c);
	free_bitset(cc);
	free_bitset(ccc);
	free_bitset(ccc_up);

	free_context(attacks);
	free_context(not_attacks);
}

void one_stable_extension_nourine(Context* attacks, FILE *outfile) {
	// TODO
}
