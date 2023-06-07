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
    #include "entity.h"

    using namespace may;

    extern long integer_value(const string& image);

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

%token <Token> '{' '}'
%token <Token> VOID CHAR SHORT INT LONG
%token <Token> TYPEDEF STRUCT UNION ENUM
%token <Token> STATIC EXTERN
%token <Token> SIGNED UNSIGNED CONST
%token <Token> IF ELSE SWITCH CASE DEFAULT WHILE DO FOR RETURN BREAK CONTINUE GOTO
%token <Token> IMPORT SIZEOF
%token <Token> IDENTIFIER TYPENAME INTEGER CHARACTER STRING

%type <void*> compilation_unit import_stmts top_defs
%type <void*> import_stmt

%type <shared_ptr<DefinedFunction>> def_func 

%type <pv_defined_variable> def_var_list def_vars
%type <shared_ptr<StructNode>> def_struct
%type <shared_ptr<UnionNode>> def_union
%type <shared_ptr<Constant>> def_const
%type <shared_ptr<TypedefNode>> def_typedef

%type <shared_ptr<Params>> params fixed_params
%type <shared_ptr<Parameter>> param

%type <pv_stmt_node> stmts

%type <shared_ptr<StmtNode>> stmt
%type <shared_ptr<LabelNode>> label_stmt
%type <shared_ptr<IfNode>> if_stmt
%type <shared_ptr<DoWhileNode>> dowhile_stmt
%type <shared_ptr<WhileNode>> while_stmt
%type <shared_ptr<ForNode>> for_stmt
%type <shared_ptr<SwitchNode>> switch_stmt
%type <shared_ptr<GotoNode>> goto_stmt
%type <shared_ptr<ReturnNode>> return_stmt
%type <shared_ptr<ContinueNode>> continue_stmt
%type <shared_ptr<BreakNode>> break_stmt 

%type <pv_case_node> case_clauses
%type <shared_ptr<CaseNode>> case_clause
%type <shared_ptr<BlockNode>> case_body block

%type <shared_ptr<ParamTypeRefs>> param_typerefs
%type <shared_ptr<ParamTypeRefs>> fixed_param_typerefs
%type <shared_ptr<TypeRef>> typeref_base typeref
%type <pv_slot> slots member_list
%type <pv_expr_node> cases
%type <pv_expr_node> args
%type <shared_ptr<TypeNode>> type
%type <shared_ptr<ExprNode>> opt_expr term expr
%type <shared_ptr<ExprNode>> expr10 expr9 expr8 expr7 expr6 expr5 expr4 expr3 expr2 expr1
%type <shared_ptr<ExprNode>> postfix
%type <shared_ptr<ExprNode>> primary unary
%type <string> name assign_op

%start compilation_unit

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

def_func : typeref name '(' VOID ')' block {
              pv_parameter p = pv_parameter(new vector<shared_ptr<Parameter>>);

              auto params = shared_ptr<Params>(
                  new Params(Location($4), p));

              auto ref = shared_ptr<TypeRef>(new 
                  FunctionTypeRef($1, // return type
                    params->parameter_typerefs())); // typeref

              $$ = shared_ptr<DefinedFunction>(new 
                  DefinedFunction(false, // priv
                    shared_ptr<TypeNode>(new TypeNode(ref)), // type
                    $2, // name 
                    params, // params
                    $6)); // body
          }
        | STATIC typeref name '(' VOID ')' block {
              pv_parameter p = pv_parameter(new vector<shared_ptr<Parameter>>);

              auto params = shared_ptr<Params>(
                  new Params(Location($5), p));

              auto ref = shared_ptr<TypeRef>(
                  new FunctionTypeRef($2, params->parameter_typerefs()));

              $$ = shared_ptr<DefinedFunction>(new 
                    DefinedFunction(true, // priv
                      shared_ptr<TypeNode>(new TypeNode(ref)), 
                      $3, // name
                      params, // params
                      $7)); // boddy
          }
        | typeref name '(' params ')' block {
              auto ref = shared_ptr<TypeRef>(
                  new FunctionTypeRef($1, $4->parameter_typerefs()));

              $$ = shared_ptr<DefinedFunction>(new 
                  DefinedFunction(false, 
                    shared_ptr<TypeNode>(new TypeNode(ref)), 
                    $2, 
                    $4, 
                    $6));
          }
        | STATIC typeref name '(' params ')' block {
              auto ref = shared_ptr<TypeRef>(
                  new FunctionTypeRef($2, $5->parameter_typerefs()));

              $$ = shared_ptr<DefinedFunction>(new 
                  DefinedFunction(false, 
                    shared_ptr<TypeNode>(new TypeNode(ref)), 
                    $3, 
                    $5, 
                    $7));
          }
        ;

