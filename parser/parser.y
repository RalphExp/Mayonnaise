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
%expect 6

%code requires
{
    #include <vector>
    #include <string>
    #include <sstream>

    #include "ast.h"
    #include "decl.h"
    #include "token.h"
    #include "node.h"
    #include "type.h"
    #include "loader.h"
    #include "entity.h"
    #include "option.h"

    using namespace cbc;

    // see lexer.l
    extern long integer_value(const string& image);

    string string_value(const string& image);
    char character_code(const string& image);
    Location loc(yyscan_t lexer, const Token& token);
    IntegerLiteralNode* integer_node(const Location &loc, const string& image);

    // util functions
    Option* get_option(yyscan_t);
    string get_src_file(yyscan_t);
    Loader& get_loader(yyscan_t);
    set<string>& get_typename(yyscan_t);
    void add_known_types(Declarations*, yyscan_t);
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
    int yylex(parser::Parser::semantic_type *yylval, \
            parser::Parser::location_type *loc, yyscan_t yyscanner);

    #define YY_DECL \
        int _yylex(parser::Parser::semantic_type *yylval, \
            parser::Parser::location_type *loc, yyscan_t yyscanner) \

    YY_DECL;

    #define ZERO(x) x = nullptr
    #define DZERO(x) delete x; x = nullptr
}

%token <Token> COMPILE DECLARE ERROR
%token <Token> '{' '}' '(' ')'
%token <Token> PLUS_PLUS MINUS_MINUS AND_AND OR_OR LSHIFT RSHIFT
%token <Token> EQ NE LE GE
%token <Token> PLUS_ASSIGN MINUS_ASSIGN MULTIPLY_ASSIGN DIVIDE_ASSIGN MOD_ASSIGN
%token <Token> AND_ASSIGN OR_ASSIGN XOR_ASSIGN LSHIFT_ASSIGN RSHIFT_ASSIGN
%token <Token> POINT_TO ELLIPSIS
%token <Token> VOID CHAR SHORT INT LONG
%token <Token> TYPEDEF STRUCT UNION ENUM
%token <Token> STATIC EXTERN
%token <Token> SIGNED UNSIGNED CONST
%token <Token> IF ELSE SWITCH CASE DEFAULT WHILE DO FOR RETURN BREAK CONTINUE GOTO
%token <Token> IMPORT SIZEOF
%token <Token> IDENTIFIER TYPENAME INTEGER CHARACTER STRING

%type <int> compilation_or_declaraion
%type <Declarations*> import_stmts top_defs
%type <string> import_stmt import_component
%type <DefinedFunction*> def_func
%type <UndefinedFunction*> decl_func
%type <vector<DefinedVariable*>> def_var_list def_vars
%type <UndefinedVariable*> decl_var
%type <StructNode*> def_struct
%type <UnionNode*> def_union
%type <Constant*> def_const
%type <TypedefNode*> def_typedef
%type <Params*> params fixed_params
%type <Parameter*> param
%type <vector<StmtNode*>> stmts
%type <StmtNode*> stmt
%type <LabelNode*> label_stmt
%type <IfNode*> if_stmt
%type <DoWhileNode*> dowhile_stmt
%type <WhileNode*> while_stmt
%type <ForNode*> for_stmt
%type <SwitchNode*> switch_stmt
%type <GotoNode*> goto_stmt
%type <ReturnNode*> return_stmt
%type <ContinueNode*> continue_stmt
%type <BreakNode*> break_stmt 
%type <vector<CaseNode*>> case_clauses
%type <CaseNode*> case_clause
%type <BlockNode*> case_body block
%type <ParamTypeRefs*> param_typerefs
%type <ParamTypeRefs*> fixed_param_typerefs
%type <TypeRef*> typeref_base typeref
%type <vector<Slot*>> slots member_list
%type <vector<ExprNode*>> cases
%type <vector<ExprNode*>> args
%type <TypeNode*> type
%type <ExprNode*> opt_expr term expr
%type <ExprNode*> expr10 expr9 expr8 expr7 expr6 expr5 expr4 expr3 expr2 expr1
%type <ExprNode*> postfix
%type <ExprNode*> primary unary
%type <string> name assign_op

