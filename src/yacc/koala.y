/*
 * Copyright (c) 2018 James, https://github.com/zhuguangxiang
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

%{

#include "parser.h"

#define yyloc_row(loc) ((loc).first_line)
#define yyloc_col(loc) ((loc).first_column)

#define DeclareIdent(name, s, loc) \
  Ident name = {(s).str, {yyloc_row(loc), yyloc_col(loc)}}

#define DeclareType(name, type, loc) \
  TypeWrapper name = {type, {yyloc_row(loc), yyloc_col(loc)}}

#define DeclarePosition(name, loc) \
  Position name = {yyloc_row(loc), yyloc_col(loc)}

#define SetPosition(pos, loc) \
({                            \
  (pos).row = yyloc_row(loc); \
  (pos).col = yyloc_col(loc); \
})

#ifndef NDEBUG
#define YYERROR_VERBOSE 1
static int yyerror(void *loc, ParserState *ps, void *scanner, const char *msg)
{
  UNUSED_PARAMETER(scanner);
  YYLTYPE *yyloc = loc;
  fprintf(stderr, ">>>>%s:%d:%d: %s\n", ps->filename,
          yyloc->first_line, yyloc->first_column, msg);
  return 0;
}
#else
#define yyerror(loc, ps, scanner, errmsg) ((void)0)
#endif

#define EMPTY_FILE \
  fprintf(stderr, "%s: \x1b[33mwarn:\x1b[0m empty source file\n", ps->filename);

#define PKG_NAME_LEN_OVER_MAX_MSG \
  "package name length is too larger(%d >= MAXIMUM(%d))."

#define ERRMSG "expected '%s', but found '%s'\n"

#define YYSyntax_Error(loc, expected) \
({                                    \
  yyerrok;                            \
  DeclarePosition(pos, loc);          \
  char *token = ps->line.token;       \
  if (token[0] == '\n') {             \
    token = "<newline>";              \
    pos = ps->line.lastpos;           \
  }                                   \
  Syntax_Error(ps, &pos, ERRMSG,      \
               expected, token);      \
})

#define YYSyntax_Error_Clear(loc, expected) \
({                                          \
  yyclearin;                                \
  YYSyntax_Error(loc, expected);            \
})

#define YYSyntax_ErrorMsg(loc, fmt, ...)          \
({                                                \
  yyerrok;                                        \
  DeclarePosition(pos, loc);                      \
  Syntax_Error(ps, &pos, fmt"\n", ##__VA_ARGS__); \
})

#define YYSyntax_ErrorMsg_Clear(loc, fmt, ...) \
({                                             \
  yyclearin;                                   \
  YYSyntax_ErrorMsg(loc, fmt, ##__VA_ARGS__);  \
})

%}

%union {
  uchar ucVal;
  int64 IVal;
  float64 FVal;
  String SVal;
  Vector *List;
  Expr *Expr;
  Stmt *Stmt;
  TypeDesc *TypeDesc;
  IdType *IdType;
  int Operator;
}

%token TYPELESS_ASSIGN
%token PLUS_ASSGIN
%token MINUS_ASSIGN
%token MULT_ASSIGN
%token DIV_ASSIGN
%token MOD_ASSIGN
%token AND_ASSIGN
%token OR_ASSIGN
%token XOR_ASSIGN
%token RSHIFT_ASSIGN
%token LSHIFT_ASSIGN
%token OP_POWER
%token DOTDOTDOT
%token DOTDOTLESS

%token OP_EQ
%token OP_NE
%token OP_GE
%token OP_LE
%token OP_AND
%token OP_OR
%token OP_NOT
%token OP_LSHIFT
%token OP_RSHIFT

%token PACKAGE
%token IF
%token ELSE
%token WHILE
%token FOR
%token MATCH
%token BREAK
%token FALLTHROUGH
%token CONTINUE
%token DEFAULT
%token VAR
%token FUNC
%token FAT_ARROW
%token TOKEN_RETURN
%token CLASS
%token TRAIT
%token ENUM
%token IN
%token CONST
%token IMPORT
%token GO
%token DEFER
%token NATIVE

%token CHAR
%token BYTE
%token INTEGER
%token FLOAT
%token BOOL
%token STRING
%token ERROR
%token ANY

%token SELF
%token SUPER
%token TOKEN_NIL
%token TOKEN_TRUE
%token TOKEN_FALSE

%token <IVal> BYTE_LITERAL
%token <ucVal> CHAR_LITERAL
%token <IVal> INT_LITERAL
%token <FVal> FLOAT_LITERAL
%token <SVal> STRING_LITERAL
%token <SVal> ID

%type <TypeDesc> Type
%type <TypeDesc> ArrayType
%type <TypeDesc> BaseType
%type <TypeDesc> VArgType
%type <TypeDesc> PrimitiveType
%type <TypeDesc> KlassType
%type <TypeDesc> FunctionType
%type <TypeDesc> MapType
%type <TypeDesc> KeyType
%type <TypeDesc> TupleType
%type <IdType> IDType
%type <List> IDTypeList
%type <List> TypeList
%type <List> ParameterList
%type <List> IDList

%type <Stmt> ConstDeclaration
%type <Stmt> VariableDeclaration
%type <Stmt> FunctionDeclaration
%type <Stmt> TypeDeclaration
%type <List> ClassMemberDeclsOrEmpty
%type <List> ClassMemberDeclarations
%type <Stmt> ClassMemberDeclaration
%type <List> ExtendsOrEmpty
%type <List> WithesOrEmpty
%type <List> Traits
%type <List> TraitMemberDeclsOrEmpty
%type <List> TraitMemberDeclarations
%type <Stmt> TraitMemberDeclaration
%type <Stmt> FieldDeclaration
%type <Stmt> ProtoDeclaration
%type <List> Block
%type <List> ExprOrBlock
%type <List> LocalStatements
%type <Stmt> LocalStatement
%type <Stmt> ExprStatement
%type <Stmt> VariableDeclarationTypeless
%type <Stmt> Assignment
%type <Stmt> IfStatement
%type <Stmt> OrElseStatement
%type <Stmt> WhileStatement
%type <Stmt> MatchStatement
%type <List> MatchClauses
%type <Stmt> ForStatement
%type <Stmt> JumpStatement
%type <Stmt> ReturnStatement

%type <Expr> PrimaryExpression
%type <Expr> Atom
%type <Expr> AttributeExpression
%type <Expr> SubScriptExpression
%type <Expr> CallExpression
%type <Expr> SliceExpression
%type <Expr> CONSTANT
%type <Expr> ArrayExpression
%type <Expr> MapExpression
%type <List> MapKeyValueList
%type <Expr> MapKeyValue
%type <Expr> AnonyExpression
%type <Expr> TupleExpression
%type <Operator> UnaryOp
%type <Expr> UnaryExpression
%type <Expr> MultipleExpression
%type <Expr> AddExpression
%type <Expr> ShiftExpression
%type <Expr> RelationExpression
%type <Expr> EqualityExpression
%type <Expr> AndExpression
%type <Expr> ExclusiveOrExpression
%type <Expr> InclusiveOrExpression
%type <Expr> LogicAndExpression
%type <Expr> LogicOrExpression
%type <Expr> RangeExpression
%type <Expr> Expression
%type <List> ExpressionList
%type <Operator> AssignOp

/*
%token PREC_0
%token PREC_1

%precedence PREC_0
%precedence PREC_1
*/

