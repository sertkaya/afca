/*
 * af_parser.h
 *
 *  Created on: 15.11.2022
 *      Author: bs
 */

#ifndef PARSER_AF_PARSER_H_
#define PARSER_AF_PARSER_H_

#include <stdio.h>
#include "../fca/context.h"

// Read the argumentation framework from file into context.
void read_af(FILE* af, Context* c);

#endif /* PARSER_AF_PARSER_H_ */
