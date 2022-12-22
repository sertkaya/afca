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
 

%{
	#include <stdio.h>
	#include <assert.h>
	#include <string.h>
	
	// #define YYSTYPE Expression

	extern char* yytext;
	int yylex(void);
	extern int yylineno;
	void yyerror(char* msg);
	extern FILE *yyin;

%}

	// %parse-param {KB* kb} 

	// %start ontologyDocument

%token ARGUMENT ARG ATT

%%

/*****************************************************************************/
/* General Definitions */

Argument:
	ARG '(' ARGUMENT ')'	{ printf("arg: %s", $3); };

Attacks:
	ATT '(' ARGUMENT',' ARGUMENT ')' {
		printf("%s attacks %s\n", $3, $4);
	};


%%

void yyerror(char* msg) {
	fprintf(stderr, "\nline %d: %s\n", yylineno, msg);
}
