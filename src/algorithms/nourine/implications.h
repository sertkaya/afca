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

#ifndef ALGORITHMS_NOURINE_IMPLICATIONS_H_
#define ALGORITHMS_NOURINE_IMPLICATIONS_H_

// Implication. lhs and rhs are bitsets.
struct implication {
	BitSet *lhs;
	BitSet *rhs;
};

typedef struct implication Implication;

Implication *create_implication(BitSet *lhs, BitSet *rhs);

void print_implication(Implication *imp);

unsigned long free_implication(Implication *imp);

// Implication set is just an array of implications
struct implication_set {
	int size;
	Implication **elements;
};

typedef struct implication_set ImplicationSet;

ImplicationSet *create_implication_set();

// Free the set of implications.
// Returns the number of bytes freed.
unsigned long free_implication_set(ImplicationSet *imps);

struct implication_node {
	Implication* implication;
	struct implication_node* next;
};

typedef struct implication_node ImplicationNode;

ImplicationNode *create_implication_node(Implication* i, ImplicationNode* next);

void free_implication_node(ImplicationNode* node, bool free_tail, bool free_impl);

void add_implication(Implication *imp, ImplicationSet *imps);

ImplicationSet *attacks_to_implications(AF* attacks);

void print_implication_set(ImplicationSet *imps);

void naive_closure(BitSet *x, ImplicationSet *imps, BitSet *c);

void close(BitSet* x, ImplicationNode* head);

void compute_closure(BitSet* x, ImplicationNode* head, BitSet* c);

unsigned int count_implications(ImplicationNode* head);

ImplicationNode* reduce_implications(ImplicationNode* head);

#endif /* ALGORITHMS_NOURINE_IMPLICATIONS_H_ */