%precedence INT_LITERAL CHAR_LITERAL
%precedence '|'
%precedence ID
%precedence '(' '.'
%precedence ')'
%precedence FAT_ARROW

%locations

%parse-param {ParserState *ps}
%parse-param {void *scanner}
%define api.pure full
%lex-param {void *scanner}
%code provides {
  int yylex(YYSTYPE *yylval_param, YYLTYPE *yylloc, void *yyscanner);
}

%start CompileUnit

%%

Type
  : BaseType
  {
    $$ = $1;
  }
  | ArrayType
  {
    $$ = $1;
  }
  ;

BaseType
  : PrimitiveType
  {
    $$ = $1;
  }
  | KlassType
  {
    $$ = $1;
  }
  | FunctionType
  {
    $$ = $1;
  }
  | MapType
  {
    $$ = $1;
  }
  | TupleType
  {
    $$ = NULL;
  }
  ;

ArrayType
  : '[' Type ']'
  {
    $$ = NULL; //$$ = TypeDesc_Get_Array($1, $2);
  }
  ;

MapType
  : '[' KeyType ':' Type ']'
  {
    $$ = TypeDesc_Get_Map($2, $4);
  }
  ;

KeyType
  : INTEGER
  {
    $$ = TypeDesc_Get_Base(BASE_INT);
  }
  | STRING
  {
    $$ = TypeDesc_Get_Base(BASE_STRING);
  }
  | KlassType
  {
    $$ = $1;
  }
  ;

PrimitiveType
  : BYTE
  {
    $$ = TypeDesc_Get_Base(BASE_BYTE);
  }
  | CHAR
  {
    $$ = TypeDesc_Get_Base(BASE_CHAR);
  }
  | INTEGER
  {
    $$ = TypeDesc_Get_Base(BASE_INT);
  }
  | FLOAT
  {
    $$ = TypeDesc_Get_Base(BASE_FLOAT);
  }
  | BOOL
  {
    $$ = TypeDesc_Get_Base(BASE_BOOL);
  }
  | STRING
  {
    $$ = TypeDesc_Get_Base(BASE_STRING);
  }
  | ERROR
  {
    $$ = TypeDesc_Get_Base(BASE_ERROR);
  }
  | ANY
  {
    $$ = TypeDesc_Get_Base(BASE_ANY);
  }
  ;

KlassType
  : ID
  {
    DeclareIdent(klazz, $1, @1);
    $$ = Parser_New_KlassType(ps, NULL, &klazz);
  }
  | ID '.' ID
  {
    DeclareIdent(id, $1, @1);
    DeclareIdent(klazz, $3, @3);
    $$ = Parser_New_KlassType(ps, &id, &klazz);
  }
  | ID '<' TypeList '>'
  {

  }
  | ID '.' ID '<' TypeList '>'
  {

  }
  ;

FunctionType
  : FUNC '(' TypeList ')' Type
  {
    /*
    if ($3 == NULL || $5 == NULL) {
      $$ = NULL;
    } else {
      Vector *pvec = Vector_New();
      Vector *rvec = Vector_New();
      IdType *idType;
      Vector_ForEach(idType, $3) {
        TYPE_INCREF(idType->type.desc);
        Vector_Append(pvec, idType->type.desc);
      }
      Vector_ForEach(idType, $5) {
        TYPE_INCREF(idType->type.desc);
        Vector_Append(rvec, idType->type.desc);
      }
      $$ = TypeDesc_Get_Proto(pvec, rvec);
    }
    Free_IdTypeList($3);
    Free_IdTypeList($5);
    */
  }
  | FUNC '<' GenericTypes '>' '(' TypeList ')' Type
  {

  }
  | FUNC '(' TypeList ')'
  {
    /*
    if ($3 == NULL) {
      $$ = NULL;
    } else {
      Vector *pvec = Vector_New();
      IdType *idType;
      Vector_ForEach(idType, $3) {
        TYPE_INCREF(idType->type.desc);
        Vector_Append(pvec, idType->type.desc);
      }
      $$ = TypeDesc_Get_Proto(pvec, NULL);
    }
    Free_IdTypeList($3);
    */
  }
  | FUNC '<' GenericTypes '>' '(' TypeList ')'
  {

  }
  | FUNC '(' ')' Type
  {
    //$$ = Parser_Get_Proto(NULL, $4);
  }
  | FUNC '<' GenericTypes '>' '(' ')' Type
  {

  }
  | FUNC '(' ')'
  {
    //$$ = Parser_Get_Proto(NULL, NULL);
  }
  ;