%start compilation_or_declaraion

%%

compilation_or_declaraion : COMPILE top_defs {
              Token token;
              token.begin_line_ = @1.begin.line;
              token.begin_column_ = @1.begin.column; 
              // use a pseudo token to get its location
              auto* ast = new AST(loc(lexer, token), $2);
              auto* option = get_option(lexer);
              option->ast_ = ast;
          }
        | COMPILE import_stmts top_defs {
              Token token;
              token.begin_line_ = @$.begin.line;
              token.begin_column_ = @$.begin.column; 
              $3->add($2);
              auto* ast = new AST(loc(lexer, token), $3);
              auto* option = get_option(lexer);
              option->ast_ = ast;
              // now $2 can be safely deleted;
              DZERO($2);
          }
        | DECLARE import_stmts {
              auto* option = get_option(lexer);
              if ($2->defvars().size()) {
                  throw string("can not define variable in .hb: ") + \
                      option->src_; 
              } else if ($2->deffuncs().size()) {
                  throw string("can not define function in .hb: ") + \
                      option->src_; 
              }
              option->decl_ = $2;
          }
        | DECLARE top_defs {
              auto* option = get_option(lexer);
              if ($2->defvars().size()) {
                  throw string("can not define variable in .hb: ") + \
                      option->src_; 
              } else if ($2->deffuncs().size()) {
                  throw string("can not define function in .hb: ") + \
                      option->src_; 
              }
              option->decl_ = $2;
          }
        | DECLARE import_stmts top_defs {
              auto* option = get_option(lexer);
              $3->add($2);
              if ($3->defvars().size()) {
                  /* TODO: print location */
                  throw string("can not define variable in .hb: ") + \
                      option->src_; 
              } else if ($3->deffuncs().size()) {
                  throw string("can not define function in .hb: ") + \
                      option->src_; 
              }
              option->decl_ = $3;
              // now $2 can be safely deleted;
              DZERO($2);
          }
        ;

import_stmts : import_stmt {
              $$ = new Declarations;
              auto decls = get_loader(lexer).load_library($1);
              if (decls) {
                  $$->add(decls);
                  add_known_types(decls, lexer);
              }
          }
        | import_stmts import_stmt {
              auto* decls = get_loader(lexer).load_library($2);
              if (decls) {
                 $1->add(decls);
                 add_known_types(decls, lexer);
                 // decls is managed by loader
                 // don't delete decls;
              }
              $$ = $1;
          }
        ;

import_stmt : IMPORT import_component ';' {
              $$ = $2;
          }

import_component : name {
              $$ = $1;
          }

import_component : import_component '.' name {
              $$ = $1 + "." + $3;
          }

top_defs : def_func { $$ = new Declarations; $$->add_deffunc($1); ZERO($1); }
        | def_vars ';' { $$ = new Declarations; $$->add_defvars(move($1)); }
        | decl_func { $$ = new Declarations; $$->add_declfunc($1); ZERO($1); }
        | decl_var { $$ = new Declarations; $$->add_declvar($1); ZERO($1); }
        | def_const { $$ = new Declarations; $$->add_constant($1); ZERO($1); }
        | def_struct { $$ = new Declarations; $$->add_defstruct($1); ZERO($1); }
        | def_union { $$ = new Declarations; $$->add_defunion($1); ZERO($1); }
        | def_typedef { $$ = new Declarations; $$->add_typedef($1); ZERO($1); }
        | top_defs def_func { $1->add_deffunc($2); $$ = $1; ZERO($1); }
        | top_defs def_vars ';' { $1->add_defvars(move($2)); $$ = $1; }
        | top_defs decl_func { $1->add_declfunc($2); $$ = $1; ZERO($1); }
        | top_defs decl_var { $1->add_declvar($2); $$ = $1; ZERO($1); }
        | top_defs def_const { $1->add_constant($2); $$ = $1; ZERO($1); }
        | top_defs def_struct { $1->add_defstruct($2); $$ = $1; ZERO($1); }
        | top_defs def_union { $1->add_defunion($2); $$ = $1; ZERO($1); }
        | top_defs def_typedef { $1->add_typedef($2); $$ = $1; ZERO($1); }
        ;