def_var_list : def_vars { $$ = $1; }
        | def_var_list def_vars {
              for (auto v : *$2) {
                  $1->push_back(v);
              }
              $$ = $1;
          }
        ;

def_vars : type name '=' expr ';' {
               auto v = new vector<shared_ptr<DefinedVariable>>;
               $$ = pv_defined_variable(v);
               $$->emplace_back(
                      new DefinedVariable(false, $1, $2, $4));
           }
        | type name ';' {
              auto v = new vector<shared_ptr<DefinedVariable>>;
              $$ = pv_defined_variable(v);
              $$->emplace_back(
                      new DefinedVariable(false, $1, $2, nullptr));
          }
        | def_vars ',' type name '=' expr ';' {
              $1->emplace_back(
                      new DefinedVariable(false, $3, $4, $6));
              $$ = $1;
          }
        | def_vars ',' type name ';' {
              $1->emplace_back(
                   new DefinedVariable(false, $3, $4, nullptr));
              $$ = $1;
          }
        | STATIC type name '=' expr ';' {
              auto v = new vector<shared_ptr<DefinedVariable>>;
              $$ = pv_defined_variable(v);
              $$->emplace_back(new DefinedVariable(true, $2, $3, $5));
          }
        | STATIC type name ';' {
              auto v = new vector<shared_ptr<DefinedVariable>>;
              $$ = pv_defined_variable(v);
              $$->emplace_back(new DefinedVariable(true, $2, $3, nullptr));
          }
        | def_vars ',' STATIC type name '=' expr ';' {
              $1->emplace_back(
                      new DefinedVariable(true, $4, $5, $7));
              $$ = $1;
          }
        | def_vars ',' STATIC type name ';' {
              $1->emplace_back(
                      new DefinedVariable(false, $4, $5, nullptr));
              $$ = $1;
          }
        ;

def_const : CONST type name '=' expr ';' {
                $$ = shared_ptr<Constant>(
                    new Constant($2, $3, $5));
            }


def_struct : STRUCT name member_list ';' {
                auto sp = shared_ptr<TypeRef>(new StructTypeRef($2));

                $$ = shared_ptr<StructNode>(
                    new StructNode(Location($1), sp, $2, $3));
            }

def_union : UNION name member_list ';' {
                auto sp = shared_ptr<TypeRef>(new UnionTypeRef($2));
                
                $$ = shared_ptr<UnionNode>(
                    new UnionNode(Location($1), sp, $2, $3));
            }

def_typedef : TYPEDEF typeref IDENTIFIER ';' {
                  $$ = shared_ptr<TypedefNode>(
                      new TypedefNode(Location($1), $2, $3.image_));
              }

params : fixed_params { $$ = $1; }
        | fixed_params ',' "..." { 
              $$->accept_varargs();
          }
        ;

fixed_params : param {
              auto v = new vector<shared_ptr<Parameter>>{$1};
              auto sp = pv_parameter(v);
              $$ = shared_ptr<Params>(new Params($1->location(), sp));
          }
        | fixed_params ',' param  {
              $1->parameters()->push_back($3);
              $$ = $1;
          }
        ;

param : type name { 
              $$ = shared_ptr<Parameter>(new Parameter($1, $2)); 
          }
        ;

