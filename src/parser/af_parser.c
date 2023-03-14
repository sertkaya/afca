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
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

#include "../bitset/bitset.h"
#include "../fca/context.h"

void read_af(FILE* af, Context* c) {
	int arg_count = 0, att_count = 0;

	int rc = fscanf(af, "p af %d", &arg_count);

	// Allocate space for the context
	init_context(c, arg_count);

	int arg1, arg2;
	// while (rc != EOF) {
	do {
		rc = fscanf(af, "%d %d\n", &arg1, &arg2);
		if (rc <= 0)
			// Line does not match to the format,
			// skip until end-of-line
			fscanf(af, "%*[^\n]\n");
		else {
			++att_count;
			ADD_ATTRIBUTE(c, arg2, arg1);
		}
	} while (rc != EOF);

	printf("Argument count: %d\n", arg_count);
	printf("Attacks count : %d\n", att_count);
}

