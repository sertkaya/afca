//
// Created by bs on 16.01.25.
//

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include "../af/af.h"
#include "../parser/af_parser.h"
#include "../utils/array_list.h"

bool validate(char *af_file_name, char *extension_file_name, char *semantic) {
	FILE* af_fd;
	af_fd = fopen(af_file_name, "r");
	assert(af_fd != NULL);

	FILE* e_fd;
	e_fd = fopen(extension_file_name, "r");
	assert(e_fd != NULL);

	AF *input_af = read_af(af_fd);
	fclose(af_fd);

    int rc = 0;
    int arg;
    ArrayList* extension = list_create();
	do {
		rc = fscanf(e_fd, "%d", &arg);
        if (rc > 0)
        	list_add(arg - 1, extension);

	} while (rc != EOF);
	fclose(e_fd);

    print_list(stdout, extension, "\n");

	if (strcmp(semantic, "ST") == 0) {
          // ...
    }
	else if (strcmp(semantic, "PR") == 0) {
      // ...
    }
	else if (strcmp(semantic, "CO") == 0) {
      return(is_set_admissible(input_af, extension));
    }

 }