TupleType
  : '(' TypeList ')'
  {
    $$ = NULL;
  }
  ;

IDType
  : ID Type
  {
    DeclareIdent(id, $1, @1);
    DeclareType(type, $2, @2);
    $$ = New_IdType(&id, type);
  }
  | ID error
  {
    YYSyntax_Error(@2, "Type");
    $$ = NULL;
  }
  ;

IDTypeList
  : IDType
  {
    if ($1 != NULL) {
      $$ = Vector_New();
      Vector_Append($$, $1);
    } else {
      $$ = NULL;
    }
  }
  | IDTypeList ',' IDType
  {
    if ($1 != NULL && $3 != NULL) {
      $$ = $1;
      Vector_Append($$, $3);
    } else {
      $$ = NULL;
      Free_IdTypeList($1);
      Free_IdType($3);
    }
  }
  | IDTypeList ',' error
  {
    Free_IdTypeList($1);
    YYSyntax_Error_Clear(@3, "IDType");
    $$ = NULL;
  }
  | IDTypeList error
  {
    Free_IdTypeList($1);
    YYSyntax_Error_Clear(@2, ",");
    $$ = NULL;
  }
  ;

ParameterList
  : IDTypeList
  {
    $$ = $1;
  }
  | IDTypeList ',' ID VArgType
  {
    DeclareIdent(id, $3, @3);
    DeclareType(type, $4, @4);
    $$ = $1;
    Vector_Append($$, New_IdType(&id, type));
  }
  | ID VArgType
  {
    DeclareIdent(id, $1, @1);
    DeclareType(type, $2, @2);
    $$ = Vector_Capacity(1);
    Vector_Append($$, New_IdType(&id, type));
  }
  | error
  {
    YYSyntax_ErrorMsg_Clear(@1, "invalid parameter list");
    $$ = NULL;
  }
  ;

VArgType
  : DOTDOTDOT
  {
    $$ = TypeDesc_Get_Varg(NULL);
  }
  | DOTDOTDOT BaseType
  {
    $$ = TypeDesc_Get_Varg($2);
  }
  ;

TypeList
  : Type
  {
    $$ = Vector_New();
    DeclareType(type, $1, @1);
    Vector_Append($$, New_IdType(NULL, type));
  }
  | TypeList ',' Type
  {
    if ($1 != NULL) {
      $$ = $1;
      DeclareType(type, $3, @3);
      Vector_Append($$, New_IdType(NULL, type));
    } else {
      $$ = NULL;
      /* FIXME: has error ? */
      TYPE_DECREF($3);
    }
  }
  | TypeList ',' error
  {
    Free_IdTypeList($1);
    YYSyntax_Error_Clear(@3, "Type");
    $$ = NULL;
  }
  ;

IDList
  : ID
  {
    Ident *id = New_Ident($1);
    SetPosition(id->pos, @1);
    $$ = Vector_New();
    Vector_Append($$, id);
  }
  | IDList ',' ID
  {
    Ident *id = New_Ident($3);
    SetPosition(id->pos, @3);
    $$ = $1;
    Vector_Append($$, id);
  }
  ;

CompileUnit
  : Package Imports ModuleStatementsOrEmpty
  | Package ModuleStatementsOrEmpty
  ;

Package
  : PACKAGE ID ';'
  {
    int len = strlen($2.str);
    if (len >= PKG_NAME_MAX)
      YYSyntax_ErrorMsg(@2, PKG_NAME_LEN_OVER_MAX_MSG, len, PKG_NAME_MAX);
    DeclareIdent(id, $2, @2);
    Parser_Set_PkgName(ps, &id);
  }
  | PACKAGE ID error
  {
    YYSyntax_Error(@2, ";");
    YYACCEPT;
  }
  | PACKAGE error
  {
    YYSyntax_Error(@2, "<package-name>");
    YYACCEPT;
  }
  | error
  {
    YYSyntax_Error(@1, "<package-name>");
    YYACCEPT;
  }
  ;

Imports
  : Import
  | Imports Import
  ;

Import
  : IMPORT STRING_LITERAL ';'
  {
    DeclareIdent(path, $2, @2);
    Parser_New_Import(ps, NULL, &path);
  }
  | IMPORT ID STRING_LITERAL ';'
  {
    DeclareIdent(id, $2, @2);
    DeclareIdent(path, $3, @3);
    Parser_New_Import(ps, &id, &path);
  }
  | IMPORT '.' STRING_LITERAL ';'
  {
    Ident id;
    id.name = ".";
    SetPosition(id.pos, @2);
    DeclareIdent(path, $3, @3);
    Parser_New_Import(ps, &id, &path);
  }
  | IMPORT STRING_LITERAL error
  {
    YYSyntax_Error(@3, ";");
    YYACCEPT;
  }
  | IMPORT ID STRING_LITERAL error
  {
    YYSyntax_Error(@3, ";");
    YYACCEPT;
  }
  | IMPORT '.' STRING_LITERAL error
  {
    YYSyntax_Error(@3, ";");
    YYACCEPT;
  }
  | IMPORT ID error
  {
    YYSyntax_Error(@3, "<package-path>");
    YYACCEPT;
  }
  | IMPORT '.' error
  {
    YYSyntax_Error(@2, "<package-path>");
    YYACCEPT;
  }
  | IMPORT error
  {
    YYSyntax_Error(@2, "<package-path>, <ID> or *");
    YYACCEPT;
  }
  ;

