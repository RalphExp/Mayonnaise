%{
#include <stdio.h>
#include "parser/lexer.hh"
%}

// version of bison
%require "3.8"

// use c++ language
%language "c++"

// c++ namespace
%define api.namespace {parser}

// parser's class name
%define api.parser.class {Parser}

// use variant instead of the old union format
%define api.value.type variant

// get more detailed information when error occurs
%define parse.error detailed

// parser constructor's parameter, seems it is much easier 
// to use yyscan_t than c++ class
%param {yyscan_t lexer}

// tracking location
%locations

// 6 sr-conflicts, shifting is always the correct way to solve.
// %expect 6 

%code requires
{
    #include <vector>
    #include <string>
    #include <sstream>

    #include "token.h"
    #include "node.h"
    #include "type.h"

    using namespace may;

    string string_value(const string& image);
    char character_code(const string& image);
    IntegerLiteralNode* integer_node(const Location &loc, const string& image);
}

// inefficient but works
%code provides
{
    #define YY_USER_ACTION \
        loc->begin.line = loc->end.line = yylineno; \
        loc->begin.column = yycolumn + 1; \
        for (int i = 0; yytext[i] != 0; i++) { \
            if (yytext[i] == '\n') { \
                loc->end.line++; \
                loc->end.column = 0; \
                yycolumn = 0; \
            } else { \
                loc->end.column++; \
                ++yycolumn; \
            } \
        }
}

%code provides
{
    #define YY_SET_LOCATION \
        tok.begin_line_ = loc->begin.line; \
        tok.begin_column_ = loc->begin.column; \
        tok.end_line_ = loc->end.line; \
        tok.end_column_ = loc->end.column; \
        yylval->emplace<Token>(tok); \
        return tok.kind_;
}

// out yylex version
%code provides
{
    #define YY_DECL \
        int yylex(parser::Parser::semantic_type *yylval, \
            parser::Parser::location_type *loc, yyscan_t yyscanner) \

    YY_DECL;
}

%token <Token> VOID CHAR SHORT INT LONG 
%token <Token> TYPEDEF STRUCT UNION ENUM 
%token <Token> STATIC EXTERN 
%token <Token> SIGNED UNSIGNED CONST
%token <Token> IF ELSE SWITCH CASE DEFAULT WHILE DO FOR RETURN BREAK CONTINUE GOTO
%token <Token> IMPORT SIZEOF
%token <Token> IDENTIFIER TYPENAME INTEGER CHARACTER STRING

%type <int> compilation_unit import_stmts top_defs
%type <int> def_func def_vars def_const def_union def_typedef
%type <int> import_stmt
%type <vector<StmtNode*>> stmts
%type <StmtNode*> stmt
%type <StmtNode*> block if_stmt while_stmt dowhile_stmt for_stmt switch_stmt
%type <StmtNode*> label_stmt break_stmt continue_stmt return_stmt goto_stmt
%type <vector<Slot>> slots member_list
%type <TypeRef*> typeref_base typeref
%type <TypeNode*> type
%type <vector<ExprNode*>> args
%type <ExprNode*> opt_expr
%type <ExprNode*> term
%type <ExprNode*> expr10 expr9 expr8 expr7 expr6 expr5 expr4 expr3 expr2 expr1
%type <ExprNode*> expr
%type <ExprNode*> postfix
%type <ExprNode*> primary unary
%type <string> name assign_op

%start compilation_unit

%destructor { delete $$; } <StmtNode*> 
%destructor { delete $$; } <ExprNode*>
%destructor { for (auto *expr : $$) delete expr; } <vector<ExprNode*>>

%%
compilation_unit : top_defs 
        | import_stmts top_defs
        ;

import_stmts : import_stmt 
        | import_stmts import_stmt 
        ;

import_stmt : IMPORT ';' 


top_defs : def_func
        | def_vars
        | def_const 
        | def_struct 
        | def_union 
        | def_typedef
        | top_defs def_func 
        | top_defs def_vars 
        | top_defs def_const 
        | top_defs def_struct 
        | top_defs def_union 
        | top_defs def_typedef 
        ;

