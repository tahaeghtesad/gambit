
%{
//#
//# FILE: gcompile.y -- yaccer/compiler for the GCL
//#
//# $Id$
//#

#include <stdlib.h>
#include <ctype.h>
#include "basic.h"
#include "gambitio.h"
#include "gstring.h"
#include "rational.h"
#include "glist.h"
#include "ggrstack.h"
#include "gsm.h"
#include "gsminstr.h"

%}

%name GCLCompiler

%define MEMBERS   \
  int index; \
  gString input_text; \
  bool bval; \
  gInteger ival; \
  double dval; \
  gRational rval; \
  gString tval, formal;  \
  gList<Instruction *> program; \
  gGrowableStack<gString> formalstack; \
  gGrowableStack<int> labels, listlen; \
  gGrowableStack<char> matching; \
  GSM gsm; \
  bool quit; \
  \
  char nextchar(void); \
  void ungetchar(char c); \
  \
  void emit(Instruction *); \
  \
  int Parse(void); \
  void Execute(void);

%define CONSTRUCTOR_INIT     : index(0), gsm(256), formalstack(4), labels(4), \
                               listlen(4), matching(4), quit(false)

%token LOR
%token LAND
%token LNOT
%token EQU
%token NEQ
%token LTN
%token LEQ
%token GTN
%token GEQ
%token PLUS
%token MINUS
%token STAR
%token SLASH
%token ASSIGN
%token SEMI
%token LBRACK
%token DBLLBRACK
%token RBRACK
%token LBRACE
%token RBRACE
%token RARROW
%token LARROW
%token DBLARROW
%token COMMA
%token HASH

%token NAME
%token BOOLEAN
%token FLOAT
%token INTEGER
%token RATIONAL
%token PERCENT
%token DIV
%token LPAREN
%token RPAREN

%token IF
%token WHILE
%token FOR
%token QUIT

%token BOOLEAN
%token INTEGER
%token FLOAT
%token RATCONST
%token TEXT
%token NAME

%%

program:      statements   { emit(new Display); }
       |      error   { while (yylex() != EOF);  return 0; }

statements:   statement
          |   statements SEMI statement

statement:
         |    expression
         |    conditional
         |    whileloop
         |    forloop
         |    QUIT     { quit = true; emit(new Quit); }

conditional:  IF LBRACK expression COMMA 
              { emit(new NOT); emit(0);
                labels.Push(program.Length()); } statements 
              { emit(0);
		program[labels.Pop()] = new IfGoto(program.Length() + 1);
		labels.Push(program.Length());
	      }
              alternative RBRACK
              { emit(new NOP);
		program[labels.Pop()] = new Goto(program.Length());
              } 

alternative:   
           |  COMMA statements

whileloop:    WHILE LBRACK { labels.Push(program.Length() + 1); }
              expression { emit(new NOT); emit(0);
			   labels.Push(program.Length()); }
              COMMA statements RBRACK 
              { program[labels.Pop()] = new IfGoto(program.Length() + 2);
		emit(new Goto(labels.Pop()));
		emit(new NOP);
	      }

forloop:      FOR LBRACK exprlist COMMA  { labels.Push(program.Length() + 1); }
              expression COMMA   { emit(new NOT);  emit(0);
				   labels.Push(program.Length()); }
              exprlist COMMA statements RBRACK
                   { program[labels.Pop()] = new IfGoto(program.Length() + 2);
		     emit(new Goto(labels.Pop()));
		     emit(new NOP);
		   }

exprlist:     expression  { emit(new Pop); }
        |     exprlist SEMI expression  { emit(new Pop); }

expression:   E0
          |   E0 ASSIGN expression { emit(new Assign()); }
          ;

E0:           E1
  |           E0 LOR E1  { emit(new OR); }
  ;

E1:           E2
  |           E1 LAND E2  { emit(new AND); } 
  ;

E2:           E3
  |           LNOT E2     { emit(new NOT); }
  ;

