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
			}
		}
	}

	return(imps);
}

// Compute closure of x under imps and store in c
void naive_closure(BitSet *x, ImplicationSet *imps, BitSet *c) {
	int i;
	do {
		copy_bitset(x, c);
		for (i = 0; i < imps->size; ++i) {
			if (bitset_is_subset(imps->elements[i]->lhs, x)) {
				SET_BIT(x, imps->elements[i]->rhs);
			}
		}
	} while (!bitset_is_equal(x, c));
}

// Compute the next conflict-free closure coming after "current" and store it in "next"
char next_imp_closure(Context* attacks, ImplicationSet *imps, BitSet* current, BitSet* next) {
	int i,j;
	BitSet* tmp = create_bitset(current->size);
	BitSet* next_complement = create_bitset(current->size);

	for (i = attacks->size - 1; i >= 0; --i) {
		if (TEST_BIT(current, i))
			RESET_BIT(current, i);
		else {
			// check if i attacks i
			// if (TEST_BIT(attacks->a[i], i))
			// 		continue;

			// check if argument i attacks the set current
			// bitset_intersection(current, attacks->a[i], tmp);
			// if (!bitset_is_emptyset(tmp))
			// 	continue;

			// check if current attacks i
			// char flag = 0;
			// for (j = 0; j < current->size; ++j) {
			// 	if (TEST_BIT(current,j) && TEST_BIT(attacks->a[j], i)) {
			// 		flag = 1;
			// 		break;
			// 	}
			// }
			// if (flag)
			// 	continue;

			reset_bitset(tmp);
			SET_BIT(current, i);

			// compute closure
			naive_closure(current, imps, next);
			RESET_BIT(current, i);

			// check if the complement of next is conflict free
			// negate_bitset(current, next_complement);
			// if (is_conflict_free(attacks, next_complement)) {
				// printf("next:");
				// print_bitset(next, stdout);
				// printf("\t");

				// printf("next_complement:");
				// print_bitset(next_complement, stdout);
				// printf("\n");

				// continue;
			// }

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

	while (!bitset_is_fullset(next)) {
		printf("current:");
		print_bitset(current, stdout);
		printf("\t");
		for (i = attacks->size - 1; i >= 0; --i) {
				// printf("i:%d\n", i);
			if (TEST_BIT(current, i))
				RESET_BIT(current, i);
			else {
				reset_bitset(tmp);
				SET_BIT(current, i);

				// compute closure
				naive_closure(current, imps, next);
				RESET_BIT(current, i);

				complement_bitset(next, complement_next);
				// check if the complement of next is conflict free
				// in this case we cut this branch
				if (is_conflict_free(attacks, complement_next)) {
					// check if complement_next is a stable extension
					up_arrow(not_attacks, complement_next, next_complement_up);
					if (bitset_is_equal(complement_next, next_complement_up)) {
						++stable_extension_count;
						print_bitset(complement_next, outfile);
						fprintf(outfile, "\n");
					}

					// printf("CURRENT:");
					// print_bitset(current, stdout);
					// printf("\t");
					// printf("NEXT:");
					// print_bitset(next, stdout);
					// printf("\t");
					// print_bitset(next_complement, stdout);
					// printf(" cf ");
					// printf("i: %d\t",i);

					// RESET_BIT(current, i + 1 );
					// printf("CURRENT_:");
					// print_bitset(current, stdout);
					// printf("\n");
					continue;
				}

				// TODO: optimize!
				bitset_set_minus(next, current, tmp);
				char flag = 0;
				for (j = 0; j < i; ++j)
					// check if next \ current contains a bit larger than i
					if (TEST_BIT(tmp, j)) {
						flag = 1;
						break;
					}
				if (!flag) {
					// return(1);
					++closure_count;

					break;
				}
			}
		}
		printf("next:");
		print_bitset(next, stdout);
		printf("\n");
		copy_bitset(next, current);
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

/*
void all_stable_extensions_nourine(Context* attacks, FILE *outfile) {

	Context* not_attacks = negate_context(attacks);

	BitSet* tmp = create_bitset(attacks->size);
	// next closure
	BitSet* nc = create_bitset(attacks->size);
	// next closure complement
	BitSet* ncc = create_bitset(attacks->size);
	// next closure complement up
	BitSet* ncc_up = create_bitset(attacks->size);

	ImplicationSet *imps = attacks_to_implications(attacks);

	printf("imps: ");
	print_implication_set(imps);

	int closure_count = 0, stable_extension_count = 0;

	while (1) {
		if (!next_imp_closure(attacks, imps, tmp, nc))
			break;
		++closure_count;
		printf("nc:");
		print_bitset(nc, stdout);
		printf("\t");

		negate_bitset(nc, ncc);
		printf("ncc:");
		print_bitset(ncc, stdout);
		printf("\t");

		char cf = is_conflict_free(attacks, ncc);
		printf("%d\n", cf);

		up_arrow(not_attacks, ncc, ncc_up);
		// printf("ncc_up:");
		// print_bitset(ncc_up, stdout);
		// printf("\n");
		// ni is closed but has a conflict
		// if (!bitset_is_subset(ni, tmp)) {
		// 	printf("*");
		// 	print_bitset(ni, stdout);
		// 	printf("\n");
		// }
		if (bitset_is_equal(ncc, ncc_up)) {
			++stable_extension_count;
			print_bitset(ncc, outfile);
			fprintf(outfile, "\n");
		}
		copy_bitset(nc, tmp);
	}
	printf("Number of closures generated: %d\n", closure_count);
	printf("Number of stable extensions: %d\n", stable_extension_count);

	free_bitset(tmp);
	free_bitset(nc);
	free_bitset(ncc);
	free_bitset(ncc_up);

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
		if (!next_imp_closure(attacks, imps, tmp, nc))
			break;
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
