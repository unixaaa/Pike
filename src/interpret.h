/*\
||| This file a part of Pike, and is copyright by Fredrik Hubinette
||| Pike is distributed as GPL (General Public License)
||| See the files COPYING and DISCLAIMER for more information.
\*/

/*
 * $Id: interpret.h,v 1.25 1999/01/21 09:15:02 hubbe Exp $
 */
#ifndef INTERPRET_H
#define INTERPRET_H

#include "global.h"
#include "program.h"

#ifndef STRUCT_FRAME_DECLARED
#define STRUCT_FRAME_DECLARED
#endif
struct frame
{
  unsigned char *pc;
  struct frame *parent_frame;
  struct svalue *locals;
  struct svalue *expendible;
  struct object *current_object;
  struct inherit context;
  char *current_storage;
  INT32 args;
  INT32 fun;
  INT16 num_locals;
  INT16 num_args;
};

#ifdef PIKE_DEBUG
#define debug_check_stack() do{if(sp<evaluator_stack)fatal("Stack error.\n");}while(0)
#define check__positive(X,Y) if((X)<0) fatal(Y)
#include "error.h"
#else
#define check__positive(X,Y)
#define debug_check_stack() 
#endif

#define check_stack(X) do {			\
  if(sp - evaluator_stack + (X) >= stack_size)	\
    error("Stack overflow.\n");			\
  }while(0)

#define check_mark_stack(X) do {		\
  if(mark_sp - mark_stack + (X) >= stack_size)	\
    error("Mark stack overflow.\n");		\
  }while(0)

#define check_c_stack(X) do { 			\
  long x_= ((char *)&x_) + STACK_DIRECTION * (X) - stack_top ;	\
  x_*=STACK_DIRECTION;							\
  if(x_>0)								\
    error("C stack overflow.\n");					\
  }while(0)


#define pop_stack() do{ free_svalue(--sp); debug_check_stack(); }while(0)

#define pop_n_elems(X)						\
 do { int x_=(X); if(x_) { 					\
   check__positive(x_,"Popping negative number of args....\n");	\
   sp-=x_; debug_check_stack();					\
  free_svalues(sp,x_,BIT_MIXED);				\
 } } while (0)

#define push_program(P) do{ struct program *_=(P); debug_malloc_touch(_); sp->u.program=_; sp++->type=T_PROGRAM; }while(0)
#define push_int(I) do{ INT32 _=(I); sp->u.integer=_;sp->type=T_INT;sp++->subtype=NUMBER_NUMBER; }while(0)
#define push_mapping(M) do{ struct mapping *_=(M); debug_malloc_touch(_); sp->u.mapping=_; sp++->type=T_MAPPING; }while(0)
#define push_array(A) do{ struct array *_=(A); debug_malloc_touch(_); sp->u.array=_ ;sp++->type=T_ARRAY; }while(0)
#define push_multiset(L) do{ struct multiset *_=(L); debug_malloc_touch(_); sp->u.multiset=_; sp++->type=T_MULTISET; }while(0)
#define push_string(S) do{ struct pike_string *_=(S); debug_malloc_touch(_); sp->subtype=0; sp->u.string=_; sp++->type=T_STRING; }while(0)
#define push_object(O) do{ struct object  *_=(O); debug_malloc_touch(_); sp->u.object=_; sp++->type=T_OBJECT; }while(0)
#define push_float(F) do{ float _=(F); sp->u.float_number=_; sp++->type=T_FLOAT; }while(0)
#define push_text(T) push_string(make_shared_string((T)))
#define push_constant_text(T) do{ sp->subtype=0; MAKE_CONSTANT_SHARED_STRING(sp->u.string,T); sp++->type=T_STRING; }while(0)

#define ref_push_program(P) do{ struct program *_=(P); debug_malloc_touch(_); _->refs++; sp->u.program=_; sp++->type=T_PROGRAM; }while(0)
#define ref_push_mapping(M) do{ struct mapping *_=(M); debug_malloc_touch(_); _->refs++; sp->u.mapping=_; sp++->type=T_MAPPING; }while(0)
#define ref_push_array(A) do{ struct array *_=(A); debug_malloc_touch(_); _->refs++; sp->u.array=_ ;sp++->type=T_ARRAY; }while(0)
#define ref_push_multiset(L) do{ struct multiset *_=(L); debug_malloc_touch(_); _->refs++; sp->u.multiset=_; sp++->type=T_MULTISET; }while(0)
#define ref_push_string(S) do{ struct pike_string *_=(S); debug_malloc_touch(_); _->refs++; sp->subtype=0; sp->u.string=_; sp++->type=T_STRING; }while(0)
#define ref_push_object(O) do{ struct object  *_=(O); debug_malloc_touch(_); _->refs++; sp->u.object=_; sp++->type=T_OBJECT; }while(0)