def_func : typeref name '(' ')' block {
              auto v = vector<Parameter*>{};
              auto params = new Params(loc(lexer, $3), move(v));
              auto ref = new FunctionTypeRef($1, // return type
                    move(params->parameter_typerefs())); // typeref

              $$ = new DefinedFunction(false, // priv
                    new TypeNode(ref), // type
                    $2, // name 
                    params, // params
                    $5); // body
              ZERO($1);
              ZERO($5);
          }
        | typeref name '(' VOID ')' block {
              auto v = vector<Parameter*>{};
              auto params = new Params(loc(lexer, $4), move(v));
              auto ref = new FunctionTypeRef($1, // return type
                    move(params->parameter_typerefs())); // typeref

              $$ = new DefinedFunction(false, // priv
                    new TypeNode(ref), // type
                    $2, // name 
                    params, // params
                    $6); // body
              ZERO($1);
              ZERO($6);
          }
        | STATIC typeref name '(' ')' block {
              auto v = vector<Parameter*>{};
              auto params = new Params(loc(lexer, $4), move(v));
              auto ref = new FunctionTypeRef(
                  $2, move(params->parameter_typerefs()));

              $$ = new DefinedFunction(true, // priv
                      new TypeNode(ref), 
                      $3, // name
                      params, // params
                      $6); // boddy
              ZERO($2);
              ZERO($6);
          }
        | STATIC typeref name '(' VOID ')' block {
              auto v = vector<Parameter*>{};
              auto params = new Params(loc(lexer, $5), move(v));
              auto ref = new FunctionTypeRef(
                  $2, move(params->parameter_typerefs()));

              $$ = new DefinedFunction(true, // priv
                      new TypeNode(ref), 
                      $3, // name
                      params, // params
                      $7); // boddy
              ZERO($2);
              ZERO($7);
          }
        | typeref name '(' params ')' block {
              auto ref = new FunctionTypeRef($1, move($4->parameter_typerefs()));
              $$ = new DefinedFunction(false, 
                    new TypeNode(ref), 
                    $2, $4, $6);
              ZERO($1);
              ZERO($4);
              ZERO($6);
          }
        | STATIC typeref name '(' params ')' block {
              auto ref = new FunctionTypeRef($2, move($5->parameter_typerefs()));
              $$ = new DefinedFunction(false, 
                    new TypeNode(ref), 
                    $3, $5, $7);
              ZERO($2);
              ZERO($5);
              ZERO($7);
          }
        ;

decl_func : EXTERN typeref name '(' ')' ';' {
              auto v = vector<Parameter*>{};
              auto params = new Params(loc(lexer, $4), move(v));
              auto ref = new FunctionTypeRef($2, // return type
                      move(params->parameter_typerefs())); // typeref

              $$ = new UndefinedFunction(
                    new TypeNode(ref), // type
                    $3, // name
                    params);
              ZERO($2);
          }
        | EXTERN typeref name '(' VOID ')' ';' {
              auto v = vector<Parameter*>{};
              auto params = new Params(loc(lexer, $4), move(v));
              auto ref = new FunctionTypeRef($2, // return type
                      move(params->parameter_typerefs())); // typeref

              $$ = new UndefinedFunction(
                    new TypeNode(ref), // type
                    $3, // name
                    params);
              ZERO($2);
          }
        | EXTERN typeref name '(' params ')' ';' {
              auto ref = new FunctionTypeRef($2, // return type
                      move($5->parameter_typerefs())); // typeref

              $$ = new UndefinedFunction(
                  new TypeNode(ref), // type
                  $3, // name
                  $5);
              ZERO($2);
              ZERO($5);
          }
        ;

decl_var : EXTERN typeref name ';' { 
             TypeNode* type = new TypeNode($2);
             $$ = new UndefinedVariable(type, $3); 
          }
        ;

def_var_list : def_vars ';' { $$ = move($1); }
        | def_var_list def_vars ';' {
              for (auto* v : $2) {
                  v->inc_ref();
                  $1.push_back(v);
              }
              $$ = move($1);
              $2.clear();
          }
        ;

