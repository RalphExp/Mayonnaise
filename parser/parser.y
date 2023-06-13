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

    using namespace cbc;

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

%type <DefinedFunction*> def_func 

%type <vector<DefinedVariable*>> def_var_list def_vars
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
              auto v = vector<Parameter*>{};

              auto params = new Params(Location($4), move(v));

              auto ref = new FunctionTypeRef($1, // return type
                    move(params->parameter_typerefs())); // typeref

              $$ = new DefinedFunction(false, // priv
                    new TypeNode(ref), // type
                    $2, // name 
                    params, // params
                    $6); // body
          }
        | STATIC typeref name '(' VOID ')' block {
              auto v = vector<Parameter*>{};

              auto params = new Params(Location($5), move(v));

              auto ref = new FunctionTypeRef(
                  $2, move(params->parameter_typerefs()));

              $$ = new DefinedFunction(true, // priv
                      new TypeNode(ref), 
                      $3, // name
                      params, // params
                      $7); // boddy
          }
        | typeref name '(' params ')' block {
              auto ref = new FunctionTypeRef($1, move($4->parameter_typerefs()));

              $$ = new DefinedFunction(false, 
                    new TypeNode(ref), 
                    $2, 
                    $4, 
                    $6);
          }
        | STATIC typeref name '(' params ')' block {
              auto ref = new FunctionTypeRef($2, move($5->parameter_typerefs()));

              $$ = new DefinedFunction(false, 
                    new TypeNode(ref), 
                    $3, 
                    $5, 
                    $7);
          }
        ;

def_var_list : def_vars { $$ = $1; }
        | def_var_list def_vars {
              for (auto v : $2) {
                  v->inc_ref();
                  $1.push_back(v);
              }
              $$ = move($1);
          }
        ;

def_vars : type name '=' expr ';' {
              auto p = new DefinedVariable(false, $1, $2, $4);
              p->inc_ref();
              $$ = vector<DefinedVariable*>{p};
           }
        | type name ';' {
              auto p = new DefinedVariable(false, $1, $2, nullptr);
              p->inc_ref();
              $$ = vector<DefinedVariable*>{p};
          }
        | def_vars ',' type name '=' expr ';' {
              auto p = new DefinedVariable(false, $3, $4, $6);
              p->inc_ref();
              $1.push_back(p);
              $$ = move($1);
          }
        | def_vars ',' type name ';' {
              auto p = new DefinedVariable(false, $3, $4, nullptr);
              p->inc_ref();
              $1.push_back(p);
              $$ = move($1);
          }
        | STATIC type name '=' expr ';' {
              auto p = new DefinedVariable(true, $2, $3, $5);
              p->inc_ref();
              $$ = vector<DefinedVariable*>{p};
          }
        | STATIC type name ';' {
              auto p = new DefinedVariable(true, $2, $3, nullptr);
              p->inc_ref();
              $$ = vector<DefinedVariable*>{p};
          }
        | def_vars ',' STATIC type name '=' expr ';' {
              auto p = new DefinedVariable(true, $4, $5, $7);
              p->inc_ref();
              $1.push_back(p);
              $$ = move($1);
          }
        | def_vars ',' STATIC type name ';' {
              auto p = new DefinedVariable(false, $4, $5, nullptr);
              p->inc_ref();
              $1.push_back(p);
              $$ = move($1);
          }
        ;

def_const : CONST type name '=' expr ';' {
                $$ = new Constant($2, $3, $5);
            }


def_struct : STRUCT name member_list ';' {
                auto p = new StructTypeRef($2);
                $$ = new StructNode(Location($1), p, $2, move($3));
            }

def_union : UNION name member_list ';' {
                auto p = new UnionTypeRef($2);          
                $$ = new UnionNode(Location($1), p, $2, move($3));
            }

def_typedef : TYPEDEF typeref IDENTIFIER ';' {
                  $$ = new TypedefNode(Location($1), $2, $3.image_);
              }

params : fixed_params { $$ = $1; }
        | fixed_params ',' "..." { 
              $$->accept_varargs();
          }
        ;

fixed_params : param {
              $1->inc_ref();
              auto v = vector<Parameter*>{$1};
              $$ = new Params($1->location(), move(v));
          }
        | fixed_params ',' param  {
              $3->inc_ref();
              $1->param_descs_.push_back($3);
              $$ = $1;
          }
        ;