ModuleStatementsOrEmpty
  : %empty
  {
    EMPTY_FILE
  }
  | ModuleStatements
  ;

ModuleStatements
  : ModuleStatement
  | ModuleStatements ModuleStatement
  ;

ModuleStatement
  : ';'
  {
  }
  | VariableDeclaration
  {
    //Parser_New_Variables(ps, $1);
  }
  | ConstDeclaration
  {
    //Parser_New_Variables(ps, $1);
  }
  | FunctionDeclaration
  {
    //Parser_New_Function(ps, $1);
  }
  | NATIVE ProtoDeclaration
  {
    //((FuncDeclStmt *)$2)->native = 1;
    //Parser_New_Function(ps, $2);
  }
  | TypeDeclaration
  {
    //Parser_New_ClassOrTrait(ps, $1);
  }
  | error
  {
    YYSyntax_ErrorMsg_Clear(@1, "invalid statement");
  }
  ;

ConstDeclaration
  : CONST ID '=' Expression ';'
  {
    //TypeWrapper type = {NULL};
    //$$ = Parser_Do_Constants(ps, $2, type, $4);
  }
  | CONST ID Type '=' Expression ';'
  {
    //DeclareType(type, $3, @3);
    //$$ = Parser_Do_Constants(ps, $2, type, $5);
  }
  | CONST ID '=' error
  {
    //Free_IdentList($2);
    YYSyntax_Error(@4, "expr-list");
    $$ = NULL;
  }
  | CONST ID Type '=' error
  {
    //Free_IdentList($2);
    YYSyntax_Error(@5, "expr-list");
    $$ = NULL;
  }
  | CONST error
  {
    YYSyntax_Error(@2, "id-list");
    $$ = NULL;
  }
  ;

VariableDeclaration
  : VAR ID Type ';'
  {
    //DeclareType(type, $3, @3);
    //$$ = Parser_Do_Variables(ps, $2, type, NULL);
  }
  | VAR ID '=' Expression ';'
  {
    //TypeWrapper type = {NULL};
    //$$ = Parser_Do_Variables(ps, $2, type, $4);
  }
  | VAR ID Type '=' Expression ';'
  {
    //DeclareType(type, $3, @3);
    //$$ = Parser_Do_Variables(ps, $2, type, $5);
  }
  | VAR '(' IDList ')' '=' Expression ';'
  {

  }
  | VAR ID '=' error
  {
    //Free_IdentList($2);
    YYSyntax_Error(@4, "right's expression-list");
    $$ = NULL;
  }
  | VAR ID Type '=' error
  {
    //Free_IdentList($2);
    YYSyntax_Error(@5, "right's expression-list");
    $$ = NULL;
  }
  | VAR error
  {
    YYSyntax_Error(@2, "id-list");
    $$ = NULL;
  }
  ;

FunctionDeclaration
  : FUNC Name '(' ParameterList ')' Type ExprOrBlock
  {
    //DeclareIdent(id, $2, @2);
    //$$ = Stmt_From_FuncDecl(id, $4, $6, $7);
  }
  | FUNC Name '(' ParameterList ')' ExprOrBlock
  {
    //DeclareIdent(id, $2, @2);
    //$$ = Stmt_From_FuncDecl(id, $4, NULL, $6);
  }
  | FUNC Name '(' ')' Type ExprOrBlock
  {
    //DeclareIdent(id, $2, @2);
    //$$ = Stmt_From_FuncDecl(id, NULL, $5, $6);
  }
  | FUNC Name '(' ')' ExprOrBlock
  {
    //DeclareIdent(id, $2, @2);
    //$$ = Stmt_From_FuncDecl(id, NULL, NULL, $5);
  }
  | FUNC error
  {
    YYSyntax_Error(@2, "ID");
    $$ = NULL;
  }
  ;

TypeDeclaration
  : CLASS Name ExtendsOrEmpty '{' ClassMemberDeclsOrEmpty '}'
  {
    printf("class-1\n");
    //DeclareIdent(id, $2, @2);
    //$$ = Stmt_From_Klass(id, CLASS_KIND, $3, $5);
  }
  | CLASS Name ExtendsOrEmpty
  {
    printf("class-2\n");
    //DeclareIdent(id, $2, @2);
    //$$ = Stmt_From_Klass(id, CLASS_KIND, $3, NULL);
  }
  | TRAIT Name ExtendsOrEmpty '{' TraitMemberDeclsOrEmpty '}'
  {
    printf("trait-1\n");
    //DeclareIdent(id, $2, @2);
    //$$ = Stmt_From_Klass(id, TRAIT_KIND, $3, $5);
  }
  | TRAIT Name ExtendsOrEmpty
  {
    printf("trait-2\n");
    //DeclareIdent(id, $2, @2);
    //$$ = Stmt_From_Klass(id, TRAIT_KIND, $3, NULL);
  }
  | ENUM Name '{' EnumDeclaration '}'
  {

  }
  | ENUM Name '{' EnumDeclaration ',' '}'
  {

  }
  ;

Name
  : ID
  | ID '<' GenericTypes '>'
  {

  }
  ;

GenericTypes
  : GenericType
  | GenericTypes ',' GenericType
  ;

GenericType
  : ID
  | ID ':' KlassType
  ;

ExtendsOrEmpty
  : %empty
  {
    $$ = NULL;
  }
  | ':' KlassType WithesOrEmpty
  {
    /*
    int size = Vector_Size($3);
    $$ = Vector_Capacity(size + 1);
    TYPE_INCREF($2);
    Vector_Append($$, $2);
    Vector_Concat($$, $3);
    Vector_Free_Self($3);
    */
  }
  ;

