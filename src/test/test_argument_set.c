/*
 * The ELepHant Reasoner
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

#include "../utils/argument_set.h"

int main(int argc, char *argv[]) {

	ArgumentSet *set = new_argument_set(100);

	// add 100 arguments to the set
	for (SIZE_TYPE i = 0; i < 100; ++i) {
          add_to_argument_set(i, set);
	}

    print_argument_set(set);

	// remove two of the elements from the set
	delete_from_argument_set(10, set);
	delete_from_argument_set(99, set);
	delete_from_argument_set(0, set);

	// check which of the elements from the array are still
	// in the set
	for (SIZE_TYPE i = 0; i < 100; ++i)
		if (!argument_set_contains(i, set))
			printf("%d: not found!\n", i);
		else
			printf("%d: found!\n", i);

	// iterate over the elements of the set
    ListNode *n = set->list;
    while (n) {
      printf("%d ", n->e->n);
      n = n->next;
    }
    printf("\n");

	return 1;
}


