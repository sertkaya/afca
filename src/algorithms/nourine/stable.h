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

#include "../../af/af.h"
#include "../../bitset/bitset.h"

// In our case implications are unit implications.
// lhs is a bitset, rhs is an index
struct unit_implication {
	BitSet *lhs;
	int rhs;
};

typedef struct unit_implication UnitImplication;

UnitImplication *create_unit_implication(BitSet *lhs, int rhs);

void free_implication(UnitImplication *imp);

// Implication set is just an array of implications
struct implication_set {
	int size;
	UnitImplication **elements;
};

typedef struct implication_set ImplicationSet;

ImplicationSet *create_implication_set();

void add_implication(UnitImplication *imp, ImplicationSet *imps);

ImplicationSet *attacks_to_implications(AF* attacks);

void print_implication_set(ImplicationSet *imps);

void stable_extensions_nourine(AF* c, FILE *f);

void one_stable_extension_nourine(AF* c, FILE *f);

void naive_closure(BitSet *x, ImplicationSet *imps, BitSet *c);