#define push_svalue(S) do { struct svalue *_=(S); assign_svalue_no_free(sp,_); sp++; }while(0)

#define stack_dup() push_svalue(sp-1)
#define stack_swap() do { struct svalue _=sp[-1]; sp[-1]=sp[-2]; sp[-2]=_; } while(0)


enum apply_type
{
  APPLY_STACK, /* The function is the first argument */
  APPLY_SVALUE, /* arg1 points to an svalue containing the function */
   APPLY_LOW    /* arg1 is the object pointer,(int)arg2 the function */
};

#define apply_low(O,FUN,ARGS) \
  mega_apply(APPLY_LOW, (ARGS), (void*)(O),(void*)(FUN))

#define strict_apply_svalue(SVAL,ARGS) \
  mega_apply(APPLY_SVALUE, (ARGS), (void*)(SVAL),0)

#define APPLY_MASTER(FUN,ARGS) \
do{ \
  static int fun_,master_cnt=0; \
  struct object *master_ob=master(); \
  if(master_cnt != master_ob->prog->id) \
  { \
    fun_=find_identifier(FUN,master_ob->prog); \
    master_cnt = master_ob->prog->id; \
  } \
  apply_low(master_ob, fun_, ARGS); \
}while(0)

#define SAFE_APPLY_MASTER(FUN,ARGS) \
do{ \
  static int fun_,master_cnt=0; \
  struct object *master_ob=master(); \
  if(master_cnt != master_ob->prog->id) \
  { \
    fun_=find_identifier(FUN,master_ob->prog); \
    master_cnt = master_ob->prog->id; \
  } \
  safe_apply_low(master_ob, fun_, ARGS); \
}while(0)

#define check_threads_etc() \
  call_callback(& evaluator_callbacks, (void *)0)

#ifdef PIKE_DEBUG
#define fast_check_threads_etc(X) do { \
  static int div_; if(d_flag || !(div_++& ((1<<(X))-1))) check_threads_etc(); } while(0)

#else
#define fast_check_threads_etc(X) do { \
  static int div_; if(!(div_++& ((1<<(X))-1))) check_threads_etc(); } while(0)
#endif

/* Prototypes begin here */
void push_sp_mark(void);
int pop_sp_mark(void);
void init_interpreter(void);
void lvalue_to_svalue_no_free(struct svalue *to,struct svalue *lval);
void assign_lvalue(struct svalue *lval,struct svalue *from);
union anything *get_pointer_if_this_type(struct svalue *lval, TYPE_T t);
void print_return_value(void);
void reset_evaluator(void);
struct backlog;
void dump_backlog(void);
void mega_apply(enum apply_type type, INT32 args, void *arg1, void *arg2);
void f_call_function(INT32 args);
int apply_low_safe_and_stupid(struct object *o, INT32 offset);
void safe_apply_low(struct object *o,int fun,int args);
void safe_apply(struct object *o, char *fun ,INT32 args);
void apply_lfun(struct object *o, int fun, int args);
void apply_shared(struct object *o,
		  struct pike_string *fun,
		  int args);
void apply(struct object *o, char *fun, int args);
void apply_svalue(struct svalue *s, INT32 args);
void slow_check_stack(void);
void cleanup_interpret(void);
/* Prototypes end here */

extern struct svalue *sp;
extern struct svalue **mark_sp;
extern struct svalue *evaluator_stack;
extern struct svalue **mark_stack;
extern struct frame *fp; /* frame pointer */
extern char *stack_top;
extern int stack_size;
extern int evaluator_stack_malloced, mark_stack_malloced;
struct callback;
extern struct callback_list evaluator_callbacks;
extern void call_callback(struct callback_list *, void *);

#ifdef PROFILING
#ifdef HAVE_GETHRTIME
extern long long accounted_time;
extern long long time_base;
#endif
#endif

#endif