E3:           E4       
  |           E3 EQU E4    { emit(new Equ); } 
  |           E3 NEQ E4    { emit(new Neq); }
  |           E3 LTN E4    { emit(new Ltn); }
  |           E3 LEQ E4    { emit(new Leq); }
  |           E3 GTN E4    { emit(new Gtn); } 
  |           E3 GEQ E4    { emit(new Geq); }
  ;

E4:           E5
  |           E4 PLUS E5   { emit(new Add); }
  |           E4 MINUS E5  { emit(new Sub); }
  ;

E5:           E6
  |           E5 STAR E6    { emit(new Mul); }
  |           E5 SLASH E6   { emit(new Div); }
  |           E5 PERCENT E6 { emit(new Mod); }
  |           E5 DIV E6     { emit(new IntDiv); }
  ;

E6:           PLUS E7
  |           MINUS E7      { emit(new Neg); }
  |           E7

E7:           E8
  |           E7 HASH E8   { emit(new Child); }
  |           E7 DBLLBRACK expression RBRACK RBRACK 
                 { emit(new Subscript); }
  ;

E8:           BOOLEAN       { emit(new Push<bool>(bval)); }
  |           INTEGER       { emit(new Push<gInteger>(ival)); }
  |           FLOAT         { emit(new Push<double>(dval)); }
  |           RATCONST      { emit(new Push<gRational>(rval)); }
  |           TEXT          { emit(new Push<gString>(tval)); }
  |           LPAREN expression RPAREN
  |           NAME          { emit(new PushRef(tval)); }
  |           function      { emit(new CallFunction()); }
  |           list          { emit(new PushList(listlen.Pop())); }
  ;

function:     NAME LBRACK  { emit(new InitCallFunction(tval)); } arglist RBRACK

arglist:
       |      unnamed_args
       |      unnamed_args COMMA named_args
       |      named_args

unnamed_args: unnamed_arg
            | unnamed_args COMMA unnamed_arg

unnamed_arg:  expression  { emit(new Bind()); }

named_args:   named_arg
          |   named_args COMMA named_arg

named_arg:    NAME RARROW { formalstack.Push(tval); } expression
                           { emit(new Bind(formalstack.Pop())); }
         |    NAME DBLARROW  { formalstack.Push(tval); } NAME
                           { emit(new PushRef(tval));
                             emit(new Bind(formalstack.Pop())); }

list:         LBRACE  { listlen.Push(0); } listels RBRACE
    |         LBRACE  { listlen.Push(0); }  RBRACE

listels:      listel
       |      listels COMMA listel

listel:       expression   { listlen.Push(listlen.Pop() + 1); }

%%


char GCLCompiler::nextchar(void)
{
  char c;

  gin >> c;

  return c;
}

void GCLCompiler::ungetchar(char c)
{
  gin.unget(c);
}

void GCLCompiler::yyerror(char *s)
{
  gerr << "Error: " << s << '\n';
}

const char CR = (char) 10;