def_vars : typeref name {
              TypeNode* type = new TypeNode($1);
              auto p = new DefinedVariable(false, type, $2, nullptr);
              $$ = vector<DefinedVariable*>{p};
              ZERO($1);
          }
        | typeref name '=' expr {
              TypeNode* type = new TypeNode($1);
              auto p = new DefinedVariable(false, type, $2, $4);
              $$ = vector<DefinedVariable*>{p};
              ZERO($4);
          }
        | STATIC typeref name {
              TypeNode* type = new TypeNode($2);
              auto p = new DefinedVariable(true, type, $3, nullptr);
              $$ = vector<DefinedVariable*>{p};
              ZERO($2);
          }
        | STATIC typeref name '=' expr {
              TypeNode* type = new TypeNode($2);
              auto p = new DefinedVariable(true, type, $3, $5);
              $$ = vector<DefinedVariable*>{p};
              ZERO($2);
              ZERO($5);
          }
        | def_vars ',' name {
              TypeNode* type = $1.back()->type_node();
              bool is_private = $1.back()->is_private();
              auto p = new DefinedVariable(is_private, type, $3, nullptr);
              $1.push_back(p);
              $$ = move($1);
          }
        | def_vars ',' name '=' expr {
              TypeNode* type = $1.back()->type_node();
              bool is_private = $1.back()->is_private();
              auto p = new DefinedVariable(is_private, type, $3, $5);
              $1.push_back(p);
              $$ = move($1);
              ZERO($5);
          }
        ;
         
def_const : CONST typeref name '=' expr ';' {
              TypeNode* type = new TypeNode($2);
              $$ = new Constant(type, $3, $5);
              ZERO($2);
              ZERO($5);
          }
        ;


def_struct : STRUCT name member_list ';' {
              auto p = new StructTypeRef($2);
              $$ = new StructNode(loc(lexer, $1), p, $2, move($3));
          }
        ;

def_union : UNION name member_list ';' {
              auto p = new UnionTypeRef($2);          
              $$ = new UnionNode(loc(lexer, $1), p, $2, move($3));
          }
        ;

def_typedef : TYPEDEF typeref IDENTIFIER ';' {
              $$ = new TypedefNode(loc(lexer, $1), $2, $3.image_);
              auto& s = get_typename(lexer);
              s.insert($3.image_);
              ZERO($2);
          }
        ;

params : fixed_params { $$ = move($1); }
        | fixed_params ',' ELLIPSIS { 
              $1->accept_varargs();
              $$ = move($1);
          }
        ;

fixed_params : param {
              auto v = vector<Parameter*>{$1};
              $$ = new Params($1->location(), move(v));
          }
        | fixed_params ',' param  {
              assert($3->get_oref() == 1);
              $1->param_descs_.push_back($3);
              $$ = $1;
          }
        ;

param : typeref name {
              TypeNode* type = new TypeNode($1);
              $$ = new Parameter(type, $2);
              ZERO($1);
          }
        ;

block : '{' '}' { 
              auto v = vector<DefinedVariable*>{};
              auto v2 = vector<StmtNode*>{};
              $$ = new BlockNode(loc(lexer, $1), move(v), move(v2));
          }
        | '{' stmts '}' {
              auto v = vector<DefinedVariable*>{};
              $$ = new BlockNode(loc(lexer, $1), move(v), move($2));
          }
        | '{' def_var_list '}' {
              auto v = vector<StmtNode*>{};
              $$ = new BlockNode(loc(lexer, $1), move($2), move(v));
          }
        | '{' def_var_list stmts '}' {
              $$ = new BlockNode(loc(lexer, $1), move($2), move($3));
          }
        ;

type : typeref { 
              $$ = new TypeNode($1);
              ZERO($1);
          }
        ;

