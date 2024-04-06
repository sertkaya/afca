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

#include "stable.h"
#include "implications.h"

ImplicationSet *attacks_to_implications(AF* attacks) {
	ImplicationSet *imps = create_implication_set();
	AF* attacked_by = transpose_argumentation_framework(attacks);

	int i, j;
	// Implications from the Nourine paper.
	for (i = 0; i < attacks->size; ++i) {
		for (j = 0; j < attacks->size; ++j) {
			if (CHECK_ARG_ATTACKS_ARG(attacks, i, j) && !CHECK_ARG_ATTACKS_ARG(attacks, j, i)) {
				BitSet *lhs = create_bitset(attacks->size);
				copy_bitset(attacked_by->graph[i], lhs);

				BitSet *rhs = create_bitset(attacks->size);
				SET_BIT(rhs, j);

				Implication *imp = create_implication(lhs, rhs);
				add_implication(imp, imps);
			}
		}
	}

/*
	// Self-attacks. This is part of "reducing implications" in the paper.
	for (i = 0; i < attacks->size; ++i)
		if (CHECK_ARG_ATTACKS_ARG(attacks, i, i)) {
			BitSet *lhs = create_bitset(attacks->size);
			BitSet *rhs = create_bitset(attacks->size);
			SET_BIT(rhs, i);
			Implication *imp = create_implication(lhs, rhs);
			add_implication(imp, imps);
		}
*/

/*
	// Extra implications for stable extensions.
	// {a} U B -> A where B is the set of attackers of "a".
	for (i = 0; i < attacked_by->size; ++i) {
		BitSet *lhs = create_bitset(attacked_by->size);
		SET_BIT(lhs, i);
		bitset_union(lhs, attacked_by->graph[i], lhs);

		BitSet *rhs = create_bitset(attacked_by->size);
		set_bitset(rhs);

		Implication *imp = create_implication(lhs, rhs);
		add_implication(imp, imps);
	}
	*/

	return(imps);
}

ImplicationSet* attacks_to_implications_partially_reduced(AF* attacks) {
	ImplicationSet* imps = create_implication_set();
	AF* attacked_by = transpose_argumentation_framework(attacks);

	BitSet* closure = create_bitset(attacks->size);
	// Implications from the Nourine paper.
	for (unsigned short i = 0; i < attacks->size; ++i) {
		for (unsigned short j = 0; j < attacks->size; ++j) {
			if (CHECK_ARG_ATTACKS_ARG(attacks, i, j) && !CHECK_ARG_ATTACKS_ARG(attacks, j, i)) {
				naive_closure(attacked_by->graph[i], imps, closure);
				if (!TEST_BIT(closure, j)) {
					BitSet *lhs = create_bitset(attacks->size);
					copy_bitset(attacked_by->graph[i], lhs);

					BitSet *rhs = create_bitset(attacks->size);
					SET_BIT(rhs, j);

					Implication *imp = create_implication(lhs, rhs);
					add_implication(imp, imps);
				}
			}
		}
	}
	free_bitset(closure);
	return imps;
}

// Extra implications for stable extensions.
// {a} U B -> A where B is the set of attackers of "a".
void add_our_implications(AF* attacks, ImplicationSet *imps) {
	AF* attacked_by = transpose_argumentation_framework(attacks);
	int i;
	for (i = 0; i < attacked_by->size; ++i) {
		BitSet *lhs = create_bitset(attacked_by->size);
		SET_BIT(lhs, i);
		bitset_union(lhs, attacked_by->graph[i], lhs);

		BitSet *rhs = create_bitset(attacked_by->size);
		set_bitset(rhs);

		Implication *imp = create_implication(lhs, rhs);
		add_implication(imp, imps);
	}
}