param : type name { 
              $$ = new Parameter($1, $2); 
          }
        ;

block : '{' '}' { 
              auto v = vector<DefinedVariable*>{};
              auto v2 = vector<StmtNode*>{};
              $$ = new BlockNode(Location($1), move(v), move(v2));
          }
        | '{' stmts '}' {
                auto v = vector<DefinedVariable*>{};
                $$ = new BlockNode(Location($1), move(v), move($2));
           }
        | '{' def_var_list '}' {
                auto v = vector<StmtNode*>{};
                $$ = new BlockNode(Location($1), move($2), move(v));
            }
        | '{' def_var_list stmts '}' {
                $$ = new BlockNode(Location($1), move($2), move($3));
           }
        ;

type : typeref { 
              $$ = new TypeNode($1); 
          }
        ;

typeref : typeref_base  { $$ = $1; }
        | typeref_base '[' ']' { 
              $$ = new ArrayTypeRef($1); 
          }
        | typeref_base '[' INTEGER ']' {
              $$ = new ArrayTypeRef($1, integer_value($3.image_));
          }
        | typeref_base '*' { 
              $$ = new PointerTypeRef($1); 
          }
        | typeref_base '(' VOID ')' {
              auto v = vector<TypeRef*>{};;
              auto param = new ParamTypeRefs(move(v));
              $$ = new FunctionTypeRef($1, param);
          }
        | typeref_base '(' param_typerefs ')' {
              $$ = new FunctionTypeRef($1, $3);
          }
        | typeref '[' ']' { new ArrayTypeRef($1); 
           }
        | typeref '[' INTEGER ']' {
              $$ = new ArrayTypeRef($1, integer_value($3.image_));
          }
        | typeref '*' { 
              $$ = new PointerTypeRef($1); 
          }
        | typeref '(' VOID ')' {
              auto v = vector<TypeRef*>{};
              auto param = new ParamTypeRefs(move(v));
              $$ = new FunctionTypeRef($1, param);
          }
        | typeref '(' param_typerefs ')' { 
                $$ = new FunctionTypeRef($1, $3); 
          }
        ;

param_typerefs: fixed_param_typerefs { $$ = $1; }
        | fixed_param_typerefs ',' "..." {
              $$ = $1;
              $$->accept_varargs();
          }
        ;

fixed_param_typerefs : typeref {
              $1->inc_ref();
              auto v = vector<TypeRef*>{$1};
              $$ = new ParamTypeRefs(move(v));
          }
        | fixed_param_typerefs ',' typeref {
              $3->inc_ref();
              $$->param_descs_.push_back($3);
          }
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

label_stmt : IDENTIFIER ':' stmt {
                 $$ = new LabelNode(Location($1), $1.image_, $3);
             }
        ;

if_stmt : IF '(' expr ')' stmt ELSE stmt {
              $$ = new IfNode(Location($1), $3, $5, $7);
          }
        | IF '(' expr ')' stmt {
              $$ = new IfNode(Location($1), $3, $5);
          }
        ;

while_stmt : WHILE '(' expr ')' stmt {
                 $$ = new WhileNode(Location($1), $3, $5);
             }

dowhile_stmt : DO stmt WHILE '(' expr ')' ';' {
                   $$ = new DoWhileNode(Location($1), $2, $5);
               }

for_stmt : FOR '(' opt_expr ';' opt_expr ';' opt_expr ')' stmt
           {
               $$ = new ForNode(Location($1), $3, $5, $7, $9);
           }

goto_stmt : GOTO IDENTIFIER ';' {
                $$ = new GotoNode(Location($1), $2.image_);
            }

switch_stmt : SWITCH '(' expr ')' '{' case_clauses '}'
              {
                  $$ = new SwitchNode(Location($1), $3, move($6));
              }

case_clauses : case_clause {
               $1->inc_ref();
               $$ = vector<CaseNode*>{$1};
            }
        | case_clauses case_clause {
              $2->inc_ref();
              $1.push_back($2);
              $$ = move($1);
          }
        ;

case_clause : cases case_body {
                  $$ = new CaseNode($2->location(), move($1), $2 /* BlockNode */ );
              }
        ;

