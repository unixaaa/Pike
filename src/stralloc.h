/*\
||| This file a part of Pike, and is copyright by Fredrik Hubinette
||| Pike is distributed as GPL (General Public License)
||| See the files COPYING and DISCLAIMER for more information.
\*/
#ifndef STRALLOC_H
#define STRALLOC_H
#include "global.h"

#define STRINGS_ARE_SHARED

#ifndef STRUCT_PIKE_STRING_DECLARED
#define STRUCT_PIKE_STRING_DECLARED
#endif
struct pike_string
{
  INT32 refs;
  INT32 len;
  unsigned INT32 hval;
  struct pike_string *next; 
  char str[1];
};

#ifdef DEBUG
struct pike_string *debug_findstring(const struct pike_string *foo);
#endif

#define free_string(s) do{ struct pike_string *_=(s); debug_malloc_touch(_); if(--_->refs<=0) really_free_string(_); }while(0)

#define my_hash_string(X) ((unsigned long)debug_malloc_pass(X))
#define my_order_strcmp(X,Y) ((char *)(X)-(char *)(Y))
#define is_same_string(X,Y) ((X)==(Y))

#ifdef DEBUG_MALLOC

#define reference_shared_string(s) do { struct pike_string *S_=(s); debug_malloc_touch(S_); S_->refs++; }while(0)
#define copy_shared_string(to,s) do { struct pike_string *S_=(to)=(s); debug_malloc_touch(S_); S_->refs++; }while(0)

#else

#define reference_shared_string(s) (s)->refs++
#define copy_shared_string(to,s) ((to)=(s))->refs++


#endif


/* Prototypes begin here */
struct pike_string *binary_findstring(const char *foo, INT32 l);
struct pike_string *findstring(const char *foo);
struct pike_string *debug_begin_shared_string(int len);
struct pike_string *end_shared_string(struct pike_string *s);
struct pike_string *debug_make_shared_binary_string(const char *str,int len);
struct pike_string *debug_make_shared_string(const char *str);
void unlink_pike_string(struct pike_string *s);
void really_free_string(struct pike_string *s);
struct pike_string *add_string_status(int verbose);
void check_string(struct pike_string *s);
void verify_shared_strings_tables(void);
struct pike_string *debug_findstring(const struct pike_string *foo);
void dump_stralloc_strings(void);
int low_quick_binary_strcmp(char *a,INT32 alen,
			    char *b,INT32 blen);
int my_quick_strcmp(struct pike_string *a,struct pike_string *b);
int my_strcmp(struct pike_string *a,struct pike_string *b);
struct pike_string *realloc_unlinked_string(struct pike_string *a, INT32 size);
struct pike_string *realloc_shared_string(struct pike_string *a, INT32 size);
struct pike_string *add_shared_strings(struct pike_string *a,
					 struct pike_string *b);
struct pike_string *string_replace(struct pike_string *str,
				     struct pike_string *del,
				     struct pike_string *to);
void init_shared_string_table(void);
void cleanup_shared_string_table(void);
void count_memory_in_strings(INT32 *num, INT32 *size);
void gc_mark_all_strings(void);
/* Prototypes end here */

#ifdef DEBUG_MALLOC
#define make_shared_string(X) \
 ((struct pike_string *)debug_malloc_update_location(debug_make_shared_string(X),__FILE__,__LINE__))
#define make_shared_binary_string(X,Y) \
 ((struct pike_string *)debug_malloc_update_location(debug_make_shared_binary_string((X),(Y)),__FILE__,__LINE__))
#define begin_shared_string(X) \
 ((struct pike_string *)debug_malloc_update_location(debug_begin_shared_string(X),__FILE__,__LINE__))
#else
#define make_shared_string debug_make_shared_string
#define make_shared_binary_string debug_make_shared_binary_string
#define begin_shared_string debug_begin_shared_string
#endif

#endif /* STRALLOC_H */