/* XXX: typeref and type is different? */
def_func : type name '(' VOID ')' block 
        | STATIC type name '(' VOID ')' block
        | type name '(' params ')' block 
        | STATIC type name '(' params ')' block 
        ;

def_var_list : def_vars 
        | def_var_list def_vars 
        ;

def_vars : type name '=' expr ';' 
        | type name ';' 
        | def_vars ',' type name '=' expr ';' 
        | def_vars ',' type name ';' 
        | STATIC type name '=' expr ';' 
        | STATIC type name ';' 
        | def_vars ',' STATIC type name '=' expr ';' 
        | def_vars ',' STATIC type name ';' 
        ;

def_const : CONST type name '=' expr ';'  

 /* TODO: need to check null struct */
def_struct : STRUCT name member_list ';'
    
/* TODO: need to check null union */
def_union : UNION name member_list ';'


def_typedef : TYPEDEF typeref IDENTIFIER ';'

params : fixed_params 
        | fixed_params ',' "..."
        ;

fixed_params : param
        | fixed_params ',' param 
        ;

param_typerefs: fixed_param_typerefs
        | fixed_param_typerefs ',' "..."
        ;

fixed_param_typerefs : typeref
        | fixed_param_typerefs ',' typeref
        ;

param : type name
        ;

block : '{' '}' 
        | '{' stmts '}'
        | '{' def_var_list stmts '}'
        ;

type : typeref
        ;

typeref : typeref_base 
        | typeref_base '[' ']'
        | typeref_base '[' INTEGER ']'
        | typeref_base '*'
        | typeref_base '(' VOID ')'
        | typeref_base '(' param_typerefs ')'
        | typeref '[' ']'
        | typeref '[' INTEGER ']'
        | typeref '*'
        | typeref '(' VOID ')'
        | typeref '(' param_typerefs ')'
        ;

stmts : stmt { $$ = vector<StmtNode*>{$1}; }
        | stmts stmt {
              if ($2) {
                  $1.push_back($2);
              }
              $$ = move($1); 
          }; 
        ;

stmt : ';' { $$ = nullptr; }
        | label_stmt     { $$ = $1; }
        | expr ';'       { $$ = new ExprStmtNode($1->location(), $1); }
        | block          { $$ = $1; }
        | if_stmt        { $$ = $1; }
        | while_stmt     { $$ = $1; }
        | dowhile_stmt   { $$ = $1; }
        | for_stmt       { $$ = $1; }
        | switch_stmt    { $$ = $1; }
        | break_stmt     { $$ = $1; }
        | continue_stmt  { $$ = $1; }
        | goto_stmt      { $$ = $1; }
        | return_stmt    { $$ = $1; }
        ;

label_stmt : IDENTIFIER ':' stmt
        ;

if_stmt : IF '(' expr ')' stmt ELSE stmt
        | IF '(' expr ')' stmt
        ;

while_stmt : WHILE '(' expr ')' stmt

dowhile_stmt : DO stmt WHILE '(' expr ')' ';'

for_stmt : FOR '(' opt_expr ';' opt_expr ';' opt_expr ')' stmt

goto_stmt : GOTO IDENTIFIER ';' { $$ = new GotoNode(Location($1), $2.image_); }

switch_stmt : SWITCH '(' expr ')' '{' case_clauses '}'

case_clauses : default_clause
        | case_clause
        | case_clauses default_clause
        | case_clauses case_clause
        ;

case_clause : cases
        | cases case_body
        ;

cases : CASE primary ':'

case_body : stmt

default_clause : DEFAULT ':'
        | DEFAULT ':' case_body
        ;

return_stmt : RETURN ';'  { $$ = new ReturnNode(Location($1), nullptr); }
        | RETURN expr ';' { $$ = new ReturnNode(Location($1), $2); }
        ;

continue_stmt : CONTINUE ';' { $$ = new ContinueNode(Location($1)); }

