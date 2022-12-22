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
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <getopt.h>

#include "parser/af_parser.h"
#include "fca/context.h"

void usage(char* program) {
	fprintf(stderr, "Usage: %s -i input-file -o output-file -c\n", program);
}

int main(int argc, char *argv[]) {
	FILE* input_af;
	FILE* output;

	int c, input_flag = 0, output_flag = 0, wrong_argument_flag = 0, verbose_flag = 0;
	char *af_file_name = "", *output_file = "";
	static char usage[] = "Usage: %s -i graph -o output\n";
	while ((c = getopt(argc, argv, "i:o:v")) != -1)
		switch (c) {
		case 'i':
			input_flag = 1;
			af_file_name = optarg;
			break;
		case 'o':
			output_flag = 1;
			output_file = optarg;
			break;
		case 'v':
			verbose_flag = 1;
			break;
		case '?':
			wrong_argument_flag = 1;
			break;
		}
	if (input_flag == 0) {
		fprintf(stderr, "%s: Provide an input argumentation framework\n", argv[0]);
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}
	if (output_flag == 0) {
		fprintf(stderr, "%s: Provide an output file\n", argv[0]);
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}
	if (wrong_argument_flag) {
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}
	// open the af file
	input_af = fopen(af_file_name, "r");
	assert(input_af != NULL);

	// Create an empty context.
	Context* context = create_context();

	// Read the af into the context.
	read_af(input_af, context);

	// read and parse the graph
	fclose(input_af);

	if (verbose_flag) {
		// print_short_stats(kb);
	}

	// open the output file
	output = fopen(output_file, "w");
	assert(output != NULL);

	// close the output file
	fclose(output);

	return 0;
}

