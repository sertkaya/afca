/*
 * af_parser.c
 *
 *  ...
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

#include "../bitset/bitset.h"
#include "../fca/context.h"
#include "../utils/timer.h"

void read_af(FILE* af, Context* c) {
	struct timeval start_time, stop_time;
	int arg_count = 0, att_count = 0;

	// printf("Reading af.........................: ");
	// fflush(stdout);

	START_TIMER(start_time);
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
	STOP_TIMER(stop_time);

	/*
	if (parser != 0) {
		print_short_stats(kb);
		fprintf(stderr,"aborting\n");
		exit(-1);
	}
	*/
	printf("AF read in %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);
	printf("Argument count: %d\n", arg_count);
	printf("Attacks count : %d\n", att_count);
}