WithesOrEmpty
  : %empty
  {
  }
  | Traits
  {
    //$$ = $1;
  }
  ;

Traits
  : ',' KlassType
  {
    printf("With\n");
    //$$ = Vector_New();
    //TYPE_INCREF($2);
    //Vector_Append($$, $2);
  }
  | Traits ',' KlassType
  {
    printf("With Traits\n");
   // $$ = $1;
   // TYPE_INCREF($3);
   // Vector_Append($$, $3);
  }
  ;

ClassMemberDeclsOrEmpty
  : %empty
  {
    $$ = NULL;
  }
  | ClassMemberDeclarations
  {
    $$ = $1;
  }
  ;

ClassMemberDeclarations
  : ClassMemberDeclaration
  {
    $$ = Vector_New();
    if ($1 != NULL)
      Vector_Append($$, $1);
  }
  | ClassMemberDeclarations ClassMemberDeclaration
  {
    $$ = $1;
    if ($2 != NULL)
      Vector_Append($$, $2);
  }
  ;

ClassMemberDeclaration
  : FieldDeclaration
  {
    $$ = $1;
  }
  | FunctionDeclaration
  {
    $$ = $1;
  }
  | NATIVE ProtoDeclaration
  {
    ((FuncDeclStmt *)$2)->native = 1;
    $$ = $2;
  }
  | ';'
  {
    $$ = NULL;
  }
  ;

TraitMemberDeclsOrEmpty
  : %empty
  {
    $$ = NULL;
  }
  | TraitMemberDeclarations
  {
    $$ = $1;
  }
  ;

TraitMemberDeclarations
  : TraitMemberDeclaration
  {
    $$ = Vector_New();
    if ($1 != NULL)
      Vector_Append($$, $1);
  }
  | TraitMemberDeclarations TraitMemberDeclaration
  {
    $$ = $1;
    if ($2 != NULL)
      Vector_Append($$, $2);
  }
  ;

TraitMemberDeclaration
  : ClassMemberDeclaration
  {
    $$ = $1;
  }
  | ProtoDeclaration
  {
    $$ = $1;
  }
  ;

FieldDeclaration
  : ID Type ';'
  {
    DeclareIdent(id, $1, @1);
    DeclareType(type, $2, @2);
    $$ = Stmt_From_VarDecl(id, type, NULL);
  }
  | ID '=' Expression ';'
  {
    DeclareIdent(id, $1, @1);
    TypeWrapper type = {NULL};
    $$ = Stmt_From_VarDecl(id, type, $3);
  }
  | ID Type '=' Expression ';'
  {
    DeclareIdent(id, $1, @1);
    DeclareType(type, $2, @2);
    $$ = Stmt_From_VarDecl(id, type, $4);
  }
  ;

ProtoDeclaration
  : FUNC Name '(' ParameterList ')' Type ';'
  {
    //DeclareIdent(id, $2, @2);
    //$$ = Stmt_From_ProtoDecl(id, $4, $6);
  }
  | FUNC Name '(' ParameterList ')' ';'
  {
    //DeclareIdent(id, $2, @2);
    //$$ = Stmt_From_ProtoDecl(id, $4, NULL);
  }
  | FUNC Name '(' ')' Type ';'
  {
    //DeclareIdent(id, $2, @2);
    //$$ = Stmt_From_ProtoDecl(id, NULL, $5);
  }
  | FUNC Name '(' ')' ';'
  {
    //DeclareIdent(id, $2, @2);
    //$$ = Stmt_From_ProtoDecl(id, NULL, NULL);
  }
  ;

EnumDeclaration
  : ID
  | ID '(' TypeList ')'
  | EnumDeclaration ',' ID
  | EnumDeclaration ',' ID '(' TypeList ')'
  ;

Block
  : '{' LocalStatements '}'
  {
    $$ = $2;
  }
  | '{' '}'
  {
    $$ = NULL;
  }
  ;

LocalStatements
  : LocalStatement
  {
    $$ = Vector_New();
    if ($1 != NULL)
      Vector_Append($$, $1);
  }
  | LocalStatements LocalStatement
  {
    $$ = $1;
    if ($2 != NULL)
      Vector_Append($$, $2);
  }
  ;

LocalStatement
  : ';'
  {
    $$ = NULL;
  }
  | ExprStatement
  {
    $$ = $1;
  }
  | VariableDeclaration
  {
    $$ = $1;
  }
  | VariableDeclarationTypeless
  {
    $$ = $1;
  }
  | Assignment
  {
    $$ = $1;
  }
  | IfStatement
  {
    $$ = $1;
  }
  | WhileStatement
  {
    $$ = $1;
  }
  | MatchStatement
  {
    $$ = $1;
  }
  | ForStatement
  {
    $$ = $1;
  }
  | JumpStatement
  {
    $$ = $1;
  }
  | ReturnStatement
  {
    $$ = $1;
  }
  | Block
  {
    $$ = Stmt_From_List($1);
    ((ListStmt *)$$)->block = 1;
  }
  | error
  {
    YYSyntax_ErrorMsg_Clear(@1, "invalid local statement");
    $$ = NULL;
  }
  ;

ExprStatement
  : Expression ';'
  {
    $$ = Stmt_From_Expr($1);
  }
  ;

/*
 * TYPELESS_ASSIGN is used only in local blocks
 * ExpressionList is really IDList??
 */
VariableDeclarationTypeless
  : Expression TYPELESS_ASSIGN Expression ';'
  {
    //$$ = Parser_Do_Typeless_Variables(ps, $1, $3);
  }
  ;

IfStatement
  : IF Expression ExprOrBlock OrElseStatement
  {
    //$$ = stmt_from_if($2, $3, $4);
    //$$->if_stmt.belse = 0;
  }
  ;