typeref : typeref_base  { $$ = $1; ZERO($1); }
        | typeref_base '[' ']' { $$ = new ArrayTypeRef($1); ZERO($1);}
        | typeref_base '[' INTEGER ']' {
              $$ = new ArrayTypeRef($1, integer_value($3.image_));
              ZERO($1);
          }
        | typeref_base '*' { $$ = new PointerTypeRef($1); ZERO($1); }
        | typeref_base '(' VOID ')' {
              auto v = vector<TypeRef*>{};;
              auto param = new ParamTypeRefs(move(v));
              $$ = new FunctionTypeRef($1, param);
              ZERO($1);
          }
        | typeref_base '(' param_typerefs ')' {
              $$ = new FunctionTypeRef($1, $3);
              ZERO($1);
          }
        | typeref '[' ']' { $$ = new ArrayTypeRef($1); ZERO($1); }
        | typeref '[' INTEGER ']' {
              $$ = new ArrayTypeRef($1, integer_value($3.image_));
              ZERO($1);
          }
        | typeref '*' { $$ = new PointerTypeRef($1); ZERO($1); }
        | typeref '(' VOID ')' {
              auto v = vector<TypeRef*>{};
              auto param = new ParamTypeRefs(move(v));
              $$ = new FunctionTypeRef($1, param);
              ZERO($1);
          }
        | typeref '(' param_typerefs ')' { 
              $$ = new FunctionTypeRef($1, $3);
              ZERO($1); 
          }
        ;

param_typerefs: fixed_param_typerefs { $$ = move($1); }
        | fixed_param_typerefs ',' ELLIPSIS {
              $$ = move($1);
              $$->accept_varargs();
          }
        ;

fixed_param_typerefs : typeref {
              assert($1->get_oref() == 1);
              auto v = vector<TypeRef*>{$1};
              $$ = new ParamTypeRefs(move(v));
              ZERO($1);
          }
        | fixed_param_typerefs ',' typeref {
              assert($3->get_oref() == 1);
              $1->param_descs_.push_back($3);
              $$ = $1;
              ZERO($3);
          }
        ;

stmt : ';' { $$ = nullptr; }
        | label_stmt     { $$ = $1; ZERO($1); }
        | expr ';'       { $$ = new ExprStmtNode($1->location(), $1); ZERO($1); }
        | block          { $$ = $1; ZERO($1); }
        | if_stmt        { $$ = $1; ZERO($1); }
        | while_stmt     { $$ = $1; ZERO($1); }
        | dowhile_stmt   { $$ = $1; ZERO($1); }
        | for_stmt       { $$ = $1; ZERO($1); }
        | switch_stmt    { $$ = $1; ZERO($1); }
        | break_stmt     { $$ = $1; ZERO($1); }
        | continue_stmt  { $$ = $1; ZERO($1); }
        | goto_stmt      { $$ = $1; ZERO($1); }
        | return_stmt    { $$ = $1; ZERO($1); }
        ;

label_stmt : IDENTIFIER ':' stmt {
              $$ = new LabelNode(loc(lexer, $1), $1.image_, $3);
              ZERO($3);
          }
        ;

if_stmt : IF '(' expr ')' stmt ELSE stmt {
              $$ = new IfNode(loc(lexer, $1), $3, $5, $7);
              ZERO($3);
              ZERO($5);
              ZERO($7);
          }
        | IF '(' expr ')' stmt {
              $$ = new IfNode(loc(lexer, $1), $3, $5);
              ZERO($3);
              ZERO($5);
          }
        ;

while_stmt : WHILE '(' expr ')' stmt {
              $$ = new WhileNode(loc(lexer, $1), $3, $5);
              ZERO($3);
              ZERO($5);
          }
        ;

dowhile_stmt : DO stmt WHILE '(' expr ')' ';' {
              $$ = new DoWhileNode(loc(lexer, $1), $2, $5);
              ZERO($2);
              ZERO($5);
          }
        ;

for_stmt : FOR '(' opt_expr ';' opt_expr ';' opt_expr ')' stmt {
              $$ = new ForNode(loc(lexer, $1), $3, $5, $7, $9);
              ZERO($3);
              ZERO($5);
              ZERO($7);
              ZERO($9);
          }
        ;

goto_stmt : GOTO IDENTIFIER ';' {
              $$ = new GotoNode(loc(lexer, $1), $2.image_);
          }
        ;

switch_stmt : SWITCH '(' expr ')' '{' case_clauses '}' {
              $$ = new SwitchNode(loc(lexer, $1), $3, move($6));
              ZERO($3);
          }
        ;