block : '{' '}' { 
              auto v = new vector<shared_ptr<DefinedVariable>>;
              auto sp = pv_defined_variable(v);
              auto v2 = new vector<shared_ptr<StmtNode>>;
              auto sp2 = pv_stmt_node(v2);
              $$ = shared_ptr<BlockNode>(new BlockNode(Location($1), sp, sp2));
          }
        | '{' stmts '}' {
                auto v = new vector<shared_ptr<DefinedVariable>>;
                auto sp = pv_defined_variable(v);

                $$ = shared_ptr<BlockNode>(
                    new BlockNode(Location($1), sp, $2));
           }
        | '{' def_var_list '}' {
                auto v = new vector<shared_ptr<StmtNode>>;
                auto sp = pv_stmt_node(v);

                $$ = shared_ptr<BlockNode>(
                    new BlockNode(Location($1), $2, sp));
            }
        | '{' def_var_list stmts '}' {
                $$ = shared_ptr<BlockNode>(
                    new BlockNode(Location($1), $2, $3));
           }
        ;

type : typeref { 
              $$ = shared_ptr<TypeNode>(new TypeNode($1)); 
          }
        ;

typeref : typeref_base  { $$ = $1; }
        | typeref_base '[' ']' { 
              $$ = shared_ptr<TypeRef>(new ArrayTypeRef($1)); 
          }
        | typeref_base '[' INTEGER ']' {
              $$ = shared_ptr<TypeRef>(
                  new ArrayTypeRef($1, integer_value($3.image_)));
          }
        | typeref_base '*' { 
              $$ = shared_ptr<TypeRef>(new PointerTypeRef($1)); 
          }
        | typeref_base '(' VOID ')' {
              auto v = new vector<shared_ptr<TypeRef>>{};
              auto sp = pv_typeref(v);
              auto param = shared_ptr<ParamTypeRefs>(new ParamTypeRefs(sp));
              $$ = shared_ptr<TypeRef>(new FunctionTypeRef($1, param));
          }
        | typeref_base '(' param_typerefs ')' {
              $$ = shared_ptr<TypeRef>(
                new FunctionTypeRef($1, $3));
          }
        | typeref '[' ']' { $$ = shared_ptr<TypeRef>(
                new ArrayTypeRef($1)); 
           }
        | typeref '[' INTEGER ']' {
              $$ = shared_ptr<TypeRef>(
                   new ArrayTypeRef($1, integer_value($3.image_))
              );
          }
        | typeref '*' { 
              $$ = shared_ptr<TypeRef>(
                new PointerTypeRef($1)); 
          }
        | typeref '(' VOID ')' {
              auto v = new vector<shared_ptr<TypeRef>>{};
              auto sp = pv_typeref(v);
              auto param = shared_ptr<ParamTypeRefs>(new ParamTypeRefs(sp));
             
              $$ = shared_ptr<TypeRef>(new FunctionTypeRef($1, param));
          }
        | typeref '(' param_typerefs ')' { 
                $$ = shared_ptr<TypeRef>(new FunctionTypeRef($1, $3)); 
          }
        ;

param_typerefs: fixed_param_typerefs { $$ = $1; }
        | fixed_param_typerefs ',' "..." {
              $$ = $1;
              $$->accept_varargs();
          }
        ;

fixed_param_typerefs : typeref {
              auto v = new vector<shared_ptr<TypeRef>>{$1};
              auto sp = pv_typeref(v);

              $$ = shared_ptr<ParamTypeRefs>(new ParamTypeRefs(sp));
          }
        | fixed_param_typerefs ',' typeref {
              $$->typerefs()->push_back($3);
          }
        ;

stmt : ';' { $$ = nullptr; }
        | label_stmt     { $$ = $1; }
        | expr ';'       { $$ = shared_ptr<ExprStmtNode>(new ExprStmtNode($1->location(), $1)); }
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

label_stmt : IDENTIFIER ':' stmt {
                 $$ = shared_ptr<LabelNode>(
                        new LabelNode(Location($1), $1.image_, $3));
             }
        ;

if_stmt : IF '(' expr ')' stmt ELSE stmt {
              $$ = shared_ptr<IfNode>(
                new IfNode(Location($1), $3, $5, $7));
          }
        | IF '(' expr ')' stmt {
              $$ = shared_ptr<IfNode>(
                new IfNode(Location($1), $3, $5));
          }
        ;

while_stmt : WHILE '(' expr ')' stmt {
                 $$ = shared_ptr<WhileNode>(
                        new WhileNode(Location($1), $3, $5));
             }

dowhile_stmt : DO stmt WHILE '(' expr ')' ';' {
                   $$ = shared_ptr<DoWhileNode>(
                        new DoWhileNode(Location($1), $2, $5));
               }