OrElseStatement
  : %empty
  {
    $$ = NULL;
  }
  | ELSE ExprOrBlock
  {
    //$$ = stmt_from_if(NULL, $2, NULL);
    //$$->if_stmt.belse = 1;
  }
  | ELSE IfStatement
  {
    //$$ = $2;
    //$$->if_stmt.belse = 1;
  }
  ;

WhileStatement
  : WHILE Expression ExprOrBlock
  {
    //stmt_from_while($2, $3, 1);
  }
  ;

MatchStatement
  : MATCH Expression '{' MatchClauses '}'
  {
    printf("MatchStatement-1\n");
    $$ = NULL;
  }
  | MATCH Expression '{' MatchClauses ',' '}'
  {

  }
  ;

MatchClauses
  : MatchClause
  {
    /*
    $$ = Vector_New();
    Vector_Append($$, $1);
    */
  }
  | MatchClauses ',' MatchClause
  {
    /*
    $$ = $1;
    if ($2->test == NULL) {
      // default case
      struct test_block *tb = Vector_Get($1, 0);
      if (tb != NULL && tb->test == NULL) {
        fprintf(stderr, "[ERROR] default case needs only one\n");
        exit(0);
      } else {
        Vector_Set($1, 0, $2);
      }
    } else {
      Vector_Append($1, $2);
    }
    */
  }
  ;

MatchClause
  : PatternExpression PatternCondition FAT_ARROW PatternClause
  {
    //$$ = new_test_block($2, $4);
  }
  ;

PatternExpression
  : '_'
  | CONSTANT
  | IntOr INT_LITERAL
  | CharOr CHAR_LITERAL
  | RangeExpression
  | ID
  | ID '(' ExpressionList ')'
  | ID '.' ID
  | ID '.' ID '(' ExpressionList ')'
  | TupleExpression
  ;

PatternCondition
  : %empty
  | IF Expression
  ;

PatternClause
  : ExprOrBlock
  | Expression
  ;

IntOr
  : INT_LITERAL '|'
  | IntOr INT_LITERAL '|'
  ;

CharOr
  : CHAR_LITERAL '|'
  | CharOr CHAR_LITERAL '|'
  ;

ForStatement
  : FOR IDList IN Expression ExprOrBlock
  {
    $$ = NULL;
  }
  ;

JumpStatement
  : BREAK ';'
  {
    //$$ = stmt_from_jump(BREAK_KIND, 1);
  }
  | CONTINUE ';'
  {
    //$$ = stmt_from_jump(CONTINUE_KIND, 1);
  }
  ;

ReturnStatement
  : TOKEN_RETURN ';'
  {
    $$ = Stmt_From_Return(NULL);
    SetPosition(((ReturnStmt *)$$)->pos, @1);
  }
  | TOKEN_RETURN Expression ';'
  {
    $$ = Stmt_From_Return($2);
    SetPosition(((ReturnStmt *)$$)->pos, @2);
  }
  | TOKEN_RETURN error
  {
    YYSyntax_Error(@2, "expression");
    $$ = NULL;
  }
  ;

PrimaryExpression
  : Atom
  {
    $$ = $1;
  }
  | AttributeExpression
  {
    $$ = $1;
  }
  | CallExpression
  {
    $$ = $1;
  }
  | SubScriptExpression
  {
    $$ = $1;
  }
  | SliceExpression
  {
    $$ = $1;
  }
  ;

AttributeExpression
  : PrimaryExpression '.' ID
  {
    DeclareIdent(id, $3, @3);
    $$ = Expr_From_Attribute(id, $1);
    SetPosition($$->pos, @1);
  }
  ;

CallExpression
  : PrimaryExpression '(' ')'
  {
    $$ = Expr_From_Call(NULL, $1);
    SetPosition($$->pos, @1);
  }
  | PrimaryExpression '(' ExpressionList ')'
  {
    $$ = Expr_From_Call($3, $1);
    SetPosition($$->pos, @1);
  }
  ;

SubScriptExpression
  : PrimaryExpression '[' Expression ']'
  {
    $$ = Expr_From_SubScript($3, $1);
    SetPosition($$->pos, @1);
  }
  ;

SliceExpression
  : PrimaryExpression '[' Expression ':' Expression ']'
  {
    $$ = Expr_From_Slice($3, $5, $1);
    SetPosition($$->pos, @1);
  }
  | PrimaryExpression '[' ':' Expression ']'
  {
    $$ = Expr_From_Slice(NULL, $4, $1);
    SetPosition($$->pos, @1);
  }
  | PrimaryExpression '[' Expression ':' ']'
  {
    $$ = Expr_From_Slice($3, NULL, $1);
    SetPosition($$->pos, @1);
  }
  | PrimaryExpression '[' ':' ']'
  {
    $$ = Expr_From_Slice(NULL, NULL, $1);
    SetPosition($$->pos, @1);
  }
  ;

Atom
  : ID
  {
    $$ = Expr_From_Ident($1.str);
    SetPosition($$->pos, @1);
  }
  | CONSTANT
  {
    $$ = $1;
  }
  | TOKEN_NIL
  {
    $$ = Expr_From_Nil();
    SetPosition($$->pos, @1);
  }
  | SELF
  {
    $$ = Expr_From_Self();
    SetPosition($$->pos, @1);
  }
  | SUPER
  {
    $$ = Expr_From_Super();
    SetPosition($$->pos, @1);
  }
  | '(' Expression ')'
  {
    $$ = $2;
  }
  | ArrayExpression
  {
    $$ = $1;
  }
  | MapExpression
  {
    $$ = $1;
  }
  | TupleExpression
  {

  }
  | AnonyExpression
  {
    $$ = $1;
  }
  ;