ImplicationSet *reduce_adm(AF* attacks, ImplicationSet *imps) {
	ImplicationSet *imps_r = create_implication_set();
	ImplicationSet *imps_r2 = create_implication_set();

	int i;

	// Self-attacks.
	for (i = 0; i < attacks->size; ++i)
		if (CHECK_ARG_ATTACKS_ARG(attacks, i, i)) {
			BitSet *lhs = create_bitset(attacks->size);
			BitSet *rhs = create_bitset(attacks->size);
			SET_BIT(rhs, i);
			Implication *imp = create_implication(lhs, rhs);
			add_implication(imp, imps_r);
		}

	// Conflict type 1
	BitSet *X_closure = create_bitset(attacks->size);
	BitSet *empty = create_bitset(attacks->size);
	BitSet *empty_closure = create_bitset(attacks->size);
	BitSet *X_union_empty_closure = create_bitset(attacks->size);
	BitSet *diff = create_bitset(attacks->size);
	BitSet *tmp = create_bitset(attacks->size);
	BitSet *T = create_bitset(attacks->size);
	for (i = 0; i < imps->size; ++i) {
		if ((i%1000) == 0)
			printf("%d\n", i);
		if (bitset_is_emptyset(imps->elements[i]->lhs)) {
			BitSet *new_lhs = create_bitset(attacks->size);
			BitSet *rhs = create_bitset(attacks->size);
			copy_bitset(imps->elements[i]->rhs, rhs);
			Implication *imp = create_implication(new_lhs, rhs);
			add_implication(imp, imps_r);
			continue;
		}
		naive_closure(imps->elements[i]->lhs, imps, X_closure);
		naive_closure(empty, imps, empty_closure);
		bitset_union(imps->elements[i]->lhs, empty_closure, X_union_empty_closure);
		bitset_set_minus(X_closure, X_union_empty_closure, diff);
		int z;
		for (z = 0; z < attacks->size; ++z) {
			if (TEST_BIT(diff, z)) {
				reset_bitset(tmp);
				reset_bitset(T);
				SET_BIT(tmp, z);
				int t;
				for (t = 0; t < attacks->size; ++t) {
					if (TEST_BIT(imps->elements[i]->lhs, t)) {
						SET_BIT(tmp, t);
						if (!is_set_conflict_free(attacks, tmp)) {
							SET_BIT(T, t);
						}
						RESET_BIT(tmp, t);
					}
				}
				BitSet *new_lhs = create_bitset(attacks->size);
				BitSet *rhs = create_bitset(attacks->size);
				bitset_set_minus(imps->elements[i]->lhs, T, new_lhs);
				SET_BIT(rhs, z);
				Implication *imp = create_implication(new_lhs, rhs);
				add_implication(imp, imps_r);
			}
		}
	}

	printf("After conflict type 1:\n");
	printf("\n Size: %d\n\n", imps_r->size);

	// Conflict type 2
	BitSet *Gamma_t = create_bitset(attacks->size);
	BitSet *Gamma_t_closure = create_bitset(attacks->size);
	for (i = 0; i < imps_r->size; ++i) {
		reset_bitset(T);
		int t;
		// Iterate over elements of X
		for (t = 0; t < attacks->size; ++t) {
			if (TEST_BIT(imps_r->elements[i]->lhs, t)) {
				reset_bitset(Gamma_t);
				reset_bitset(Gamma_t_closure);
				// Compute Gamma(t)
				int j;
				for (j = 0; j < attacks->size; ++j) {
					reset_bitset(tmp);
					SET_BIT(tmp, t);
					SET_BIT(tmp, j);
					if  (!is_set_conflict_free(attacks, tmp)) {
						SET_BIT(Gamma_t, j);
					}
				}
				// Closure of Gamma(t)
				naive_closure(Gamma_t, imps_r, Gamma_t_closure);
				// if (TEST_BIT(Gamma_t_closure, imps_r->elements[i]->rhs)) {
				if (bitset_is_subset(imps_r->elements[i]->rhs, Gamma_t_closure)) {
					SET_BIT(T, t);
				}
			}
		}
		BitSet *new_lhs = create_bitset(attacks->size);
		bitset_set_minus(imps_r->elements[i]->lhs, T, new_lhs);
		BitSet *rhs = create_bitset(attacks->size);
		copy_bitset(imps_r->elements[i]->rhs, rhs);

		Implication *imp = create_implication(new_lhs, rhs);
		add_implication(imp, imps_r2);
		if (((imps_r2->size)%100) == 0)
			printf("Size of imps_r2: %d\n", imps_r2->size);
	}

	free_bitset(X_closure);
	free_bitset(empty);
	free_bitset(empty_closure);
	free_bitset(X_union_empty_closure);
	free_bitset(diff);
	free_bitset(tmp);
	free_bitset(T);

	free_bitset(Gamma_t);
	free_bitset(Gamma_t_closure);

	return(imps_r2);
}