case_clauses : case_clause {
              $$ = vector<CaseNode*>{$1};
              ZERO($1);
          }
        | case_clauses case_clause {
              $1.push_back($2);
              $$ = move($1);
              ZERO($2);
          }
        ;

case_clause : cases case_body {
              $$ = new CaseNode($2->location(), move($1), $2 /* BlockNode */ );
              ZERO($2);
          }
        ;

/* need to check invalid cases */
cases : CASE primary ':' {
              $$ = vector<ExprNode*>{};
              $$.push_back($2);
              ZERO($2);
          }
        | DEFAULT ':' {
              /* TODO: fix this */
              $$ = vector<ExprNode*>{};
          }
        | cases CASE primary ':' {
              $1.push_back($3);
              $$ = move($1);
              ZERO($3);
          }
        | cases DEFAULT ':' {
              /* TODO: fix this */
              $1.push_back(nullptr);
              $$ = move($1);
          }
        ;

case_body : stmts {
              /* don't need to check break, C-Language switch
              * statement and have no breaks. */
              auto v = vector<DefinedVariable*>{};
              $$ = new BlockNode($1[0]->location(), move(v), move($1));
          }
        ;

return_stmt : RETURN ';'  {
              $$ = new ReturnNode(loc(lexer, $1), nullptr);
          }
        | RETURN expr ';' {
              $$ = new ReturnNode(loc(lexer, $1), $2);
          }
        ;

continue_stmt : CONTINUE ';' {
              $$ = new ContinueNode(loc(lexer, $1));
          }
        ;

break_stmt : BREAK ';' {
              $$ = new BreakNode(loc(lexer, $1));
          }
        ;

stmts : stmt {
              $$ = vector<StmtNode*>{};
              if ($1) {
                  $$.push_back($1); 
              }
              ZERO($1);
          }
        | stmts stmt {
              if ($2) {
                  $1.push_back($2);
              }
              $$ = move($1);
              ZERO($2);
          }
        ;

member_list : '{' '}'   { $$ = vector<Slot*>{}; }
        | '{' slots '}' { $$ = move($2); }
        ;

slots : type name ';' {
              $$ = vector<Slot*>{};
              auto s = new Slot($1, $2);
              $$.emplace_back(s);
              ZERO($1);
          }
        | slots type name ';' {
              auto s = new Slot($2, $3);
              $1.emplace_back(s);
              $$ = move($1);
              ZERO($2);
          }
        ;

opt_expr : %empty { $$ = nullptr; }
        | expr { $$ = $1; ZERO($1); }
        ;

typeref_base : VOID { $$ = new VoidTypeRef(loc(lexer, $1)); }
        | CHAR { $$ = IntegerTypeRef::char_ref(loc(lexer, $1)); }
        | SHORT { $$ = IntegerTypeRef::short_ref(loc(lexer, $1)); }
        | INT { $$ = IntegerTypeRef::int_ref(loc(lexer, $1)); }
        | LONG { $$ = IntegerTypeRef::long_ref(loc(lexer, $1)); }
        | UNSIGNED CHAR { $$ = IntegerTypeRef::uchar_ref(loc(lexer, $1)); }
        | UNSIGNED SHORT { $$ = IntegerTypeRef::ushort_ref(loc(lexer, $1)); }
        | UNSIGNED INT { $$ = IntegerTypeRef::uint_ref(loc(lexer, $1)); }
        | UNSIGNED LONG {  $$ = IntegerTypeRef::ulong_ref(loc(lexer, $1)); }
        | STRUCT IDENTIFIER {  $$ = new StructTypeRef(loc(lexer, $1), $2.image_); }
        | UNION IDENTIFIER { $$ = new UnionTypeRef(loc(lexer, $1), $2.image_); }
        | TYPENAME { $$ = new UserTypeRef(loc(lexer, $1), $1.image_); }
        ;

assign_op : PLUS_ASSIGN { $$ = "+"; }
        | MINUS_ASSIGN { $$ = "-"; }
        | MULTIPLY_ASSIGN { $$ = "*"; }
        | DIVIDE_ASSIGN { $$ = "/"; }
        | MOD_ASSIGN   { $$ = "%"; }
        | AND_ASSIGN { $$ = "&"; }
        | OR_ASSIGN { $$ = "|"; }
        | XOR_ASSIGN { $$ = "^"; }
        | LSHIFT_ASSIGN { $$ = "<<"; }
        | RSHIFT_ASSIGN { $$ = ">>"; }
        ;