CONSTANT
  : CHAR_LITERAL
  {
    $$ = Expr_From_Char($1);
    SetPosition($$->pos, @1);
  }
  | INT_LITERAL
  {
    $$ = Expr_From_Integer($1);
    SetPosition($$->pos, @1);
  }
  | FLOAT_LITERAL
  {
    $$ = Expr_From_Float($1);
    SetPosition($$->pos, @1);
  }
  | STRING_LITERAL
  {
    $$ = Expr_From_String($1.str);
    SetPosition($$->pos, @1);
  }
  | TOKEN_TRUE
  {
    $$ = Expr_From_Bool(1);
    SetPosition($$->pos, @1);
  }
  | TOKEN_FALSE
  {
    $$ = Expr_From_Bool(0);
    SetPosition($$->pos, @1);
  }
  ;

ArrayExpression
  : '[' ExpressionList ']'
  {
    $$ = Expr_From_ArrayListExpr($2);
    SetPosition($$->pos, @1);
  }
  | '[' ExpressionList ',' ']'
  {
    /* last one with comma */
    $$ = Expr_From_ArrayListExpr($2);
    SetPosition($$->pos, @1);
  }
  | '[' ']'
  {

  }
  ;

MapExpression
  : '{' MapKeyValueList '}'
  {
    $$ = Expr_From_MapListExpr($2);
    SetPosition($$->pos, @1);
  }
  | '{' MapKeyValueList ',' '}'
  {
    /* last one with comma */
    $$ = Expr_From_MapListExpr($2);
    SetPosition($$->pos, @1);
  }
  | '{' ':' '}'
  {

  }
  ;

MapKeyValueList
  : MapKeyValue
  {
    $$ = Vector_New();
    Vector_Append($$, $1);
  }
  | MapKeyValueList ',' MapKeyValue
  {
    $$ = $1;
    Vector_Append($$, $3);
  }
  ;

MapKeyValue
  : Expression ':' Expression
  {
    $$ = Expr_From_MapEntry($1, $3);
    SetPosition($$->pos, @1);
  }
  ;

TupleExpression
  : '(' Expression ',' ')'
  {

  }
  | '(' ExpressionListComma Expression ')'
  {

  }
  | '(' ExpressionListComma Expression ',' ')'
  {

  }
  | '(' ')'
  {

  }
  ;

ExpressionListComma
  : Expression ','
  | ExpressionListComma Expression ','
  ;

AnonyExpression
  : FUNC '(' ParameterList ')' Type ExprOrBlock
  {
    //$$ = Expr_From_Anony($3, $5, $6);
    //SetPosition($$->pos, @1);
  }
  | FUNC '(' ParameterList ')' ExprOrBlock
  {
    //$$ = Expr_From_Anony($3, NULL, $5);
    //SetPosition($$->pos, @1);
  }
  | FUNC '(' ')' Type ExprOrBlock
  {
    //$$ = Expr_From_Anony(NULL, $4, $5);
    //SetPosition($$->pos, @1);
  }
  | FUNC '(' ')' ExprOrBlock
  {
    //$$ = Expr_From_Anony(NULL, NULL, $4);
    //SetPosition($$->pos, @1);
  }
  | FUNC error
  {
    YYSyntax_ErrorMsg_Clear(@2, "invalid anonymous function");
    $$ = NULL;
  }
  ;

ExprOrBlock
  : '{' Expression '}'
  {

  }
  | Block
  {
    $$ = $1;
  }
  ;

UnaryExpression
  : PrimaryExpression
  {
    $$ = $1;
  }
  | UnaryOp UnaryExpression
  {
    $$ = Expr_From_Unary($1, $2);
    SetPosition($$->pos, @1);
  }
  ;

UnaryOp
  : '+'
  {
    $$ = UNARY_PLUS;
  }
  | '-'
  {
    $$ = UNARY_NEG;
  }
  | '~'
  {
    $$ = UNARY_BIT_NOT;
  }
  | OP_NOT
  {
    $$ = UNARY_LNOT;
  }
  ;

MultipleExpression
  : UnaryExpression
  {
    $$ = $1;
  }
  | MultipleExpression '*' UnaryExpression
  {
    $$ = Expr_From_Binary(BINARY_MULT, $1, $3);
    SetPosition($$->pos, @1);
  }
  | MultipleExpression '/' UnaryExpression
  {
    $$ = Expr_From_Binary(BINARY_DIV, $1, $3);
    SetPosition($$->pos, @1);
  }
  | MultipleExpression '%' UnaryExpression
  {
    $$ = Expr_From_Binary(BINARY_MOD, $1, $3);
    SetPosition($$->pos, @1);
  }
  | MultipleExpression OP_POWER UnaryExpression
  {
    $$ = Expr_From_Binary(BINARY_POWER, $1, $3);
    SetPosition($$->pos, @1);
  }
  ;

AddExpression
  : MultipleExpression
  {
    $$ = $1;
  }
  | AddExpression '+' MultipleExpression
  {
    $$ = Expr_From_Binary(BINARY_ADD, $1, $3);
    SetPosition($$->pos, @1);
  }
  | AddExpression '-' MultipleExpression
  {
    $$ = Expr_From_Binary(BINARY_SUB, $1, $3);
    SetPosition($$->pos, @1);
  }
  ;

ShiftExpression
  : AddExpression
  {
    $$ = $1;
  }
  | ShiftExpression OP_LSHIFT AddExpression
  {
    $$ = Expr_From_Binary(BINARY_LSHIFT, $1, $3);
    SetPosition($$->pos, @1);
  }
  | ShiftExpression OP_RSHIFT AddExpression
  {
    $$ = Expr_From_Binary(BINARY_RSHIFT, $1, $3);
    SetPosition($$->pos, @1);
  }
  ;

