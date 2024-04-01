/* Definition section */
%{
    #include "compiler_common.h"
    #include "main.h"
    
    int yydebug = 1;

    void yyerror (char const *s) {
        printf("error:%d:%d %s\n", yylineno, yyleng, s);
        compileError = true;
    }

%}

/* Variable or self-defined structure */
%union {
    ObjectType var_type;

    bool b_var;
    int i_var;
    float f_var;
    char *s_var;

    Object object_val;
}

/* Token without return */
%token COUT
%token SHR SHL BAN BOR BNT BXO ADD SUB MUL DIV MOD NOT GTR LES GEQ LEQ EQL NEQ LAN LOR
%token VAL_ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN REM_ASSIGN SHR_ASSIGN SHL_ASSIGN BAN_ASSIGN BOR_ASSIGN INC_ASSIGN DEC_ASSIGN
%token IF ELSE FOR WHILE LOOP
%token RETURN BREAK
%token ARROW AS IN RSHIFT LSHIFT

/* Token with return, which need to sepcify type */
%token <var_type> VARIABLE_T
%token <b_var> BOOL_LIT
%token <i_var> INT_LIT
%token <f_var> FLOAT_LIT
%token <s_var> STR_LIT
%token <s_var> IDENT

/* Nonterminal with return, which need to sepcify type */
%type <object_val> Expression
%type <object_val> ValueStmt

%left LOR
%left LAN
%left GTR LES EQL NEQ LEQ GEQ
%left LSHIFT RSHIFT
%left ADD SUB
%left MUL DIV MOD

/* %nonassoc IFX
%nonassoc ELSE */

/* Yacc will start at this nonterminal */
%start Program

%%
/* Grammar section */

Program
    : StmtList
    | // Empty file
;

StmtList 
    : StmtList Stmt
    | Stmt
;

Stmt
    : ScopeStmt
    | VARIABLE_T IDENT '(' FunctionVariableStmtList ')' {} ScopeStmt
    | ';'
;

ScopeStmt
    : '{' StmtList '}'
    | '{' '}'
;

FunctionVariableStmtList 
    : FunctionVariableStmtList ',' FunctionVariableStmt
    | FunctionVariableStmt
    |
;
FunctionVariableStmt
    : VARIABLE_T IDENT { pushFunVar($<var_type>1, $<s_var>2); }
;

Expression
    : ValueStmt
;

ValueStmt
    : BOOL_LIT { $$ = (Object){OBJECT_TYPE_BOOL, (uint64_t)$<b_var>1}; printf("bool %s\n", $<b_var>1?"true":"false"); }
    | FLOAT_LIT { $$ = (Object){OBJECT_TYPE_FLOAT, *((uint64_t*)&$<f_var>1)}; printf("FLOAT_LIT %f\n", $<f_var>1); }
    | INT_LIT { $$ = (Object){OBJECT_TYPE_INT, (uint64_t)$<i_var>1}; printf("INT_LIT %d\n", $<i_var>1); }
    | STR_LIT { $$ = (Object){OBJECT_TYPE_STR, (uint64_t)$<s_var>1}; printf("STRING_LIT \"%s\"\n", $<s_var>1); }
;

%%
/* C code section */