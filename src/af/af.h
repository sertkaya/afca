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

#include <stdbool.h>

#include "datatypes.h"
#include "../utils/array_list.h"

struct argumentation_framework {
	// Number of arguments
	SIZE_TYPE size;
    // Adjacency lists
    ARG_TYPE **lists;
    // Number of elements in each adjacency list
    SIZE_TYPE *list_sizes;
	// Offsets required for residual frameworks. Describes how often
	// the argument was shifted left for obtaining the residual framework.
	// Add the offset at index i to index i to obtain the index
	// in the original framework. NULL if the framework is not a residual framework.
	int *offsets;
	// mapping[i] is the corresponding argument in the original framework
	int *mapping;
};

typedef struct argumentation_framework AF;

// Create argumentation framework with the given number of arguments
AF* create_argumentation_framework(SIZE_TYPE size);

// Free the space allocated for af
// Return the number of bytes freed
int free_argumentation_framework(AF* af);

void print_argumentation_framework(AF* af);

bool add_attack(AF* af, ARG_TYPE i, ARG_TYPE j);

bool is_set_self_defending(AF* attacks, AF* attacked_by, ArrayList* s);

AF* transpose_argumentation_framework(AF *af);

bool is_set_conflict_free(AF* attacks, ArrayList* s);

bool is_set_admissible(AF* af, ArrayList* s);

bool is_set_complete(AF* af, ArrayList* s);

bool is_set_stable(AF *attacks, ArrayList *s);

// AF* extract_residual_framework(AF* af, ARG_TYPE *args, int arg_count);
AF* extract_residual_framework(AF* af, bool *args, SIZE_TYPE arg_count);

int *extract_sccs(AF *af, AF *af_t);

AF *extract_source_component(AF *af);

#endif /* AF_AF_H_ */
