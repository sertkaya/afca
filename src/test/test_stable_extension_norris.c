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


#include "../af/af.h"
#include "../parser/af_parser.h"
#include "../algorithms/norris/stable.h"

int main(int argc, char *argv[]) {

	FILE *fd = fopen(argv[1], "r");
	assert(fd != NULL);

	AF* af = read_af(fd);
	// print_context(c);

	incremental_stable_extensions_norris(af, stdout);

	return(0);
}


