%{
#include "may.h"
%}

%%
"void" { return VOID; }
"char" { return CHAR; }
%%

int yywrap() {
	return 0;
}