/* need to check invalid cases */
cases : CASE primary ':' {
            $2->inc_ref();
            $$ = vector<ExprNode*>{};
            $$.push_back($2); 
        }
        | DEFAULT ':' {
            $$ = vector<ExprNode*>{};
            $$.push_back(nullptr); 
          }
        | cases CASE primary ':' {
              $3->inc_ref();
              $1.push_back($3);
              $$ = move($1);
          }
        | cases DEFAULT primary ':' {
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

return_stmt : RETURN ';'  { 
            $$ = new ReturnNode(Location($1), nullptr); 
        }
        | RETURN expr ';' { 
            $$ = new ReturnNode(Location($1), $2); 
        }
        ;

continue_stmt : CONTINUE ';' { 
               $$ = new ContinueNode(Location($1)); 
           }

break_stmt : BREAK ';' { 
               $$ = new BreakNode(Location($1)); 
           }

stmts : stmt {
            $1->inc_ref();
            $$ = vector<StmtNode*>{};
            $$.push_back($1); 
        }
        | stmts stmt {
              if ($2) {
                  $2->inc_ref();
                  $1.push_back($2);
              }
              $$ = move($1);
          };
        ;

member_list : '{' '}'   { 
               $$ = vector<Slot*>{};
           }
        | '{' slots '}' { $$ = move($2); }
        ;

slots : type name ';' {
                $$ = vector<Slot*>{};
                auto s = new Slot($1, $2);
                s->inc_ref();
                $$.emplace_back(s);
            }
        | slots type name ';' {
                auto s = new Slot($2, $3);
                s->inc_ref();
                $1.emplace_back(s);
                $$ = move($1);
            }
        ;

opt_expr : %empty { $$ = nullptr; }
        | expr { $$ = $1; }
        ;

typeref_base : VOID { 
              $$ = new VoidTypeRef(Location($1)); 
          }
        | CHAR { 
              $$ = IntegerTypeRef::char_ref(Location($1)); 
          }
        | SHORT { 
              $$ = IntegerTypeRef::short_ref(Location($1)); 
          }
        | INT { 
              $$ = IntegerTypeRef::int_ref(Location($1)); 
          }
        | LONG { 
              $$ = IntegerTypeRef::long_ref(Location($1)); 
          }
        | UNSIGNED CHAR { 
              $$ = IntegerTypeRef::uchar_ref(Location($1)); 
          }
        | UNSIGNED SHORT { 
              $$ = IntegerTypeRef::ushort_ref(Location($1));
          }
        | UNSIGNED INT { 
              $$ = IntegerTypeRef::uint_ref(Location($1)); 
          }
        | UNSIGNED LONG { 
              $$ = IntegerTypeRef::ulong_ref(Location($1)); 
          }
        | STRUCT IDENTIFIER { 
              $$ = new StructTypeRef(Location($1), $2.image_); 
          }
        | UNION IDENTIFIER { 
              $$ = new UnionTypeRef(Location($1), $2.image_); 
          }
        | TYPENAME { 
              $$ = new UserTypeRef(Location($1), $1.image_); 
          }
        ;

expr : term '=' expr { 
            $$ = new AssignNode($1, $3); 
        }
    | term assign_op expr { 
          $$ = new OpAssignNode($1, $2, $3); 
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
              $$ = new CondExprNode($1, $3, $5); 
          }
        | expr9 { $$ = $1; }
        ;

expr9 : expr8 { $$ = $1; }
        | expr9 "||" expr8 { 
              $$ = new LogicalOrNode($1, $3); 
          }
        ;

expr8 : expr7 { $$ = $1; }
        | expr8 "&&" expr7 { 
              $$ = new LogicalAndNode($1, $3); 
          }
        ;

expr7 : expr6 { $$ = $1; }
        | expr7 '>' expr6 { 
              $$ = new BinaryOpNode($1, ">", $3); 
          }
        | expr7 '<' expr6 { 
              $$ = new BinaryOpNode($1, "<", $3); 
          }
        | expr7 ">=" expr6 { 
              $$ = new BinaryOpNode($1, ">=", $3); 
          }
        | expr7 "<=" expr6 { 
              $$ = new BinaryOpNode($1, "<=", $3); 
          }
        | expr7 "==" expr6 { 
              $$ = new BinaryOpNode($1, "==", $3); 
          }
        | expr7 "!=" expr6 { 
              $$ = new BinaryOpNode($1, "!=", $3); 
          }
        ;

expr6 : expr5 { $$ = $1; }
        | expr6 '|' expr5 { 
              $$ = new BinaryOpNode($1, "|", $3); 
          }
        ;

expr5 : expr4 { $$ = $1; }
        | expr5 '^' expr4 { 
              $$ = new BinaryOpNode($1, "^", $3); 
          }
        ;

expr4 : expr3 { $$ = $1; }
        | expr4 '&' expr3 { 
              $$ = new BinaryOpNode($1, "&", $3); 
          }
        ;

expr3 : expr2 { $$ = $1; }
        | expr3 ">>" expr2 { 
              $$ = new BinaryOpNode($1, ">>", $3); 
          }
        | expr3 "<<" expr2 { 
              $$ = new BinaryOpNode($1, "<<", $3); 
          }
        ;

expr2 : expr1 { $$ = $1; }
        | expr2 '+' expr1 { 
              $$ = new BinaryOpNode($1, "+", $3); 
          }
        | expr2 '-' expr1 { 
              $$ = new BinaryOpNode($1, "-", $3); 
          }
        ;

expr1 : term { $$ = $1; }
        | expr1 '*' term { 
              $$ = new BinaryOpNode($1, "*", $3); 
          }
        | expr1 '/' term { 
              $$ = new BinaryOpNode($1, "/", $3); 
          }
        | expr1 '%' term { 
              $$ = new BinaryOpNode($1, "%", $3); 
          }
        ;

term : '(' type ')' term { 
              $$ = new CastNode($2, $4); 
          }
        | unary { $$ = $1; }
        ;

unary :   "++" unary { 
              $$ = new PrefixOpNode("++", $2); 
          }
        | "--" unary {
              $$ = new PrefixOpNode("--", $2); 
          }
        | '+' term { 
              $$ = new UnaryOpNode("+", $2); 
          }
        | '-' term { 
              $$ = new UnaryOpNode("-", $2); 
          }
        | '!' term { 
              $$ = new UnaryOpNode("!", $2); 
          }
        | '~' term { 
              $$ = new UnaryOpNode("~", $2); 
          }
        | '*' term { 
              $$ = new DereferenceNode($2);
          }
        | '&' term { 
              $$ = new AddressNode($2); 
          }
        | SIZEOF '(' type ')' { 
              $$ = new SizeofTypeNode($3, IntegerTypeRef::ulong_ref()); 
          }
        | SIZEOF unary { 
              $$ = new SizeofExprNode($2, IntegerTypeRef::ulong_ref()); 
          }
        | postfix { $$ = $1; }
        ;

postfix : primary { $$ = $1; }
        | postfix "++" { 
              $$ = new SuffixOpNode("++", $1); 
          }
        | postfix "--" {
              $$ = new SuffixOpNode("--", $1); 
          }
        | postfix '[' expr ']' { 
              $$ = new ArefNode($1, $3); 
          }
        | postfix '.' name { 
              $$ = new MemberNode($1, $3); 
          }
        | postfix "->" name { 
              $$ = new PtrMemberNode($1, $3); 
          }
        | postfix '(' ')' {
             auto v = vector<ExprNode*>{};
             $$ = new FuncallNode($1, move(v));
          }
        | postfix '(' args ')' { 
             $$ = new FuncallNode($1, move($3)); 
          }
        ;

name : IDENTIFIER {  $$ = $1.image_; }


args : expr {
              $1->inc_ref(); 
              $$ = vector<ExprNode*> {$1}; 
          }
        | args ',' expr {
              $3->inc_ref();
              $1.push_back($3);
              $$ = move($1); 
          }
        ;

primary : INTEGER       { $$ = integer_node(Location($1), $1.image_); }
        | CHARACTER     { $$ = new IntegerLiteralNode(
                              Location($1),
                              IntegerTypeRef::char_ref(), 
                              $1.image_[0]);
                        }
        | STRING        { $$ = new StringLiteralNode(
                              Location($1),
                              new PointerTypeRef(IntegerTypeRef::char_ref()),
                              $1.image_);
                        }
        | IDENTIFIER    { $$ = new VariableNode(Location($1), $1.image_); }
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
