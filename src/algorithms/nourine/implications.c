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
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "../af/af.h"
#include "../bitset/bitset.h"
#include "implications.h"

Implication *create_implication(BitSet *lhs, BitSet *rhs) {
	Implication *imp = (Implication*) calloc(1, sizeof(Implication));
	assert(imp != NULL);
	imp->lhs = lhs;
	imp->rhs = rhs;

	return(imp);
}

unsigned long free_implication(Implication *imp) {
	unsigned long freed_bytes = 0;
	freed_bytes += free_bitset(imp->lhs);
	freed_bytes += free_bitset(imp->rhs);
	freed_bytes += sizeof(Implication);
	free(imp);
	return(freed_bytes);
}

void print_implication(Implication *imp) {
	// print_bitset(imp->lhs, stdout);
	print_set(imp->lhs, stdout, "");
	printf("-> ");
	// print_bitset(imp->lhs, stdout);
	print_set(imp->rhs, stdout, "");
	printf("\n");
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

unsigned long free_implication_set(ImplicationSet *imps) {
	unsigned long freed_bytes = 0;
	int i;
	for (i = 0; i < imps->size; ++i)
		freed_bytes += free_implication(imps->elements[i]);
	freed_bytes += sizeof(ImplicationSet);
	free(imps);
	return(freed_bytes);
}

void add_implication(Implication *imp, ImplicationSet *imps) {
	Implication** tmp;
	tmp = realloc(imps->elements, (imps->size + 1) * sizeof(Implication*));
	assert(tmp != NULL);
	imps->elements = tmp;
	imps->elements[imps->size] = imp;
	++imps->size;
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
				bitset_union(c, imps->elements[i]->rhs, c);
				// SET_BIT(c, imps->elements[i]->rhs);
			}
		}
	} while (!bitset_is_equal(tmp, c));
	free_bitset(tmp);
}

