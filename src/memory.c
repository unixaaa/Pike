/*\
||| This file a part of Pike, and is copyright by Fredrik Hubinette
||| Pike is distributed as GPL (General Public License)
||| See the files COPYING and DISCLAIMER for more information.
\*/
#include "global.h"
#include "memory.h"
#include "error.h"
#include "macros.h"

char *xalloc(SIZE_T size)
{
  char *ret;
  if(!size) return 0;

  ret=(char *)malloc(size);
  if(ret) return ret;

  error("Out of memory.\n");
  return 0;
}

#if 0
/*
 * This function may NOT change 'order'
 * This function is probably too slow, but it does not use
 * any extra space, so it could still be used if no extra space
 * could be allocated.
 */
void reorder(char *memory,INT32 nitems,INT32 size,INT32 *order)
{
  INT32 e, w;

  switch(size)
  {
  case 4:
    for(e=0; e<nitems-1; e++)
    {
      INT32 tmp;
      for(w = order[e]; w < e; w = order[w]);
      if(w == e) continue;
      tmp=((INT32 *)memory)[e];
      ((INT32 *)memory)[e]=((INT32 *)memory)[w];
      ((INT32 *)memory)[w]=tmp;
    }
    break;

  case 8:
    for(e=0; e<nitems-1; e++)
    {
      typedef struct eight_bytes { char t[8]; } eight_bytes;
      eight_bytes tmp;
      for(w = order[e]; w < e; w = order[w]);
      if(w == e) continue;
      tmp=((eight_bytes *)memory)[e];
      ((eight_bytes *)memory)[e]=((eight_bytes *)memory)[w];
      ((eight_bytes *)memory)[w]=tmp;
    }
    break;

  default:
    for(e=0; e<nitems-1; e++)
    {
      INT32 z;
      for(w = order[e]; w < e; w = order[w]);
      if(w == e) continue;
      
      for(z=0; z<size; z++)
      {
	char tmp;
	tmp=memory[e*size+z];
	memory[e*size+z]=memory[w*size+z];
	memory[e*size+z]=tmp;
      }
    }
  }
}

#endif

#if 0
/*
 * This function may NOT change 'order'
 * This function is probably too slow, but it does not use
 * any extra space, so it could still be used if no extra space
 * could be allocated.
 * (actually, it _does_ change 'order' but restores it afterwards)
 */
void reorder(char *memory, INT32 nitems, INT32 size,INT32 *order)
{
  INT32 e,d,c;

#ifdef DEBUG
  for(e=0;e<nitems;e++)
  {
    for(d=0;d<nitems;d++) if(order[d]==e) break;
    if(d==nitems)
      fatal("Missing number %ld in reorder() (nitems = %ld)\n",
	    (long)e,
	    (long)nitems);
  }
#endif


  switch(size)
  {
#ifdef DEBUG
    case 0:
      fatal("FEL FEL FEL\n");
      break;
#endif

    case 1:
    {
      char a,*m;
      m=memory;

      for(e=0;e<nitems;e++)
      {
        if(order[e]==e) continue;
        if(order[e]>=0)
        {
          a=memory[c=e];
          do
	  {
	    c=order[d=c];
	    m[d]=m[c];
	    order[d]=~c;
	  } while(c!=e);
          memory[d]=a;
        }
        order[e] =~ order[e];
      }
      break;
    }

    case 2:
    {
      typedef struct TMP2 { char t[2]; } tmp2;
      tmp2 a,*m;
      m=(tmp2 *)memory;
      
      for(e=0;e<nitems;e++)
      {
        if(order[e]==e) continue;
        if(order[e]>=0)
        {
          a=m[c=e];
          do
	  {
	    c=order[d=c];
	    m[d]=m[c];
	    order[d]=~c;
	  }
	  while(c!=e);
          m[d]=a;
        }
        order[e] =~ order[e];
      }
      break;
    }

    case 4:
    {
      typedef struct TMP4 { char t[4]; } tmp4;
      tmp4 a,*m;
      m=(tmp4 *)memory;
      
      for(e=0;e<nitems;e++)
      {
        if(order[e]==e) continue;
        if(order[e]>=0)
        {
          a=m[c=e];
          do
	  {
	    c=order[d=c];
	    m[d]=m[c];
	    order[d]=~c;
	  } while(c!=e);
          m[d]=a;
        }
        order[e] =~ order[e];
      }
      break;
    }

    case 8:
    {
      typedef struct TMP8 { char t[8]; } tmp8;
      tmp8 a,*m;
      m=(tmp8 *)memory;
      
      for(e=0;e<nitems;e++)
      {
        if(order[e]==e) continue;
        if(order[e]>=0)
        {
          a=m[c=e];
          do {
            c=order[d=c];
            m[d]=m[c];
            order[d]= ~c;
          }while(c!=e);
    
          m[d]=a;
        }
        order[e] =~ order[e];
      }
      break;
    }

    case 16:
    {
      typedef struct TMP16 { char t[16]; } tmp16;
      tmp16 a,*m;
      m=(tmp16 *)memory;
      
      for(e=0;e<nitems;e++)
      {
        if(order[e]==e) continue;
        if(order[e]>=0)
        {
          a=m[c=e];
          do {
            c=order[d=c];
            m[d]=m[c];
            order[d]= ~c;
          }while(c!=e);
    
          m[d]=a;
        }
        order[e] =~ order[e];
      }
      break;
    }

    default:
    {
      char *a;
    
      a=(char *)alloca(size);
    
      for(e=0;e<nitems;e++)
      {
        if(order[e]==e) continue;
        if(order[e]>=0)
        {
          MEMCPY(a, memory+e*size, size);
    
          c=e;
          do {
            c=order[d=c];
             MEMCPY(memory+d*size, memory+c*size, size);
            order[d]= ~c;
          }while(d!=e);
    
          MEMCPY(memory+d*size, a, size);
        }
        order[e] =~ order[e];
      }
    }
  }
}
#endif

