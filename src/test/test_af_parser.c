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
// #include "../bitset/bitset.h"

int main(int argc, char *argv[]) {

	FILE *input_fd = fopen(argv[1], "r");
	assert(input_fd != NULL);

	AF *af = read_af(input_fd);
	print_argumentation_framework(af);

	int size = af->size;
	// BitSet* bs = create_bitset(size);
	// BitSet* r = create_bitset(size);

	// SET_BIT(bs, 4);
	// printf("bs: ");
	// print_bitset(bs, stdout);
	// printf("\n");

	// down_up_arrow(af, bs, r);
	// printf("r: ");
	// print_bitset(r, stdout);
	// printf("\n");

	return(0);
}


