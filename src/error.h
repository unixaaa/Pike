/*\
||| This file a part of Pike, and is copyright by Fredrik Hubinette
||| Pike is distributed as GPL (General Public License)
||| See the files COPYING and DISCLAIMER for more information.
\*/
#ifndef ERROR_H
#define ERROR_H

#include "machine.h"

#ifdef HAVE_SETJMP_H
#include <setjmp.h>
#undef HAVE_SETJMP_H
#endif

#include <stdarg.h>

#include "svalue.h"


typedef void (*error_call)(void *);

struct frame;

typedef struct ONERROR
{
  struct ONERROR *previous;
  error_call func;
  void *arg;
} ONERROR;

typedef jmp_buf my_jmp_buf; /* Maybe I'll get less warnings like this */

typedef struct JMP_BUF
{
  struct JMP_BUF *previous;
  my_jmp_buf recovery;
  struct frame *fp;
  INT32 sp;
  INT32 mark_sp;
  ONERROR *onerror;
} JMP_BUF;

extern ONERROR *onerror_stack;
extern JMP_BUF *recoveries;
extern struct svalue throw_value;
extern char *automatic_fatal, *exit_on_error;

#define SETJMP(X) setjmp((init_recovery(&X)[0]))
#define UNSETJMP(X) recoveries=X.previous;

#define SET_ONERROR(X,Y,Z) \
  do{ \
     X.func=(error_call)(Y); \
     X.arg=(void *)(Z); \
     X.previous=onerror_stack; \
     onerror_stack=&X; \
  }while(0)

#define UNSET_ONERROR(X) onerror_stack=X.previous

my_jmp_buf *init_recovery(JMP_BUF *r);
int fix_recovery(int i, JMP_BUF *r);
void throw() ATTRIBUTE((noreturn));
void va_error(char *fmt, va_list args) ATTRIBUTE((noreturn));
void error(char *fmt,...) ATTRIBUTE((noreturn,format (printf, 1, 2)));
void fatal(char *fmt, ...) ATTRIBUTE((noreturn,format (printf, 1, 2)));

#endif



