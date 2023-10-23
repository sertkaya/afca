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

	BitSet* current = create_bitset(attacks->size);
	// next closure
	BitSet* next = create_bitset(attacks->size);
	BitSet* complement_next = create_bitset(attacks->size);
	BitSet* next_complement_up = create_bitset(attacks->size);
	BitSet* tmp = create_bitset(attacks->size);

	ImplicationSet *imps = attacks_to_implications(attacks);
	print_implication_set(imps);
	printf("\nImps size: %d\n\n", imps->size);

	int i, j, closure_count = 0, stable_extension_count = 0;

	// closure of the empty set
	naive_closure(tmp, imps, current);
	++closure_count;

	while (!bitset_is_fullset(current)) {

		for (i = attacks->size - 1; i >= 0; --i) {
			if (TEST_BIT(current, i))
				RESET_BIT(current, i);
			else {
				reset_bitset(tmp);
				SET_BIT(current, i);

				// compute closure
				naive_closure(current, imps, next);
				++closure_count;

				RESET_BIT(current, i);

				// complement of next
				complement_bitset(next, complement_next);

				// TODO: optimize!
				bitset_set_minus(next, current, tmp);
				char flag = 0;
				for (j = 0; j < i; ++j) {
					// check if next \ current contains a bit larger than i
					if (TEST_BIT(tmp, j)) {
						flag = 1;
						break;
					}
				}
				if (!flag) {
					// next \ current does not contain bit larger than i
					// next is the next closure
					// break the for loop with counter i
					// return(1);
					break;
				}

				// check if complement of next is a stable extension
				up_arrow(not_attacks, complement_next, next_complement_up);
				if (bitset_is_equal(complement_next, next_complement_up)) {
					++stable_extension_count;
					print_bitset(complement_next, outfile);
					fprintf(outfile, "\n");
					continue;
				}

				// check if the complement of next is conflict free
				// in this case we cut this branch
				if (is_conflict_free(attacks, complement_next)) {
					continue;
				}

			}
		}

		if (i != -1)
			copy_bitset(next, current);
		else
			break;
	}

	printf("Number of closures generated: %d\n", closure_count);
	printf("Number of stable extensions: %d\n", stable_extension_count);

	free_bitset(tmp);
	free_bitset(current);
	free_bitset(next);
	free_bitset(complement_next);
	free_bitset(next_complement_up);

	free_context(attacks);
	free_context(not_attacks);
}


// Compute the next conflict-free closure coming after "current" and store it in "next"
char next_imp_closure(ImplicationSet *imps, BitSet* current, BitSet* next) {
	int i,j;
	BitSet* tmp = create_bitset(current->size);

	for (i = current->size - 1; i >= 0; --i) {
		if (TEST_BIT(current, i))
			RESET_BIT(current, i);
		else {
			reset_bitset(tmp);
			SET_BIT(current, i);

			// compute closure
			// reset_bitset(next);
			naive_closure(current, imps, next);
			RESET_BIT(current, i);

			// TODO: optimize!
			bitset_set_minus(next, current, tmp);
			char flag = 0;
			for (j = 0; j < i; ++j)
				// check if next \ current contains a bit larger than i
				if (TEST_BIT(tmp, j)) {
					flag = 1;
					break;
				}
			if (!flag)
				return(1);
		}
	}
	return(0);
}

/*
void all_stable_extensions_nourine(Context* attacks, FILE *outfile) {

	Context* not_attacks = negate_context(attacks);

	BitSet* tmp = create_bitset(attacks->size);
	BitSet* current = create_bitset(attacks->size);
	// next closure
	BitSet* next = create_bitset(attacks->size);
	// next closure complement
	BitSet* current_complement = create_bitset(attacks->size);
	// next closure complement up
	BitSet* current_complement_up = create_bitset(attacks->size);

	ImplicationSet *imps = attacks_to_implications(attacks);

	printf("\nImps size: %d\n\n", imps->size);
	printf("imps: ");
	print_implication_set(imps);

	int closure_count = 0, stable_extension_count = 0;

	// closure of the empty set
	naive_closure(tmp, imps, current);

	while (1) {
		++closure_count;
		print_bitset(current, stdout);
		printf("\t");

		complement_bitset(current, current_complement);
		print_bitset(current_complement, stdout);
		printf("\n");

		char cf = is_conflict_free(attacks, current_complement);

		up_arrow(not_attacks, current_complement, current_complement_up);

		if (bitset_is_equal(current_complement, current_complement_up)) {
			++stable_extension_count;
			print_bitset(current_complement, outfile);
			fprintf(outfile, "\n");
		}
		if (!next_imp_closure(imps, current, next))
			break;
		copy_bitset(next, current);
	}
	printf("Number of closures generated: %d\n", closure_count);
	printf("Number of stable extensions: %d\n", stable_extension_count);

	free_bitset(tmp);
	free_bitset(current);
	free_bitset(next);
	free_bitset(current_complement);
	free_bitset(current_complement_up);

	free_context(attacks);
	free_context(not_attacks);
}
*/

void one_stable_extension_nourine(Context* attacks, FILE *outfile) {

	Context* not_attacks = negate_context(attacks);

	BitSet* tmp = create_bitset(attacks->size);
	BitSet* nc = create_bitset(attacks->size);
	BitSet* nc_up = create_bitset(attacks->size);

	ImplicationSet *imps = attacks_to_implications(attacks);

	int closure_count = 0;

	while (1) {
		/* TODO:
		if (!next_imp_closure(attacks, imps, tmp, nc))
			break;
		*/
		++closure_count;
		// printf("*");
		// print_bitset(ni, stdout);
		// printf("\n");

		up_arrow(not_attacks, nc, nc_up);
		// ni is closed but has a conflict
		// if (!bitset_is_subset(ni, tmp)) {
		// 	printf("*");
		// 	print_bitset(ni, stdout);
		// 	printf("\n");
		// }
		if (bitset_is_equal(nc, nc_up)) {
			print_bitset(nc, outfile);
			fprintf(outfile, "\n");
			break;
		}
		copy_bitset(nc, tmp);
	}
	printf("Number of closures generated: %d\n", closure_count);

	free_bitset(tmp);
	free_bitset(nc);
	free_bitset(nc_up);

	free_context(attacks);
	free_context(not_attacks);
}
