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
#include <unistd.h>
#include <sys/time.h>

#include "../bitset/bitset.h"
#include "../af/af.h"

AF* read_af(FILE* input_fd) {
	long unsigned int arg_count = 0;
	long unsigned int att_count = 0;

	int rc = fscanf(input_fd, "p af %d", &arg_count);

	// create an AF with size arg_count
	AF *af = create_argumentation_framework(arg_count);

	int arg1, arg2;
	do {
		rc = fscanf(input_fd, "%d %d\n", &arg1, &arg2);
		if (rc <= 0)
			// Line does not match to the format,
			// skip until end-of-line
			fscanf(input_fd, "%*[^\n]\n");
		else {
			++att_count;
			ADD_ATTACK(af, arg1, arg2);
		}
	} while (rc != EOF);

	printf("Argument count: %d\n", arg_count);
	printf("Attacks count : %d\n", att_count);
	printf("Density : %.4lf\n", ((double) att_count) / (arg_count * arg_count));

	return(af);
}