break_stmt : BREAK ';' { $$ = new BreakNode(Location($1)); }

member_list : '{' '}'   { $$ = vector<Slot>{}; }
        | '{' slots '}' { $$ = move($2); }
        ;

slots : type name ';' { 
                        $$ = vector<Slot>{}; 
                        $$.push_back(Slot($1, $2)); 
                      }
        | slots type name ';' { 
                        $1.push_back(Slot($2, $3)); 
                        $$ = move($1); 
                      }
        ;

opt_expr : %empty { $$ = nullptr; }
        | expr { $$ = $1; }
        ;

typeref_base : VOID { $$ = new VoidTypeRef(Location($1)); }
        | CHAR { $$ = IntegerTypeRef::char_ref(Location($1)); }
        | SHORT { $$ = IntegerTypeRef::short_ref(Location($1)); }
        | INT { $$ = IntegerTypeRef::int_ref(Location($1)); }
        | LONG { $$ = IntegerTypeRef::long_ref(Location($1)); }
        | UNSIGNED CHAR { $$ = IntegerTypeRef::uchar_ref(Location($1)); }
        | UNSIGNED SHORT { $$ = IntegerTypeRef::ushort_ref(Location($1)); }
        | UNSIGNED INT { $$ = IntegerTypeRef::uint_ref(Location($1)); }
        | UNSIGNED LONG { $$ = IntegerTypeRef::ulong_ref(Location($1)); }
        | STRUCT IDENTIFIER { $$ = new StructTypeRef(Location($1), $2.image_); }
        | UNION IDENTIFIER { $$ = new UnionTypeRef(Location($1), $2.image_); }
        | TYPENAME { $$ = new UserTypeRef(Location($1), $1.image_);; }
        ;

expr : term '=' expr { $$ = new AssignNode($1, $3); }
    | term assign_op expr { $$ = new OpAssignNode($1, $2, $3); }
    | expr10 { $$ = $1; }
    ;

assign_op : "+=" { $$ = "+"; }
        | "-="   { $$ = "-"; }
        | "*="   { $$ = "*"; }
        | "/="   { $$ = "/"; }
        | "%="   { $$ = "%"; }
        | "&="   { $$ = "&"; }
        | "|="   { $$ = "|"; }
        | "^="   { $$ = "^"; }
        | "<<="  { $$ = "<<"; }
        | ">>="  { $$ = ">>"; }
        ;

expr10 : expr10 '?' expr ':' expr9 { $$ = new CondExprNode($1, $3, $5); }
        | expr9 { $$ = $1; }
        ;

expr9 : expr8 { $$ = $1; }
        | expr9 "||" expr8 { $$ = new LogicalOrNode($1, $3); }
        ;

expr8 : expr7 { $$ = $1; }
        | expr8 "&&" expr7 { $$ = new LogicalAndNode($1, $3); }
        ;

expr7 : expr6 { $$ = $1; }
        | expr7 '>' expr6 { $$ = new BinaryOpNode($1, ">", $3); }
        | expr7 '<' expr6 { $$ = new BinaryOpNode($1, "<", $3); }
        | expr7 ">=" expr6 { $$ = new BinaryOpNode($1, ">=", $3); }
        | expr7 "<=" expr6 { $$ = new BinaryOpNode($1, "<=", $3); }
        | expr7 "==" expr6 { $$ = new BinaryOpNode($1, "==", $3); }
        | expr7 "!=" expr6 { $$ = new BinaryOpNode($1, "!=", $3); }
        ;

expr6 : expr5 { $$ = $1; }
        | expr6 '|' expr5 { $$ = new BinaryOpNode($1, "|", $3); }
        ;

expr5 : expr4 { $$ = $1; }
        | expr5 '^' expr4 { $$ = new BinaryOpNode($1, "^", $3); }
        ;
        
expr4 : expr3 { $$ = $1; }
        | expr4 '&' expr3 { $$ = new BinaryOpNode($1, "&", $3); }
        ;

