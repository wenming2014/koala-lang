
#ifndef _KOALA_PARSER_H_
#define _KOALA_PARSER_H_

#include "printcolor.h"
#include "ast.h"
#include "codegen.h"
#include "options.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LINE_MAX_LEN 256
#define TOKEN_MAX_LEN 80

typedef struct line_buf {
  char line[LINE_MAX_LEN];
  int linelen;
  int lineleft;
  char token[TOKEN_MAX_LEN];
  int len;
  int row;
  int col;
  char print;
  char copy;
} LineBuffer;

/*---------------------------------------------------------------------------*/

void codeblock_free(CodeBlock *b);

/*---------------------------------------------------------------------------*/

typedef struct import {
  HashNode hnode;
  lineinfo_t line;
  char *path;
  Symbol *sym;
} Import;

enum scope {
  SCOPE_MODULE = 1,
  SCOPE_CLASS,
  SCOPE_FUNCTION,
  SCOPE_METHOD,
  SCOPE_CLOSURE,
  SCOPE_BLOCK
};

typedef struct parserunit {
  enum scope scope;
  int8 merge;
  int8 loop;
  struct list_head link;
  Symbol *sym;
  STable *stbl;
  CodeBlock *block;
  Vector jmps;
} ParserUnit;

struct extpkg {
  HashNode hnode;
  char *path;
  STable *stbl;
  char *id;
};

/*
 * per one package which includes all source files in the same directory.
 * These files must be the same package name.
 */
typedef struct packageinfo {
  char *pkgfile;     /* package saved in pkgfile */
  char *pkgname;     /* package name */
  Symbol *sym;       /* it's type is pacakge, includes all symbols */
  ParserUnit top;    /* top unit for all parserstate */
  HashTable expkgs;  /* external packages, path as key */
  struct options *options;
} PackageInfo;

PackageInfo *New_PackageInfo(char *pkgfile, struct options *ops);
void parse_module_scope(PackageInfo *pkg);

#define MAX_ERRORS 8

/*
 * ParserState per one source file
 */
typedef struct parserstate {
  char *filename;     /* file name for this module */
  PackageInfo *pkg;   /* package ptr, all modules have the same pacakge */
  void *scanner;      /* lexer pointer */
  LineBuffer line;    /* input line buffer */
  Vector stmts;       /* all statements */
  int lastToken;      /* save last token for if inserted semicolon or not */
  HashTable imports;  /* external types */
  STable *extstbl;    /* external symbol table */
  Symbol *sym;        /* current module's symbol */
  ParserUnit *u;
  int nestlevel;
  struct list_head ustack;
  short olevel;       /* optimization level */
  short wlevel;       /* warning level */
  int errnum;         /* number of errors */
  Vector errors;      /* error messages */
} ParserState;

void Parser_Set_Package(ParserState *ps, char *pkgname);
ParserState *new_parser(PackageInfo *pkg, char *filename);
void destroy_parser(ParserState *ps);
void parser_body(ParserState *ps, Vector *stmts);
void parser_enter_scope(ParserState *ps, STable *stbl, int scope);
void parser_exit_scope(ParserState *ps);
void Parser_PrintError(ParserState *ps, lineinfo_t *line, char *fmt, ...);
void check_unused_imports(ParserState *ps);

// API used by lex
int Lexer_DoYYInput(ParserState *ps, char *buf, int size, FILE *in);
void Lexer_DoUserAction(ParserState *ps, char *text);
#define COLOR_ERRMSG COLOR_LIGHT_RED "error: " COLOR_WHITE
#define PSError(errmsg, ...) do { \
  if (++ps->errnum >= MAX_ERRORS) { \
    fprintf(stderr, "Too many errors.\n"); \
    exit(-1); \
  } \
  LineBuffer *linebuf = &(ps)->line; \
  if (!linebuf->print) { \
    lineinfo_t line = {linebuf->line, linebuf->row, linebuf->col}; \
    Parser_PrintError(ps, &line, COLOR_ERRMSG errmsg, ##__VA_ARGS__); \
    linebuf->print = 1; \
  } \
} while (0)

// Exported API

Symbol *Parser_New_Import(ParserState *ps, char *id, char *path);
void Parser_New_Vars(ParserState *ps, struct stmt *stmt);
void Parser_New_Func(ParserState *ps, struct stmt *stmt);
void Parser_New_ClassOrTrait(ParserState *ps, struct stmt *stmt);
void Parser_New_TypeAlias(ParserState *ps, struct stmt *stmt);
char *Parser_Get_FullPath(ParserState *ps, char *id);
void Parser_SetLine(ParserState *ps, struct expr *exp);

#ifdef __cplusplus
}
#endif
#endif /* _KOALA_PARSER_H_ */