void stable_extensions_nourine(AF* attacks, FILE *outfile) {

	AF* not_attacks = complement_argumentation_framework(attacks);

	BitSet* c = create_bitset(attacks->size);
	// c closure
	BitSet* cc = create_bitset(attacks->size);
	// c closure complement
	BitSet* ccc = create_bitset(attacks->size);
	// c closure complement up
	BitSet* ccc_up = create_bitset(attacks->size);
	BitSet* tmp = create_bitset(attacks->size);

	ImplicationSet *imps = attacks_to_implications(attacks);
	printf("Implications size: %d\n", imps->size);
	print_implication_set(imps);
	printf("\n");

	ImplicationSet *imps_r = reduce_adm(attacks, imps);
	printf("\nImps reduced size: %d\n\n", imps_r->size);
	print_implication_set(imps_r);
	printf("\n");

	add_our_implications(attacks, imps_r);
	printf("\nSize after adding our implications: %d\n\n", imps_r->size);

	int i, j, concept_count = 0, stable_extension_count = 0;

	// closure of the empty set
	naive_closure(c, imps_r, cc);
	copy_bitset(cc, c);

	complement_bitset(cc, ccc);
	up_arrow(not_attacks, ccc, ccc_up);

	if (bitset_is_equal(ccc, ccc_up)) {
		++stable_extension_count;
		print_set(ccc, outfile, "\n");
	}

	while (!bitset_is_fullset(c)) {
		// print_bitset(c, stdout);
		// printf("\n");
		++concept_count;

		for (i = attacks->size - 1; i >= 0; --i) {
			if (TEST_BIT(c, i))
				RESET_BIT(c, i);
			else {
				reset_bitset(tmp);
				SET_BIT(c, i);

				// compute closure
				naive_closure(c, imps_r, cc);

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
						print_set(ccc, outfile, "\n");
						++concept_count;
						continue;
					}
					else if (is_set_conflict_free(attacks, ccc)) {
						continue;
					}
					else {
						break;
					}
				}
			}
		}
		copy_bitset(cc, c);
	}

	printf("Number of concepts generated: %d\n", concept_count);
	printf("Number of stable extensions: %d\n", stable_extension_count);

	free_bitset(tmp);
	free_bitset(c);
	free_bitset(cc);
	free_bitset(ccc);
	free_bitset(ccc_up);

	free_implication_set(imps);
	free_argumentation_framework(not_attacks);
}


// compute lectically next set closed under imps and implications of
// the form {a} U B —> \bot, where B is the set of all attackers of a 
bool next_dominating_closure(BitSet* closure, ImplicationSet* imps, AF* attacked) {
	bool next = false;
	BitSet* new_closure = create_bitset(closure->size);
	for (unsigned short i = closure->size - 1; i >= 0; --i) {
		if (TEST_BIT(closure, i)) {
			RESET_BIT(closure, i);
		} else {
			SET_BIT(closure, i);
			naive_closure(closure, imps, new_closure);
			next = true;
			for (unsigned short j = 0; j < i; ++j) { // lectic-order test || domination test: complement must attack all arguments in new_closure
				if (TEST_BIT(new_closure, j) && (!TEST_BIT(closure, j) || bitset_is_subset(attacked->graph[j], new_closure))) {
					next = false;
					break;
				}
			}
			for (unsigned short j = i; j < closure->size; ++j) {
				if (TEST_BIT(new_closure, j) && bitset_is_subset(attacked->graph[j], new_closure)) {	// domination test
					next = false;
					break;
				}
			}
			if (next) {
				copy_bitset(new_closure, closure);
				break;
			} else {
				RESET_BIT(closure, i);
			}
		}
	}
	free_bitset(new_closure);
	return next;
}

void one_stable_extension_nourine(AF* attacks, FILE *outfile) {
	ImplicationSet* edge_implications = attacks_to_implications_partially_reduced(attacks);
	printf("Implications size: %d\n", edge_implications->size);

	ImplicationSet* imps = edge_implications; //reduce_adm(attacks, edge_implications);
	// free_implication_set(edge_implications);
	// printf("\nImps reduced size: %d\n\n", imps->size);

	AF* attacked = transpose_argumentation_framework(attacks);

	BitSet* empty = create_bitset(attacks->size);
	BitSet* closure = create_bitset(attacks->size);
	naive_closure(empty, imps, closure);
	free_bitset(empty);

	BitSet* complement = create_bitset(attacks->size);
	do {
		complement_bitset(closure, complement);
		if (is_set_conflict_free(attacks, complement)) {
			print_set(complement, outfile, "\n");
			break;
		}
	} while (next_dominating_closure(closure, imps, attacked));

	free_argumentation_framework(attacked);
	free_bitset(closure);
	free_bitset(complement);
	free_implication_set(imps);
}