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

#include "../parser/af_parser.h"
#include "../af/af.h"
#include "../utils/array_list.h"

int main(int argc, char *argv[]) {

	FILE *input_fd = fopen(argv[1], "r");
	assert(input_fd != NULL);

	ARG_TYPE a = atoi(argv[2]) - 1;

	AF *af = read_af(input_fd);
	printf("AF:\n");
	// print_argumentation_framework(af);
	printf("\n");

	ARG_TYPE *mapping = calloc(af->size, sizeof(ARG_TYPE));
	assert(mapping != NULL);

	for (SIZE_TYPE i = 0; i < af->size; ++i)
		// mapping[i] = af->size - 1 - i;
		mapping[i] =  i;

	AF *mapped_af = apply_mapping(af, mapping);
	print_argumentation_framework(mapped_af);
	// AF* af_t = transpose_argumentation_framework(af);

	// Subgraph* subgraph = extract_subgraph_backwards(af_t, a);
	// printf("Subgraph:\n");
	// print_argumentation_framework(subgraph->af);
	// printf("\n");

	/*
	swap_arguments(af, 0, 4);
	printf("AF:\n");
	print_argumentation_framework(af);
	printf("\n");


	ArrayList* l = list_create();
	list_insert_at_head(3, l);
	list_insert_at_head(5, l);
	list_insert_at_head(3, l);
	list_insert_at_head(1, l);
	print_list(stdout, l, "before\n");
	list_remove(3, l);
	print_list(stdout, l, "after\n");
	*/
	// print_argumentation_framework(af_c);
	// printf("\n");

	return(0);
}


