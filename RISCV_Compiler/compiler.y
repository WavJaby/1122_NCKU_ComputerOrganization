/* Definition section */
%{
    #include "compiler_common.h"
    #include "main.h"

    int yydebug = 1;
    
    char* yyInputFileName;
    uint32_t yycolumn;
    extern int yyleng;
    extern char* yytext;
    
    bool compileError;
    char errorCache[128];

    void yyerror(char const *msg) {
        printf(ERROR_PREFIX " %s\n", yyInputFileName, yylineno, yycolumn - yyleng + 1, msg);
        yytext[strlen(yytext)] = ' ';
        for(int i = 0; yytext[i]; i++)
            if (yytext[i] == '\r' || yytext[i] == '\n') {
                yytext[i] = 0;
                break;
            }
        printf("%6d |%s\n       |%*.s^\n", yylineno, yytext - yycolumn + yyleng, yycolumn - yyleng, "");
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
%type <object_val> ExpressionListStmt
%type <object_val> ValueStmt
/* %type <object_val> IdentStmt */

%left VAL_ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN REM_ASSIGN SHR_ASSIGN SHL_ASSIGN BAN_ASSIGN BOR_ASSIGN
%left LOR
%left LAN
%left GTR LES EQL NEQ LEQ GEQ
%left LSHIFT RSHIFT
%left ADD SUB
%left MUL DIV MOD
%left INC_ASSIGN DEC_ASSIGN 

/* %nonassoc IFX
%nonassoc ELSE */

/* Yacc will start at this nonterminal */
%start Program

%%
/* Grammar section */

Program
    : VARIABLE_T IDENT '(' FunctionVariableStmtList ')' ScopeStmt
    | // Empty file
;

ScopeStmt
    : '{' StmtList '}'
    | '{' '}'
;

StmtList 
    : StmtList Stmt
    | Stmt
;

Stmt
    : ScopeStmt
    | ';'
    | FOR '(' ForVariableStmt ';' { forBegin(); } ExpressionListStmt ';' { forConditionEnd(&$<object_val>6); } VariableAssignStmt ')'
        { forHeaderEnd(); } ScopeStmt { forEnd(); }
    | VariableAssignStmt ';'
;

VariableAssignStmt
    : ExpressionListStmt VAL_ASSIGN ExpressionListStmt { 
        if(objectValueAssign(&$<object_val>1, &$<object_val>3)) yyerrorf("'%s' is not variable\n", $<object_val>1.symbol->name);
    }
    // +=
    | ExpressionListStmt ADD_ASSIGN ExpressionListStmt {
        if(objectAddAssign(&$<object_val>1, &$<object_val>3)) yyerrorf("'%s' can not add assign\n", $<object_val>1.symbol->name);
    }
    // -=
    | ExpressionListStmt SUB_ASSIGN ExpressionListStmt {
        if(objectSubAssign(&$<object_val>1, &$<object_val>3)) yyerrorf("'%s' can not sub assign\n", $<object_val>1.symbol->name);
    }
;

ForVariableStmt
    : VARIABLE_T IDENT VAL_ASSIGN ExpressionListStmt { if(!createVariable($<var_type>1, $<s_var>2, &$<object_val>4)) yyerrorf("Failed to create variable '%s'\n", $<s_var>2); }
    | ExpressionListStmt VAL_ASSIGN ExpressionListStmt { if(objectValueAssign(&$<object_val>1, &$<object_val>3)) yyerrorf("'%s' is not variable\n", $<object_val>1.symbol->name); }
    |
;

FunctionVariableStmtList 
    : FunctionVariableStmtList ',' FunctionVariableStmt
    | FunctionVariableStmt
    |
;
FunctionVariableStmt
    : VARIABLE_T IDENT { pushFunVar($<var_type>1, $<s_var>2, false); }
    | VARIABLE_T MUL IDENT { pushFunVar($<var_type>1, $<s_var>3, true); } // Pointer
;

ExpressionListStmt
    : ExpressionListStmt ADD ExpressionListStmt { if(objectAdd(&$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | ExpressionListStmt SUB ExpressionListStmt { if(objectSub(&$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | ExpressionListStmt MUL ExpressionListStmt { if(objectMul(&$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | ExpressionListStmt DIV ExpressionListStmt { if(objectDiv(&$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | ExpressionListStmt LES ExpressionListStmt { if(objectLes(&$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | ExpressionListStmt GTR ExpressionListStmt { if(objectGtr(&$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | ExpressionListStmt INC_ASSIGN { if(objectIncAssign(&$<object_val>1, &$$)) YYABORT; }
    | ExpressionListStmt DEC_ASSIGN { if(objectDecAssign(&$<object_val>1, &$$)) YYABORT; }
    | MUL ExpressionListStmt { if(getPointerValue(&$<object_val>2, &$$)) YYABORT; }
    | '(' ExpressionListStmt ')' { $$ = $<object_val>2; }
    | ValueStmt
;

ValueStmt
    : BOOL_LIT { $$ = (Object){OBJECT_TYPE_BOOL, (*(uint8_t*)&$<b_var>1), 0, NULL}; }
    | FLOAT_LIT { $$ = (Object){OBJECT_TYPE_FLOAT, (*(uint32_t*)&$<f_var>1), 0, NULL}; }
    | INT_LIT { $$ = (Object){OBJECT_TYPE_INT, (*(uint32_t*)&$<i_var>1), 0, NULL}; }
    | STR_LIT { $$ = (Object){OBJECT_TYPE_STR, (*(uint64_t*)&$<s_var>1), 0, NULL}; }
    | IDENT { Object* o = findVariable($<s_var>1);
        if(!o) yyerrorf("variable '%s' not declared\n", $<s_var>1);
        $$ = *o;
    }
    | IDENT '[' INT_LIT ']' { Object* o = findVariable($<s_var>1);
        if(!o) yyerrorf("variable '%s' not declared\n", $<s_var>1);
        $$ = (Object){o->type, VAR_FLAG_PTR_VALUE, $<i_var>3, o->symbol};
    }
;

%%
/* C code section */