for_stmt : FOR '(' opt_expr ';' opt_expr ';' opt_expr ')' stmt
           {
               $$ = shared_ptr<ForNode>(
                new ForNode(Location($1), $3, $5, $7, $9));
           }

goto_stmt : GOTO IDENTIFIER ';' {
                $$ = shared_ptr<GotoNode>(new GotoNode(Location($1), $2.image_));
            }

switch_stmt : SWITCH '(' expr ')' '{' case_clauses '}'
              {
                  $$ = shared_ptr<SwitchNode>(
                        new SwitchNode(Location($1), $3, $6));
              }

case_clauses : case_clause { 
               auto v = new vector<shared_ptr<CaseNode>>{$1};
               $$ = pv_case_node(v);
            }
        | case_clauses case_clause {
              $1->push_back($2);
              $$ = $1;
          }
        ;

case_clause : cases case_body {
                  $$ = shared_ptr<CaseNode>(
                      new CaseNode($2->location(), $1, $2 /* BlockNode */
                  ));
              }
        ;

/* need to check invalid cases */
cases : CASE primary ':' {
            auto v = new vector<shared_ptr<ExprNode>>;
            $$ = pv_expr_node(v);
            $$->push_back($2); 
        }
        | DEFAULT ':' {
            auto v = new vector<shared_ptr<ExprNode>>;
            $$ = pv_expr_node(v);
            $$->push_back(nullptr); 
          }
        | cases CASE primary ':' {
              $1->push_back($3);
              $$ = $1;
          }
        | cases DEFAULT primary ':' {
              $1->push_back(nullptr);
              $$ = $1;
          }
        ;

case_body : stmts {
                /* don't need to check break, C-Language switch
                 * statement and have no breaks. */

                auto v = new vector<shared_ptr<DefinedVariable>>;

                $$ = shared_ptr<BlockNode>(
                    new BlockNode((*$1)[0]->location(), 
                         pv_defined_variable(v), $1)
                );
            }

return_stmt : RETURN ';'  { 
            $$ = shared_ptr<ReturnNode>(new ReturnNode(Location($1), nullptr)); 
        }
        | RETURN expr ';' { 
            $$ = shared_ptr<ReturnNode>(new ReturnNode(Location($1), $2)); 
        }
        ;

continue_stmt : CONTINUE ';' { 
               $$ = shared_ptr<ContinueNode>(new ContinueNode(Location($1))); 
           }

break_stmt : BREAK ';' { 
               $$ = shared_ptr<BreakNode>(new BreakNode(Location($1))); 
           }

stmts : stmt { 
            auto v = new vector<shared_ptr<StmtNode>>;
            $$ = pv_stmt_node(v);
            $$->push_back($1); 
        }
        | stmts stmt {
              if ($2) {
                  $1->push_back($2);
              }
              $$ = $1;
          };
        ;

member_list : '{' '}'   { 
               auto v = new vector<shared_ptr<Slot>>; 
               $$ = pv_slot(v);
           }
        | '{' slots '}' { $$ = $2; }
        ;

slots : type name ';' {
                auto v = new vector<shared_ptr<Slot>>;
                $$ = pv_slot(v);
                $$->emplace_back(new Slot($1, $2));
            }
        | slots type name ';' {
                $1->emplace_back(new Slot($2, $3));
                $$ = $1;
            }
        ;

opt_expr : %empty { $$ = nullptr; }
        | expr { $$ = $1; }
        ;

