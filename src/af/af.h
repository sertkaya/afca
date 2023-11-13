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

#ifndef AF_AF_H_
#define AF_AF_H_

struct argumentation_framework {
	// Number of arguments
	int size;
	// The adjacency matrix: array of bitsets
	BitSet **graph;
	// Number of bytes required for one bitset
	int bitset_base_count;
};

typedef struct argumentation_framework AF;

// Create argumentation framework with the given number of arguments
AF* create_argumentation_framework(int size);

// Free the space allocated for af
// Return the number of bytes freed
int free_argumentation_framework(AF *af);

void print_argumentation_framework(AF *af);

// Add an attack from argument at index i to argument at index j
#define ADD_ATTACK(af,i,j)		SET_BIT(af->graph[i-1],j-1)

int is_conflict_free(Context* attacks, BitSet* x);

// Compute victims of bs (arguments attacked by bs)
// (up-arrow in FCA terms) Put the result in r
void get_victims(AF* af, BitSet* bs, BitSet* r);

// Compute attackers of bs (arguments that attack bs)
// (down-arrow in FCA terms) Put the result in r
void get_attackers(AF* af, BitSet* bs, BitSet* r);

AF* complement_argumentation_framework(AF *af );

AF* transpose_argumentation_framework(AF *af);

#endif /* AF_AF_H_ */