expr : term '=' expr { $$ = new AssignNode($1, $3); ZERO($1); ZERO($3); }
    | term assign_op expr { $$ = new OpAssignNode($1, $2, $3); ZERO($1); ZERO($3); }
    | expr10 { assert($1->get_oref() == 1); $$ = $1; ZERO($1); }
    ;

expr10 : expr10 '?' expr ':' expr9 { $$ = new CondExprNode($1, $3, $5);
            ZERO($1); ZERO($3); ZERO($5);}
        | expr9 { $$ = $1; ZERO($1); }
        ;

expr9 : expr8 { $$ = $1; ZERO($1); }
        | expr9 OR_OR expr8 { $$ = new LogicalOrNode($1, $3); ZERO($1); ZERO($3); }
        ;

expr8 : expr7 { $$ = $1; ZERO($1); }
        | expr8 AND_AND expr7 { $$ = new LogicalAndNode($1, $3); ZERO($1); ZERO($3); }
        ;

expr7 : expr6 { $$ = $1; ZERO($1); }
        | expr7 '>' expr6 { $$ = new BinaryOpNode($1, ">", $3); ZERO($1); ZERO($3); }
        | expr7 '<' expr6 { $$ = new BinaryOpNode($1, "<", $3); ZERO($1); ZERO($3); }
        | expr7 GE expr6 { $$ = new BinaryOpNode($1, ">=", $3); ZERO($1); ZERO($3); }
        | expr7 LE expr6 { $$ = new BinaryOpNode($1, "<=", $3); ZERO($1); ZERO($3); }
        | expr7 EQ expr6 { $$ = new BinaryOpNode($1, "==", $3); ZERO($1); ZERO($3); }
        | expr7 NE expr6 { $$ = new BinaryOpNode($1, "!=", $3); ZERO($1); ZERO($3); }
        ;

expr6 : expr5 { $$ = $1; ZERO($1); }
        | expr6 '|' expr5 { $$ = new BinaryOpNode($1, "|", $3); ZERO($1); ZERO($3); }
        ;

expr5 : expr4 { $$ = $1; ZERO($1); }
        | expr5 '^' expr4 { $$ = new BinaryOpNode($1, "^", $3); ZERO($1); ZERO($3); }
        ;

expr4 : expr3 { $$ = $1; ZERO($1); }
        | expr4 '&' expr3 { $$ = new BinaryOpNode($1, "&", $3); ZERO($1); ZERO($3); }
        ;

expr3 : expr2 { $$ = $1; ZERO($1); }
        | expr3 RSHIFT expr2 { $$ = new BinaryOpNode($1, ">>", $3); ZERO($1); ZERO($3); }
        | expr3 LSHIFT expr2 { $$ = new BinaryOpNode($1, "<<", $3); ZERO($1); ZERO($3); }
        ;

expr2 : expr1 { $$ = $1; ZERO($1); }
        | expr2 '+' expr1 { $$ = new BinaryOpNode($1, "+", $3); ZERO($1); ZERO($3); }
        | expr2 '-' expr1 { $$ = new BinaryOpNode($1, "-", $3); ZERO($1); ZERO($3); }
        ;

expr1 : term { $$ = $1; ZERO($1); }
        | expr1 '*' term { $$ = new BinaryOpNode($1, "*", $3); ZERO($1); ZERO($3); }
        | expr1 '/' term { $$ = new BinaryOpNode($1, "/", $3); ZERO($1); ZERO($3); }
        | expr1 '%' term { $$ = new BinaryOpNode($1, "%", $3); ZERO($1); ZERO($3); }
        ;

term : '(' type ')' term { $$ = new CastNode($2, $4); ZERO($2); ZERO($4);}
        | unary { $$ = $1; ZERO($1); }
        ;