#if 1
/*
 * This function may NOT change 'order'
 * This function is hopefully fast enough...
 */
void reorder(char *memory, INT32 nitems, INT32 size,INT32 *order)
{
  INT32 e;
  char *tmp;
  tmp=xalloc(size * nitems);

#define DOSIZE(X,Y)				\
 case X:					\
 {						\
  struct Y { char tt[X]; };			\
  struct Y *from=(struct Y *) memory;		\
  struct Y *to=(struct Y *) tmp;		\
  for(e=0;e<nitems;e++) to[e]=from[order[e]];	\
  break;					\
 }
  

  switch(size)
  {
    DOSIZE(1,TMP1)
    DOSIZE(2,TMP2)
    DOSIZE(4,TMP4)
    DOSIZE(8,TMP8)
    DOSIZE(16,TMP16)

  default:
    for(e=0;e<nitems;e++) MEMCPY(tmp+e*size, memory+order[e]*size, size);
  }

  MEMCPY(memory, tmp, size * nitems);
  free(tmp);
}
#endif

unsigned INT32 hashmem(const unsigned char *a,INT32 len,INT32 mlen)
{
  unsigned INT32 ret;

  ret=9248339*len;
  if(len<mlen) mlen=len;
  switch(mlen&7)
  {
    case 7: ret^=*(a++);
    case 6: ret^=(ret<<4)+*(a++);
    case 5: ret^=(ret<<7)+*(a++);
    case 4: ret^=(ret<<6)+*(a++);
    case 3: ret^=(ret<<3)+*(a++);
    case 2: ret^=(ret<<7)+*(a++);
    case 1: ret^=(ret<<5)+*(a++);
  }

#ifdef HANDLES_UNALIGNED_MEMORY_ACCESS
  {
    unsigned int *b;
    b=(unsigned int *)a;

    for(mlen>>=3;--mlen>=0;)
    {
      ret^=(ret<<7)+*(b++);
      ret^=(ret>>6)+*(b++);
    }
  }
#else
  for(mlen>>=3;--mlen>=0;)
  {
    ret^=(ret<<7)+((((((*(a++)<<3)+*(a++))<<4)+*(a++))<<5)+*(a++));
    ret^=(ret>>6)+((((((*(a++)<<3)+*(a++))<<4)+*(a++))<<5)+*(a++));
  }
#endif
  return ret;
}

unsigned INT32 hashstr(const unsigned char *str,INT32 maxn)
{
  unsigned INT32 ret,c;
  
  ret=str++[0];
  for(; maxn>=0; maxn--)
  {
    c=str++[0];
    if(!c) break;
    ret ^= ( ret << 4 ) + c ;
    ret &= 0x7fffffff;
  }

  return ret;
}


