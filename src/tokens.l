%{
#include <string>
#include "node.h"
#include "parser.hpp"

#define SAVE_TOKEN  yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t)    (yylval.token = t)
char Escape2Char(char ch){ //转义字符
	switch(ch){
	case 'a': return '\a';
	case 'b': return '\b';
	case 'f': return '\f';
	case 'n': return '\n';
	case 'r': return '\r';
	case 't': return '\t';
	case 'v': return '\v';
	case '\\': return '\\';
	case '\'': return '\'';
	case '\"': return '\"';
	default: return '\0';
	}
}
%}



%x COMMENT
%option noyywrap


%%
[ \t\n]					                ;
"/*"                            {BEGIN(COMMENT);}
<COMMENT>"*/"                   {BEGIN(INITIAL);}
"//".*                          {;}
"extern"                        return TOKEN(TEXTERN);
"return"				                return TOKEN(TRETURN);
"if"                            return TOKEN(TIF);
"else"                          return TOKEN(TELSE);
"for"                           return TOKEN(TFOR);
"while"                         return TOKEN(TWHILE);
"continue"                      return TOKEN(TCONTINUE);
"break"                         return TOKEN(TBREAK);

"true"                          {
                                  yylval.bVal = true;
                                  return TTRUE;
                                }
"false"                         {
                                  yylval.bVal = false;
                                  return TFALSE;
                                }


"bool"							            return TOKEN(TYPE_BOOL);  
"char"							            return TOKEN(TYPE_CHAR);        
"int"							              return TOKEN(TYPE_INT);
"double"						            return TOKEN(TYPE_DOUBLE);
"void"							            return TOKEN(TYPE_VOID);


[a-zA-Z_][a-zA-Z0-9_]*          SAVE_TOKEN; return TIDENTIFIER;
[0-9]+\.[0-9]* 			            SAVE_TOKEN; return TDOUBLE;
[0-9]+					                SAVE_TOKEN; return TINTEGER;


"\'"\\."\'"									    { 
                                  yylval.cVal = Escape2Char(yytext[2]);
                                  return TCHARACTER; 
                                }
"\'"[^\\']"\'"							    { 
															    yylval.cVal = yytext[1];
															    return TCHARACTER; 
														    }           

"="						                  return TOKEN(TEQUAL);
"=="				          	        return TOKEN(TCEQ);
"!="			          		        return TOKEN(TCNE);
"<"				          		        return TOKEN(TCLT);
"<="	          				        return TOKEN(TCLE);
">"				          		        return TOKEN(TCGT);
">="					                  return TOKEN(TCGE);

"("	          					        return TOKEN(TLPAREN);
")"					          	        return TOKEN(TRPAREN);
"{"         					          return TOKEN(TLBRACE);
"}"					          	        return TOKEN(TRBRACE);
"["                             return TOKEN(LBRACKET);
"]"                             return TOKEN(RBRACKET);

"."         					          return TOKEN(TDOT);
","				          		        return TOKEN(TCOMMA);
";"                             return TOKEN(TSEMI);

"+"				          		return TOKEN(TPLUS);
"-"		          				return TOKEN(TMINUS);
"*"		          				return TOKEN(TMUL);
"/"				          		return TOKEN(TDIV);
"%"								      return TOKEN(TMOD);

"&&"							return TOKEN(TAND);
"||"							return TOKEN(TOR);
"!"								return TOKEN(TNOT);

"&"                             return TOKEN(TBITAND);
.                               printf("Unknown token!\n"); yyterminate();

%%