int GCLCompiler::yylex(void)
{
  char c;

I_dont_believe_Im_doing_this:

  while (1)  {
    char d;
    do  {
      c = nextchar();
    }  while (isspace(c) && c != CR);
    if (c == '/')  {
      if ((d = nextchar()) == '/')  {
	while ((d = nextchar()) != CR);
	return EOF;
      }
      else if (d == '*')  {
	int done = 0;
	while (!done)  {
	  while ((d = nextchar()) != '*');
	  if ((d = nextchar()) == '/')  done = 1;
	}
      }
      else  {
	ungetchar(d);
	return SLASH;
      }
    }
    else
      break;
  }

  if (c == '\\')   {
    while (isspace(c = nextchar()) && c != CR);
    if (c == CR)
      goto I_dont_believe_Im_doing_this;
    else  {
      ungetchar(c);
      return '\\';
    }
  }

  if (isalpha(c))  {
    gString s(c);
    c = nextchar();
    while (isalpha(c) || isdigit(c) || c == '_')   {
      s += c;
      c = nextchar();
    }
    ungetchar(c);

    if (s == "True")   {
      bval = true;
      return BOOLEAN;
    }
    else if (s == "False")  {
      bval = false;
      return BOOLEAN;
    }
    else if (s == "AND")    return LAND;
    else if (s == "OR")     return LOR;
    else if (s == "NOT")    return LNOT;
    else if (s == "DIV")    return DIV;
    else if (s == "MOD")    return PERCENT;
    else if (s == "If")     return IF;
    else if (s == "While")  return WHILE;
    else if (s == "For")    return FOR;
    else if (s == "Quit")   return QUIT;
    else  { tval = s; return NAME; }
  }

  if (c == '"')   {
    tval = "";
    c = nextchar();
    while (c != '"')   {
      tval += c;
      c = nextchar();
    }
    return TEXT;
  }

  if (isdigit(c))   {
    gString s(c);
    c = nextchar();
    while (isdigit(c))   {
      s += c;
      c = nextchar();
    }

    if (c == '.')   {
      s += c;
      c = nextchar();
      while (isdigit(c))  {
	s += c;
	c = nextchar();
      }

      ungetchar(c);
      dval = atof((char *) s);
      return FLOAT;
    }
    else  {
      ungetchar(c);
      ival = atoi((char *) s);
      return INTEGER;
    }
  }

  switch (c)  {
    case ',':   return COMMA;
    case ';':   return SEMI;
    case '(':   matching.Push('(');  return LPAREN;
    case ')':   if (matching.Depth() > 0 && matching.Peek() == '(')
                  matching.Pop();
                return RPAREN;
    case '{':   matching.Push('{');  return LBRACE;
    case '}':   if (matching.Depth() > 0 && matching.Peek() == '{')
                  matching.Pop();
                return RBRACE;
    case '+':   return PLUS;
    case '-':   c = nextchar();
                if (c == '>')  return RARROW;
                else  { ungetchar(c);  return MINUS; }
    case '*':   return STAR;
    case '/':   return SLASH;
    case '%':   return PERCENT;
    case '=':   return EQU;
    case '#':   return HASH;
    case '[':   c = nextchar();
                if (c == '[')   {
		  matching.Push('[');
		  matching.Push('[');
		  return DBLLBRACK;
		}
                else   {
		  ungetchar(c);
		  matching.Push('[');
		  return LBRACK;
		}
    case ']':   if (matching.Depth() > 0 && matching.Peek() == '[')
                  matching.Pop();
                return RBRACK;
    case ':':   c = nextchar();
                if (c == '=')  return ASSIGN;
                else   { ungetchar(c);  return ':'; }  
    case '!':   c = nextchar();
                if (c == '=')  return NEQ;
		else   { ungetchar(c);  return LNOT; }
    case '<':   c = nextchar();
                if (c == '=')  return LEQ;
                else if (c != '-')  { ungetchar(c);  return LTN; }
                else   { 
		  c = nextchar();
		  if (c == '>')   return DBLARROW;
		  ungetchar(c);
		  return LARROW;
		}
    case '>':   c = nextchar();
                if (c == '=')  return GEQ;
                else   { ungetchar(c);  return GTN; }
    case '&':   c = nextchar();
                if (c == '&')  return LAND;
                else   { ungetchar(c);  return '&'; }
    case '|':   c = nextchar();
                if (c == '|')  return LOR;
                else   { ungetchar(c);  return '|'; }
    case CR:    if (matching.Depth())
                  goto I_dont_believe_Im_doing_this;
                return EOF;
    default:    return c;
  }
}

int GCLCompiler::Parse(void)
{
  matching.Flush();
  if (!yyparse())  {
    Execute();
    return 0;
  }
  else  {
    while (program.Length() > 0)   delete program.Remove(1);
    return 1;
  }
}


void GCLCompiler::emit(Instruction *op)
{
  program.Append(op);
}

void GCLCompiler::Execute(void)
{
#ifdef ASSEMBLY
  program.Dump(gout);   gout << '\n';
#endif   // ASSEMBLY
  gsm.Execute(program);
  gsm.Flush();
}

#include "gstack.imp"
#include "ggrstack.imp"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__

TEMPLATE class gStack<gString>;
TEMPLATE class gGrowableStack<gString>;

TEMPLATE class gStack<int>;
TEMPLATE class gGrowableStack<int>;

TEMPLATE class gStack<char>;
TEMPLATE class gGrowableStack<char>;