/*
 * a quick memory search function.
 * Written by Fredrik Hubinette (hubbe@lysator.liu.se)
 */
void init_memsearch(struct mem_searcher *s,
		    char *needle,
		    SIZE_T needlelen,
		    SIZE_T max_haystacklen)
{
  s->needle=needle;
  s->needlelen=needlelen;

  switch(needlelen)
  {
  case 0: s->method=no_search; break;
  case 1: s->method=use_memchr; break;
  case 2:
  case 3:
  case 4:
  case 5:
  case 6: s->method=memchr_and_memcmp; break;
  default:
    if(max_haystacklen <= needlelen + 64)
    {
      s->method=memchr_and_memcmp;
    }else{
      INT32 tmp, h;
      unsigned INT32 hsize, e, max;
      unsigned char *q;
      struct link *ptr;

      hsize=52+(max_haystacklen >> 7)  - (needlelen >> 8);
      max  =13+(max_haystacklen >> 4)  - (needlelen >> 5);

      if(hsize > NELEM(s->set))
      {
	hsize=NELEM(s->set);
      }else{
	for(e=8;e<hsize;e+=e);
	hsize=e;
      }
    
      for(e=0;e<hsize;e++) s->set[e]=0;
      hsize--;

      if(max > needlelen) max=needlelen;
      max=(max-sizeof(INT32)+1) & -sizeof(INT32);
      if(max > MEMSEARCH_LINKS) max=MEMSEARCH_LINKS;

      ptr=& s->links[0];

      q=(unsigned char *)needle;

#if BYTEORDER == 4321
      for(tmp=e=0;e<sizeof(INT32)-1;e++)
      {
	tmp<<=8;
	tmp|=*(q++);
      }
#endif

      for(e=0;e<max;e++)
      {
#if BYTEORDER == 4321
	tmp<<=8;
	tmp|=*(q++);
#else
	tmp=EXTRACT_INT(q);
	q++;
#endif
	h=tmp;
	h+=h>>7;
	h+=h>>17;
	h&=hsize;

	ptr->offset=e;
	ptr->key=tmp;
	ptr->next=s->set[h];
	s->set[h]=ptr;
	ptr++;
      }
      s->hsize=hsize;
      s->max=max;
      s->method=hubbe_search;
    }
  }
}
		    

char *memory_search(struct mem_searcher *s,
		    char *haystack,
		    SIZE_T haystacklen)
{
  if(s->needlelen > haystacklen) return 0;

  switch(s->method)
  {
  case no_search:
    return haystack;

  case use_memchr:
    return MEMCHR(haystack,s->needle[0],haystacklen);

  case memchr_and_memcmp:
    {
      char *end,c,*needle;
      SIZE_T needlelen;
      
      needle=s->needle;
      needlelen=s->needlelen;
      
      end=haystack + haystacklen - needlelen+1;
      c=needle[0];
      needle++;
      needlelen--;
      while((haystack=MEMCHR(haystack,c,end-haystack)))
	if(!MEMCMP(++haystack,needle,needlelen))
	  return haystack-1;

      return 0;
    }

  case hubbe_search:
    {
      INT32 tmp, h;
      char *q, *end;
      register struct link *ptr;
      
      end=haystack+haystacklen+1;
      q=haystack + s->max - sizeof(INT32);
      q=(char *)( ((long)q) & -sizeof(INT32));
      for(;q<end-sizeof(INT32)+1;q+=s->max)
      {
	h=tmp=*(INT32 *)q;
	
	h+=h>>7;
	h+=h>>17;
	h&=s->hsize;
	
	for(ptr=s->set[h];ptr;ptr=ptr->next)
	{
	  char *where;
	  
	  if(ptr->key != tmp) continue;
	  
	  where=q-ptr->offset;
	  if(where<haystack) continue;
	  if(where+s->needlelen>end) return 0;
	  
	  if(!MEMCMP(where,s->needle,s->needlelen))
	    return where;
	}
      }
    }
  }
  return 0;
}

char *my_memmem(char *needle,
		SIZE_T needlelen,
		char *haystack,
		SIZE_T haystacklen)
{
  struct mem_searcher tmp;
  init_memsearch(&tmp, needle, needlelen, haystacklen);
  return memory_search(&tmp, haystack, haystacklen);
}
