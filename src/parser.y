%{
	#include "node.h"
  #include <cstdio>
  #include <cstdlib>
  #include <iostream>
	NBlock *programBlock; /* the top level root node of our final AST */

	extern int yylex();
	void yyerror(const char *s) { std::printf("Error: %s\n", s);std::exit(1); }
%}

/* Represents the many different ways we can access our data */
%union {
	// Node *node;
	NBlock *block;
	NExpression *expr;
	NStatement *stmt;
	NIdentifier *ident;
	NVariableDeclaration *var_decl;
	NLiteral *literal;
	std::vector<NVariableDeclaration*> *varvec;
	std::vector<NExpression*> *exprvec;
	char cVal;
	bool bVal;
	std::string *string;
	int token;
}


/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */
%token <string> TIDENTIFIER TINTEGER TDOUBLE
%token <token> TCEQ TCNE TCLT TCLE TCGT TCGE TEQUAL
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TCOMMA TDOT TSEMI LBRACKET RBRACKET
%token <token> TPLUS TMINUS TMUL TDIV TMOD TAND TOR TNOT TBITAND
%token <token> TRETURN TEXTERN TIF TELSE TFOR TWHILE TBREAK TCONTINUE
%token <bVal> TTRUE TFALSE
%token <cVal> TCHARACTER
%token <token> TYPE_BOOL TYPE_CHAR TYPE_INT TYPE_DOUBLE TYPE_VOID



/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (NIdentifier*). It makes the compiler happy.
 */

%type <stmt> stmt var_decl func_decl extern_decl if_stmt for_stmt while_stmt
%type <stmt> break_stmt continue_stmt for_init_stmt for_end_stmt
%type <ident> ident
%type <expr> expr 
%type <varvec> func_decl_args
%type <exprvec> call_args
%type <block> program stmts block
%type <token> unary_op binary_op dtype
%type <literal> literal

/* Operator precedence for mathematical operators */

%left	TCOMMA 
%right TEQUAL  
%left	TOR
%left	TAND
%left	TBITAND
%left	TCEQ TCNE
%left	TCGE TCGT TCLE TCLT
%left	TPLUS TMINUS
%left	TMUL TDIV TMOD
%right TNOT   


%start program

%%

program : stmts { programBlock = $1; }
		;
		
stmts : stmt  { $$ = new NBlock(); $$->statements.push_back($<stmt>1); }
	  | stmts stmt  { $1->statements.push_back($<stmt>2); }
	  ;

stmt : var_decl TSEMI
     | func_decl 
	 | extern_decl TSEMI
	 | if_stmt 
	 | for_stmt 
	 | while_stmt
     | continue_stmt TSEMI
	 | break_stmt TSEMI
     | block { $$ = $1; }
	 | expr TSEMI { $$ = new NExpressionStatement(*$1); }
	 | TRETURN expr TSEMI { $$ = new NReturnStatement($2); }
	 | TRETURN TSEMI { $$ = new NReturnStatement(); }
	 | TSEMI { $$ = new NStatement(); } /*empty statement*/
     ;

for_init_stmt : var_decl
			  | /*empty*/ { $$ = new NStatement(); }
			  ;

for_end_stmt : var_decl 
             | block { $$ = $1; }
			 | expr { $$ = new NExpressionStatement(*$1); }
			 | /*empty*/ { $$ = new NStatement(); }
			 ;

block : TLBRACE stmts TRBRACE { $$ = $2; }
	  | TLBRACE TRBRACE { $$ = new NBlock(); }
	  ;

var_decl : dtype ident { $$ = new NVariableDeclaration( new BuiltInType($1) , *$2); }
		 | dtype ident TEQUAL expr { $$ = new NVariableDeclaration( new BuiltInType($1), *$2 , $4); }
     | dtype ident LBRACKET TINTEGER RBRACKET { $$ = new NVariableDeclaration( new NArrayType($1, atoi($4->c_str())) , *$2 ); }
     | dtype TMUL ident { $$ = new NVariableDeclaration( new NPointerType($1) , *$3 ); } //Pointer
		 ;

extern_decl : TEXTERN dtype ident TLPAREN func_decl_args TRPAREN
                { $$ = new NExternDeclaration(new BuiltInType($2), *$3, *$5); delete $5; }
            ;

func_decl : dtype ident TLPAREN func_decl_args TRPAREN block 
			{ $$ = new NFunctionDeclaration(new BuiltInType($1), *$2, *$4, $6); delete $4; }
		  ;
	
func_decl_args : /*blank*/  { $$ = new VariableList(); }
		  | var_decl { $$ = new VariableList(); $$->push_back($<var_decl>1); }
		  | func_decl_args TCOMMA var_decl { $1->push_back($<var_decl>3); }
		  ;

if_stmt : TIF TLPAREN expr TRPAREN stmt { $$ = new NIfStatement(*$3, $5); }
		| TIF TLPAREN expr TRPAREN stmt TELSE stmt { $$ = new NIfStatement(*$3, $5, $7); }
		;

for_stmt : TFOR TLPAREN for_init_stmt TSEMI expr TSEMI for_end_stmt TRPAREN stmt { $$ = new NForStatement($3, $5, $7, $9); }
         ;

while_stmt : TWHILE TLPAREN expr TRPAREN stmt { $$ = new NWhileStatement($3, $5); }
           ;

continue_stmt : TCONTINUE  { $$ = new NContinueStatement(); }
			  ;

break_stmt : TBREAK  { $$ = new NBreakStatement(); }
           ;

ident : TIDENTIFIER { $$ = new NIdentifier(*$1); delete $1; }
	  ;

literal : TINTEGER { $$ = new NLiteral(atoi($1->c_str())); delete $1;  }
		| TDOUBLE { $$ = new NLiteral(atof($1->c_str())); delete $1; }
		| TCHARACTER { $$ = new NLiteral($1); }
		| TTRUE { $$ = new NLiteral($1); }
		| TFALSE { $$ = new NLiteral($1); }
		;
	
expr : expr TEQUAL expr  { $$ = new NAssignment(*$1, *$3);}//assignment 赋值
  | ident TLPAREN call_args TRPAREN { $$ = new NMethodCall(*$1, *$3); delete $3; }
  | ident { $<ident>$ = $1; }
  | literal {$$ = $1;}
  | unary_op expr { $$ = new NUnaryOperator($1, *$2); }
  | expr binary_op expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
  | TLPAREN expr TRPAREN { $$ = $2; }
  | ident LBRACKET expr RBRACKET { $$ = new NArrayIndex(*$1, *$3); }
  | TMUL ident %prec TNOT { $$ = new NDereference(*$2); }
  | TBITAND ident  { $$ = new NAddressOf(*$2); }
  ;
	
call_args : /*blank*/  { $$ = new ExpressionList(); }
		  | expr { $$ = new ExpressionList(); $$->push_back($1); }
		  | call_args TCOMMA expr  { $1->push_back($3); }
		  ;

binary_op : TPLUS | TMINUS | TMUL | TDIV | TMOD | TAND | TOR | 
			TCEQ | TCNE | TCLT | TCLE | TCGT | TCGE;

unary_op : TMINUS %prec TNOT 
| TNOT ;

dtype : TYPE_BOOL { $$ = _Bool; }
      | TYPE_CHAR { $$ = _Char; }
      | TYPE_INT { $$ = _Int; }
      | TYPE_DOUBLE  { $$ = _Double; }
      | TYPE_VOID { $$ = _Void; }
      ;

%%