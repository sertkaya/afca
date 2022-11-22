/* 
 * The Elephant Reasoner
 * 
 * Copyright (C) Baris Sertkaya (sertkaya.baris@gmail.com)
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

COMMENT                         "#"[^\n]*
DIGIT                           [0-9]
LETTER							[A-Za-z]
ARGUMENT						{LETTER}{DIGIT}*


%{
	#include <stdio.h>
	#include "y.tab.h"

	// #define YYSTYPE Expression
	extern YYSTYPE yylval;

	void yyerror(char *);
%}

%%

\n                              { ++yylineno; }
                                /* skip whitespace */
[ \t]                           ;
	/* skip comments */
	/* "//".*\n								;*/
										
[()=]                           { return *yytext; }

                                /* Graph */
arg                          	{ return ARG; }
att                        		{ return ATT; }

{ARGUMENT}                      { return ARGUMENT; }

%%

int yywrap(void) {
	return 1;
}