typeref_base : VOID { 
              $$ = shared_ptr<TypeRef>(new VoidTypeRef(Location($1))); 
          }
        | CHAR { 
              $$ = shared_ptr<TypeRef>(IntegerTypeRef::char_ref(Location($1))); 
          }
        | SHORT { 
              $$ = shared_ptr<TypeRef>(IntegerTypeRef::short_ref(Location($1))); 
          }
        | INT { 
              $$ = shared_ptr<TypeRef>(IntegerTypeRef::int_ref(Location($1))); 
          }
        | LONG { 
              $$ = shared_ptr<TypeRef>(IntegerTypeRef::long_ref(Location($1))); 
          }
        | UNSIGNED CHAR { 
              $$ = shared_ptr<TypeRef>(IntegerTypeRef::uchar_ref(Location($1))); 
          }
        | UNSIGNED SHORT { 
              $$ = shared_ptr<TypeRef>(IntegerTypeRef::ushort_ref(Location($1)));
          }
        | UNSIGNED INT { 
              $$ = shared_ptr<TypeRef>(IntegerTypeRef::uint_ref(Location($1))); 
          }
        | UNSIGNED LONG { 
              $$ = shared_ptr<TypeRef>(IntegerTypeRef::ulong_ref(Location($1))); 
          }
        | STRUCT IDENTIFIER { 
              $$ = shared_ptr<TypeRef>(
                new StructTypeRef(Location($1), $2.image_)); 
          }
        | UNION IDENTIFIER { 
              $$ = shared_ptr<TypeRef>(
                new UnionTypeRef(Location($1), $2.image_)); 
          }
        | TYPENAME { 
              $$ = shared_ptr<TypeRef>(
                new UserTypeRef(Location($1), $1.image_)); 
          }
        ;

expr : term '=' expr { 
        $$ = shared_ptr<ExprNode>(
            new AssignNode($1, $3)); 
        }
    | term assign_op expr { 
        $$ = shared_ptr<ExprNode>(
            new OpAssignNode($1, $2, $3)); 
      }
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

expr10 : expr10 '?' expr ':' expr9 { 
              $$ = shared_ptr<ExprNode>(new CondExprNode($1, $3, $5)); 
          }
        | expr9 { $$ = $1; }
        ;

expr9 : expr8 { $$ = $1; }
        | expr9 "||" expr8 { 
              $$ = shared_ptr<ExprNode>(new LogicalOrNode($1, $3)); 
          }
        ;

expr8 : expr7 { $$ = $1; }
        | expr8 "&&" expr7 { 
              $$ = shared_ptr<ExprNode>(new LogicalAndNode($1, $3)); 
          }
        ;

expr7 : expr6 { $$ = $1; }
        | expr7 '>' expr6 { 
              $$ = shared_ptr<ExprNode>(new BinaryOpNode($1, ">", $3)); 
          }
        | expr7 '<' expr6 { 
              $$ = shared_ptr<ExprNode>(new BinaryOpNode($1, "<", $3)); 
          }
        | expr7 ">=" expr6 { 
              $$ = shared_ptr<ExprNode>(new BinaryOpNode($1, ">=", $3)); 
          }
        | expr7 "<=" expr6 { 
              $$ = shared_ptr<ExprNode>(new BinaryOpNode($1, "<=", $3)); 
          }
        | expr7 "==" expr6 { 
              $$ = shared_ptr<ExprNode>(new BinaryOpNode($1, "==", $3)); 
          }
        | expr7 "!=" expr6 { 
              $$ = shared_ptr<ExprNode>(new BinaryOpNode($1, "!=", $3)); 
          }
        ;

expr6 : expr5 { $$ = $1; }
        | expr6 '|' expr5 { 
              $$ = shared_ptr<ExprNode>(new BinaryOpNode($1, "|", $3)); 
          }
        ;

expr5 : expr4 { $$ = $1; }
        | expr5 '^' expr4 { 
              $$ = shared_ptr<ExprNode>(new BinaryOpNode($1, "^", $3)); 
          }
        ;

expr4 : expr3 { $$ = $1; }
        | expr4 '&' expr3 { 
              $$ = shared_ptr<ExprNode>(new BinaryOpNode($1, "&", $3)); 
          }
        ;

expr3 : expr2 { $$ = $1; }
        | expr3 ">>" expr2 { 
              $$ = shared_ptr<ExprNode>(new BinaryOpNode($1, ">>", $3)); 
          }
        | expr3 "<<" expr2 { 
              $$ = shared_ptr<ExprNode>(new BinaryOpNode($1, "<<", $3)); 
          }
        ;

expr2 : expr1 { $$ = $1; }
        | expr2 '+' expr1 { 
              $$ = shared_ptr<ExprNode>(new BinaryOpNode($1, "+", $3)); 
          }
        | expr2 '-' expr1 { 
              $$ = shared_ptr<ExprNode>(new BinaryOpNode($1, "-", $3)); 
          }
        ;

