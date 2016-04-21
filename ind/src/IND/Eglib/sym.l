%{
#include "SYM.h"
#include "sym.h"
#include <string.h>

extern yylineno;
float atof();

/* %e 180 %p 715 %n 100 %k 30 %a 5160 %o 10800 */
%}


%%

asfor				{ 
				   return(ASFOR);   }
and				{ 
				   return(AND);   }
never				{ 
				   return(NEVER);   }
contexts			{ 
				   return(CONTEXTS);   }
onlyif				{ 
				   return(ONLYIF);   }
step				{
				  yylval.tk_int =  STP;
			 	  return(CONTINUOUS);
				}
cont				{
				  yylval.tk_int = CTS;
			 	  return(CONTINUOUS);
				}
norm				{
				  yylval.tk_int =  NORM;
			 	  return(CONTINUOUS);
				}
pois				{
				  yylval.tk_int = POISS;
			 	  return(CONTINUOUS);
				}
prior				{
			 	  return(PRIOR);
				}
expn				{
				  yylval.tk_int = EXP;
			 	  return(CONTINUOUS);
				}
stratify			{
				   return(STRATIFY);   }
utilities			{
				   return(UTILITIES);   }
[:;,><=".""("")""?""{""}"]		{
				   return(yytext[0]);
				}
\".*\"  			{ 
				  /*   first remove the quotes  */
				  yytext++;
				  yylval.tk_string = salloc(strlen(yytext)+1);
                                  strcpy(yylval.tk_string,yytext);
                                  yylval.tk_string[strlen(yylval.tk_string)-1] = 0;
                                  return(STRING);
				}
[\-a-zA-Z0-9][_\-a-z\/\\A-Z0-9]*(["."][_\-a-z\/\\A-Z0-9]+)*  { 
				  yylval.tk_string = salloc(strlen(yytext)+1);
                                  strcpy(yylval.tk_string,yytext);
                                  return(IDENTIFIER);
                                }
"|"[^\n]*\n			yylineno++;	/* skip comments */
[ \t]				; /* skip spaces, tabs, newlines */
\n				yylineno++;
.              		 	{ yyerror("Unexpected character %s",yytext); }


%%