expr3 : expr2 { $$ = $1; }
        | expr3 ">>" expr2 { $$ = new BinaryOpNode($1, ">>", $3); }
        | expr3 "<<" expr2 { $$ = new BinaryOpNode($1, "<<", $3); }
        ;

expr2 : expr1 { $$ = $1; }
        | expr2 '+' expr1 { $$ = new BinaryOpNode($1, "+", $3); }
        | expr2 '-' expr1 { $$ = new BinaryOpNode($1, "-", $3); }
        ;

expr1 : term { $$ = $1; }
        | expr1 '*' term { $$ = new BinaryOpNode($1, "*", $3); }
        | expr1 '/' term { $$ = new BinaryOpNode($1, "/", $3); }
        | expr1 '%' term { $$ = new BinaryOpNode($1, "%", $3); }
        ;

term : '(' type ')' term { $$ = new CastNode($2, $4); }
        | unary { $$ = $1; }
        ;

unary : "++" unary { $$ = new PrefixOpNode("++", $2); }
        | "--" unary { $$ = new PrefixOpNode("--", $2); }
        | '+' term { $$ = new UnaryOpNode("+", $2); }
        | '-' term { $$ = new UnaryOpNode("-", $2); }
        | '!' term { $$ = new UnaryOpNode("!", $2); }
        | '~' term { $$ = new UnaryOpNode("~", $2); }
        | '*' term { $$ = new DereferenceNode($2); }
        | '&' term { $$ = new AddressNode($2); }
        | SIZEOF '(' type ')' { $$ = new SizeofTypeNode($3, IntegerTypeRef::ulong_ref()); }
        | SIZEOF unary { $$ = new SizeofExprNode($2, IntegerTypeRef::ulong_ref()); }
        | postfix { $$ = $1; }
        ;

postfix : primary { $$ = $1; }
        | postfix "++" { $$ = new SuffixOpNode("++", $1); }
        | postfix "--" { $$ = new SuffixOpNode("--", $1); }
        | postfix '[' expr ']' { $$ = new ArefNode($1, $3); }
        | postfix '.' name { $$ = new MemberNode($1, $3); }
        | postfix "->" name { $$ = new PtrMemberNode($1, $3); }
        | postfix '(' ')' { $$ = new FuncallNode($1, vector<ExprNode*>{}); }
        | postfix '(' args ')' { $$ = new FuncallNode($1, $3); }
        ;

name : IDENTIFIER {  $$ = $1.image_; } 


args : expr { $$ = vector<ExprNode*> {$1}; }
        | args ',' expr { $1.push_back($3); 
                          $$ = move($1); }
        ;

primary : INTEGER       { $$ = integer_node(Location($1), $1.image_); }
        | CHARACTER     { $$ = new IntegerLiteralNode(Location($1), 
                              IntegerTypeRef::char_ref(), $1.image_[0]);
                        }
        | STRING        { $$ = new StringLiteralNode(Location($1),
                              new PointerTypeRef(IntegerTypeRef::char_ref()), $1.image_);
                        }
        | IDENTIFIER    { $$ = new VariableNode(Location($1), $1.image_);}
        | '(' expr ')'  { $$ = $2; }
        ;

%%

IntegerLiteralNode* integer_node(const Location &loc, const string& image)
{
    long i = stol(image);
    if (image.size() >= 3 && image.substr(image.size()-2,2) == "UL")
        return new IntegerLiteralNode(loc, IntegerTypeRef::ulong_ref(), i);
    if (image.size() >= 2 && image.substr(image.size()-1,1) == "L")
        return new IntegerLiteralNode(loc, IntegerTypeRef::long_ref(), i);
    if (image.size() >= 2 && image.substr(image.size()-1,1) == "U")
        return new IntegerLiteralNode(loc, IntegerTypeRef::uint_ref(), i);
    return new IntegerLiteralNode(loc, IntegerTypeRef::int_ref(), i);
} 

void parser::Parser::error(const location_type& loc, const std::string& msg) 
{
    printf("%s at (line %d, column: %d)\n", msg.c_str(),
        loc.begin.line, loc.begin.column);
}