RelationExpression
  : ShiftExpression
  {
    $$ = $1;
  }
  | RelationExpression '<' ShiftExpression
  {
    $$ = Expr_From_Binary(BINARY_LT, $1, $3);
    SetPosition($$->pos, @1);
  }
  | RelationExpression '>' ShiftExpression
  {
    $$ = Expr_From_Binary(BINARY_GT, $1, $3);
    SetPosition($$->pos, @1);
  }
  | RelationExpression OP_LE ShiftExpression
  {
    $$ = Expr_From_Binary(BINARY_LE, $1, $3);
    SetPosition($$->pos, @1);
  }
  | RelationExpression OP_GE ShiftExpression
  {
    $$ = Expr_From_Binary(BINARY_GE, $1, $3);
    SetPosition($$->pos, @1);
  }
  ;

EqualityExpression
  : RelationExpression
  {
    $$ = $1;
  }
  | EqualityExpression OP_EQ RelationExpression
  {
    $$ = Expr_From_Binary(BINARY_EQ, $1, $3);
    SetPosition($$->pos, @1);
  }
  | EqualityExpression OP_NE RelationExpression
  {
    $$ = Expr_From_Binary(BINARY_NEQ, $1, $3);
    SetPosition($$->pos, @1);
  }
  ;

AndExpression
  : EqualityExpression
  {
    $$ = $1;
  }
  | AndExpression '&' EqualityExpression
  {
    $$ = Expr_From_Binary(BINARY_BIT_AND, $1, $3);
    SetPosition($$->pos, @1);
  }
  ;

ExclusiveOrExpression
  : AndExpression
  {
    $$ = $1;
  }
  | ExclusiveOrExpression '^' AndExpression
  {
    $$ = Expr_From_Binary(BINARY_BIT_XOR, $1, $3);
    SetPosition($$->pos, @1);
  }
  ;

InclusiveOrExpression
  : ExclusiveOrExpression
  {
    $$ = $1;
  }
  | InclusiveOrExpression '|' ExclusiveOrExpression
  {
    $$ = Expr_From_Binary(BINARY_BIT_OR, $1, $3);
    SetPosition($$->pos, @1);
  }
  ;

LogicAndExpression
  : InclusiveOrExpression
  {
    $$ = $1;
  }
  | LogicAndExpression OP_AND InclusiveOrExpression
  {
    $$ = Expr_From_Binary(BINARY_LAND, $1, $3);
    SetPosition($$->pos, @1);
  }
  ;

LogicOrExpression
  : LogicAndExpression
  {
    $$ = $1;
  }
  | LogicOrExpression OP_OR LogicAndExpression
  {
    $$ = Expr_From_Binary(BINARY_LOR, $1, $3);
    SetPosition($$->pos, @1);
  }
  ;

RangeExpression
  : LogicOrExpression DOTDOTDOT LogicOrExpression
  {
    $$ = NULL;
  }
  | LogicOrExpression DOTDOTLESS LogicOrExpression
  {
    $$ = NULL;
  }
  ;

NoBockExprOrBlock
  : Expression
  {
    printf("NoBockExprOrBlock-1\n");
  }
  | ExprOrBlock
  {
    printf("NoBockExprOrBlock-2\n");
  }
  ;

/* IDList */
LambdaExpression
  : '(' ExpressionListComma Expression ')' FAT_ARROW NoBockExprOrBlock
  {
    printf("NoBockExprOrBlock-1\n");
  }
  | '(' ID ')' FAT_ARROW NoBockExprOrBlock
  {
    printf("NoBockExprOrBlock-2\n");
  }
  | '(' ')' FAT_ARROW NoBockExprOrBlock
  {
    printf("NoBockExprOrBlock-3\n");
  }
  ;

Expression
  : LogicOrExpression
  {
    $$ = $1;
  }
  | RangeExpression
  {

  }
  | LambdaExpression
  {

  }
  ;

ExpressionList
  : Expression
  {
    $$ = Vector_New();
    Vector_Append($$, $1);
  }
  | ExpressionList ',' Expression
  {
    $$ = $1;
    Vector_Append($$, $3);
  }
  ;

Assignment
  : PrimaryExpression AssignOp Expression ';'
  {
    //$$ = Parser_Do_Assignments(ps, $1, $3);
  }
/*
  | PrimaryExpression CompAssignOp Expression ';'
  {
    if (!Expr_Maybe_Stored($1)) {
      DeclarePosition(pos, @1);
      Syntax_Error(ps, &pos, "expr is not left expr");
      Free_Expr($1);
      Free_Expr($3);
      $$ = NULL;
    } else {
      $$ = Stmt_From_Assign($2, $1, $3);
    }
  }
*/
  ;

AssignOp
  : '='
  {
    $$ = OP_ASSIGN;
  }
  | PLUS_ASSGIN
  {
    $$ = OP_PLUS_ASSIGN;
  }
  | MINUS_ASSIGN
  {
    $$ = OP_MINUS_ASSIGN;
  }
  | MULT_ASSIGN
  {
    $$ = OP_MULT_ASSIGN;
  }
  | DIV_ASSIGN
  {
    $$ = OP_DIV_ASSIGN;
  }
  | MOD_ASSIGN
  {
    $$ = OP_MOD_ASSIGN;
  }
  | AND_ASSIGN
  {
    $$ = OP_AND_ASSIGN;
  }
  | OR_ASSIGN
  {
    $$ = OP_OR_ASSIGN;
  }
  | XOR_ASSIGN
  {
    $$ = OP_XOR_ASSIGN;
  }
  | RSHIFT_ASSIGN
  {
    $$ = OP_RSHIFT_ASSIGN;
  }
  | LSHIFT_ASSIGN
  {
    $$ = OP_LSHIFT_ASSIGN;
  }
  ;

%%