unary :   PLUS_PLUS unary { $$ = new PrefixOpNode("++", $2); ZERO($2); }
        | MINUS_MINUS unary { $$ = new PrefixOpNode("--", $2); ZERO($2); }
        | '+' term { $$ = new UnaryOpNode("+", $2); ZERO($2); }
        | '-' term { $$ = new UnaryOpNode("-", $2); ZERO($2); }
        | '!' term { $$ = new UnaryOpNode("!", $2); ZERO($2); }
        | '~' term { $$ = new UnaryOpNode("~", $2); ZERO($2); }
        | '*' term { $$ = new DereferenceNode($2); ZERO($2); }
        | '&' term { $$ = new AddressNode($2); ZERO($2); }
        | SIZEOF '(' type ')' { 
              $$ = new SizeofTypeNode($3, IntegerTypeRef::ulong_ref());
              ZERO($3); 
          }
        | SIZEOF unary { 
              $$ = new SizeofExprNode($2, IntegerTypeRef::ulong_ref());
              ZERO($2);
          }
        | postfix { $$ = $1; ZERO($1); }
        ;

postfix : primary { assert($1->get_oref() == 1); $$ = $1; ZERO($1); }
        | postfix PLUS_PLUS { $$ = new SuffixOpNode("++", $1); ZERO($1); }
        | postfix MINUS_MINUS { $$ = new SuffixOpNode("--", $1); ZERO($1); }
        | postfix '[' expr ']' { $$ = new ArefNode($1, $3); ZERO($1); ZERO($3); }
        | postfix '.' name { $$ = new MemberNode($1, $3); ZERO($1); }
        | postfix POINT_TO name { $$ = new PtrMemberNode($1, $3); }
        | postfix '(' ')' {
              auto v = vector<ExprNode*>{};
              $$ = new FuncallNode($1, move(v));
              ZERO($1);
          }
        | postfix '(' args ')' { 
              $$ = new FuncallNode($1, move($3)); 
          }
        ;

name : IDENTIFIER { $$ = $1.image_; }


args : expr { $$ = vector<ExprNode*> {$1};
              ZERO($1);
          }
        | args ',' expr {
              $1.push_back($3);
              $$ = move($1);
              ZERO($3);
          }
        ;

primary : INTEGER       { $$ = integer_node(loc(lexer, $1), $1.image_); }
        | CHARACTER     { char c = character_code($1.image_);
                          $$ = new IntegerLiteralNode(
                              loc(lexer, $1),
                              IntegerTypeRef::char_ref(), c);
                        }
        | STRING        { $$ = new StringLiteralNode(
                              loc(lexer, $1),
                              new PointerTypeRef(IntegerTypeRef::char_ref()),
                              $1.image_);
                        }
        | IDENTIFIER    { $$ = new VariableNode(loc(lexer, $1), $1.image_); }
        | '(' expr ')'  { $$ = $2; ZERO($2); }
        ;

%%

Location loc(yyscan_t lexer, const Token& token)
{
    Option* opt = get_option(lexer);
    return Location(opt->src_, token);
}

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
    printf("%s at %s:%d,%d\n", msg.c_str(),
        get_src_file(lexer).c_str(),
        loc.begin.line, loc.begin.column);
}

/* A simple hack to support multi starting point. */
int yylex(parser::Parser::semantic_type *yylval,
        parser::Parser::location_type *loc, yyscan_t lexer)
{
    auto* option = get_option(lexer);
    if (option->start_) {
        auto ret = option->start_;
        option->start_ = 0;
        return ret;
    }   
    return _yylex(yylval, loc, lexer);
}

Option* get_option(yyscan_t lexer)
{
    return (Option*)yyget_extra(lexer);
}

string get_src_file(yyscan_t lexer)
{
    return ((Option*)yyget_extra(lexer))->src_;
}

Loader& get_loader(yyscan_t lexer)
{
    return ((Option*)yyget_extra(lexer))->loader_;
}

set<string>& get_typename(yyscan_t lexer)
{
    return ((Option*)yyget_extra(lexer))->typename_;
}

void add_known_types(Declarations* decl, yyscan_t lexer)
{
    auto& s = get_typename(lexer);
    for (auto* type : decl->typedefs()) {
        s.insert(type->name());
    }
}