expr1 : term { $$ = $1; }
        | expr1 '*' term { 
              $$ = shared_ptr<ExprNode>(new BinaryOpNode($1, "*", $3)); 
          }
        | expr1 '/' term { 
              $$ = shared_ptr<ExprNode>(new BinaryOpNode($1, "/", $3)); 
          }
        | expr1 '%' term { 
              $$ = shared_ptr<ExprNode>(new BinaryOpNode($1, "%", $3)); 
          }
        ;

term : '(' type ')' term { 
              $$ = shared_ptr<ExprNode>(new CastNode($2, $4)); 
          }
        | unary { $$ = $1; }
        ;

unary :   "++" unary { 
              $$ = shared_ptr<ExprNode>(new PrefixOpNode("++", $2)); 
          }
        | "--" unary {
              $$ = shared_ptr<ExprNode>(new PrefixOpNode("--", $2)); 
          }
        | '+' term { 
              $$ = shared_ptr<ExprNode>(new UnaryOpNode("+", $2)); 
          }
        | '-' term { 
              $$ = shared_ptr<ExprNode>(new UnaryOpNode("-", $2)); 
          }
        | '!' term { 
              $$ = shared_ptr<ExprNode>(new UnaryOpNode("!", $2)); 
          }
        | '~' term { 
              $$ = shared_ptr<ExprNode>(new UnaryOpNode("~", $2)); 
          }
        | '*' term { 
              $$ = shared_ptr<ExprNode>(new DereferenceNode($2));
          }
        | '&' term { 
              $$ = shared_ptr<ExprNode>(new AddressNode($2)); 
          }
        | SIZEOF '(' type ')' { 
              $$ = shared_ptr<ExprNode>(
                   new SizeofTypeNode($3, shared_ptr<TypeRef>(IntegerTypeRef::ulong_ref()))
              ); 
          }
        | SIZEOF unary { 
              $$ = shared_ptr<ExprNode>(
                   new SizeofExprNode($2, shared_ptr<TypeRef>(IntegerTypeRef::ulong_ref()))
              ); 
          }
        | postfix { $$ = $1; }
        ;

postfix : primary { $$ = $1; }
        | postfix "++" { 
              $$ = shared_ptr<ExprNode>(new SuffixOpNode("++", $1)); 
          }
        | postfix "--" {
              $$ = shared_ptr<ExprNode>(new SuffixOpNode("--", $1)); 
          }
        | postfix '[' expr ']' { 
              $$ = shared_ptr<ExprNode>(new ArefNode($1, $3)); 
          }
        | postfix '.' name { 
              $$ = shared_ptr<ExprNode>(new MemberNode($1, $3)); 
          }
        | postfix "->" name { 
              $$ = shared_ptr<ExprNode>(new PtrMemberNode($1, $3)); 
          }
        | postfix '(' ')' {
             auto v = new vector<shared_ptr<ExprNode>>;
             $$ = shared_ptr<ExprNode>(
                new FuncallNode($1, pv_expr_node(v))
             ); 
          }
        | postfix '(' args ')' { 
             $$ = shared_ptr<ExprNode>(
                 new FuncallNode($1, $3)
             ); 
          }
        ;

name : IDENTIFIER {  $$ = $1.image_; }


args : expr { auto v = new vector<shared_ptr<ExprNode>> {$1}; 
              $$ = pv_expr_node(v);
            }
        | args ',' expr { 
              $1->push_back($3);
              $$ = $1; 
          }
        ;

primary : INTEGER       { $$ = shared_ptr<ExprNode>(
                              integer_node(Location($1), $1.image_)
                          ); 
                        }
        | CHARACTER     { $$ = shared_ptr<ExprNode>(new IntegerLiteralNode(
                              Location($1),
                              shared_ptr<TypeRef>(IntegerTypeRef::char_ref()), 
                              $1.image_[0])
                          );
                        }
        | STRING        { $$ = shared_ptr<ExprNode>(new StringLiteralNode(
                              Location($1),
                              shared_ptr<TypeRef>(new PointerTypeRef(IntegerTypeRef::char_ref())),
                              $1.image_)
                          );
                        }
        | IDENTIFIER    { $$ = shared_ptr<ExprNode>(
                              new VariableNode(Location($1), $1.image_)
                          );
                        }
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
