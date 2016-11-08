/*
|| This file is part of Pike. For copyright information see COPYRIGHT.
|| Pike is distributed under GPL, LGPL and MPL. See the file COPYING
|| for more information.
*/

/*
  Pike Sprintf v2.0 By Fredrik Hubinette (Profezzorn@nannymud)
  Should be reasonably compatible and somewhat faster than v1.05+ of Lynscar's
  sprintf. It requires the buffering function provided in dynamic_buffer.c
   Fail-safe memory-leak-protection is implemented through a stack that can
   be deallocated at any time. If something fails horribly this stack will be
  deallocated at next call of sprintf. Most operators doesn't need this
  feature though as they allocate their buffers with alloca() or simply use
  pointers into other strings.
  It also has a lot more features.

 Ideas yet to be implemented:
   Line-break with fill? Lower-case? Upper case? Capitalize?
   Replace? Justify on decimal point?
   Change european/american decimal notation?
   nroff-format? Crack root password and erase all disks?
   Print-optimize? (space to tab, strip trailing spaces)
   '>' Kill this field in all lines but the first one

 Examples:

   A short 'people' function (without sort)

   sprintf("%{%-14s %2d %-30s %s\n%}\n",map(users(),lambda(object x)
       {
           return ({x->query_real_name(),
		x->query_level(),
		query_ip_name(x),
		file_name(environment(x))
	      });
       }))


   an 'ls'
   sprintf("%-#*{%s\n%}\n",width,get_dir(dir));

   debug-dump
   sprintf("%78*O",width,foo);

   A newspaper
   sprintf("%-=*s %-=*s\n",width/2,article1,width/2,article2);

   A 'dotted-line' pricelist row
   sprintf("%'.'-10s.%'.'4d\n",item,cost);

*/

/*! @decl string sprintf(strict_sprintf_format format, sprintf_args ... args)
 *!
 *!   Print formated output to string.
 *!
 *!   The @[format] string is a string containing a description of how to
 *!   output the data in @[args]. This string should generally speaking
 *!   have one @tt{%@i{<modifiers>@}@i{<operator>@}@} format specifier
 *!   (examples: @tt{%s@}, @tt{%0d@}, @tt{%-=20s@}) for each of the arguments.
 *!
 *!   The following modifiers are supported:
 *!   @int
 *!     @value '0'
 *!       Zero pad numbers (implies right justification).
 *!     @value '!'
 *!       Toggle truncation.
 *!     @value ' '
 *!       Pad positive integers with a space.
 *!     @value '+'
 *!       Pad positive integers with a plus sign.
 *!     @value '-'
 *!       Left adjust within field size (default is right).
 *!     @value '|'
 *!       Centered within field size.
 *!     @value '='
 *!       Column mode if strings are greater than field size. Breaks
 *!       between words (possibly skipping or adding spaces). Can not be
 *!       used together with @expr{'/'@}.
 *!     @value '/'
 *!       Column mode with rough line break (break at exactly field size
 *!       instead of between words). Can not be used together with @expr{'='@}.
 *!     @value '#'
 *!       Table mode, print a list of @expr{'\n'@} separated words
 *!       (top-to-bottom order).
 *!     @value '$'
 *!       Inverse table mode (left-to-right order).
 *!     @value 'n'
 *!     @value ':n'
 *!       (Where n is a number or *) field width specifier.
 *!     @value '.n'
 *!       Precision specifier.
 *!     @value ';n'
 *!       Column width specifier.
 *!     @value '*'
 *!       If n is a @tt{*@} then next argument is used for precision/field
 *!       size. The argument may either be an integer, or a modifier mapping
 *!       as received by @[lfun::_sprintf()]:
 *!       @mapping
 *!         @member int|void "precision"
 *!           Precision.
 *!         @member int(0..)|void "width"
 *!           Field width.
 *!         @member int(0..1)|void "flag_left"
 *!           Indicates that the output should be left-aligned.
 *!         @member int(0..)|void "indent"
 *!           Indentation level in @tt{%O@}-mode.
 *!       @endmapping
 *!     @value "'"
 *!       Set a pad string. @tt{'@} cannot be a part of the pad string (yet).
 *!     @value '~'
 *!       Get pad string from argument list.
 *!     @value '<'
 *!       Use same argument again.
 *!     @value '^'
 *!       Repeat this on every line produced.
 *!     @value '@'
 *!       Repeat this format for each element in the argument array.
 *!     @value '>'
 *!       Put the string at the bottom end of column instead of top.
 *!     @value '_'
 *!       Set width to the length of data.
 *!     @value '[n]'
 *!       Select argument number @tt{@i{n@}@}. Use @tt{*@} to use the next
 *!       argument as selector. The arguments are numbered starting from
 *!       @expr{0@} (zero) for the first argument after the @[format].
 *!       Note that this only affects where the current operand is fetched.
 *!   @endint
 *!
 *!   The following operators are supported:
 *!   @int
 *!     @value '%'
 *!       Percent.
 *!     @value 'b'
 *!       Signed binary integer.
 *!     @value 'd'
 *!       Signed decimal integer.
 *!     @value 'u'
 *!       Unsigned decimal integer.
 *!     @value 'o'
 *!       Signed octal integer.
 *!     @value 'x'
 *!       Lowercase signed hexadecimal integer.
 *!     @value 'X'
 *!       Uppercase signed hexadecimal integer.
 *!     @value 'c'
 *!       Character. If a fieldsize has been specified this will output
 *!       the low-order bytes of the integer in network (big endian) byte
 *!       order. To get little endian byte order, negate the field size.
 *!     @value 'f'
 *!       Float. (Locale dependent formatting.)
 *!     @value 'g'
 *!       Heuristically chosen representation of float.
 *!       (Locale dependent formatting.)
 *!     @value 'G'
 *!       Like @tt{%g@}, but uses uppercase @tt{E@} for exponent.
 *!     @value 'e'
 *!       Exponential notation float. (Locale dependent output.)
 *!     @value 'E'
 *!       Like @tt{%e@}, but uses uppercase @tt{E@} for exponent.
 *!     @value 'F'
 *!       Binary IEEE representation of float (@tt{%4F@} gives
 *!       single precision, @tt{%8F@} gives double precision)
 *!       in network (big endian) byte order. To get little endian
 *!       byte order, negate the field size.
 *!     @value 's'
 *!       String.
 *!     @value 'q'
 *!       Quoted string. Escapes all control and non-8-bit characters,
 *!       as well as the quote characters @tt{'\\'@} and @tt{'\"'@}.
 *!     @value 'O'
 *!       Any value, debug style. Do not rely on the exact formatting;
 *!       how the result looks can vary depending on locale, phase of
 *!       the moon or anything else the @[lfun::_sprintf()] method
 *!       implementor wanted for debugging.
 *!     @value 'H'
 *!       Binary Hollerith string. Equivalent to @expr{sprintf("%c%s",
 *!       strlen(str), str)@}. Arguments (such as width etc) adjust the
 *!       length-part of the format. Requires 8-bit strings.
 *!     @value 'n'
 *!       No argument. Same as @expr{"%s"@} with an empty string as argument.
 *!       Note: Does take an argument array (but ignores its content)
 *!       if the modifier @expr{'@@'@} is active.
 *!     @value 't'
 *!       Type of the argument.
 *!     @value '{'
 *!     @value '}'
 *!       Perform the enclosed format for every element of the argument array.
 *!   @endint
 *!
 *!   Most modifiers and operators are combinable in any fashion, but some
 *!   combinations may render strange results.
 *!
 *!   If an argument is an object that implements @[lfun::_sprintf()], that
 *!   callback will be called with the operator as the first argument, and
 *!   the current modifiers as the second. The callback is expected to return
 *!   a string.
 *!
 *! @note
 *!   sprintf-style formatting is applied by many formatting functions, such
 *!   @[write()] and @[werror()]. It is also possible to get sprintf-style
 *!   compile-time argument checking by using the type-attributes
 *!   @[sprintf_format] or @[strict_sprintf_format] in combination
 *!   with @[sprintf_args].
 *!
 *! @note
 *!   The @expr{'q'@} operator was added in Pike 7.7.
 *!
 *! @note
 *!   Support for specifying modifiers via a mapping was added in Pike 7.8.
 *!   This support can be tested for with the constant
 *!   @[String.__HAVE_SPRINTF_STAR_MAPPING__].
 *!
 *! @note
 *!   Support for specifying little endian byte order to @expr{'F'@}
 *!   was added in Pike 7.8. This support can be tested for with the
 *!   constant @[String.__HAVE_SPRINTF_NEGATIVE_F__].
 *!
 *! @example
 *! @code
 *! Pike v7.8 release 263 running Hilfe v3.5 (Incremental Pike Frontend)
 *! > sprintf("The unicode character %c has character code %04X.", 'A', 'A');
 *! (1) Result: "The unicode character A has character code 0041."
 *! > sprintf("#%@@02X is the HTML code for purple.", Image.Color.purple->rgb());
 *! (2) Result: "#A020F0 is the HTML code for purple."
 *! > int n=4711;
 *! > sprintf("%d = hexadecimal %x = octal %o = %b binary", n, n, n, n);
 *! (3) Result: "4711 = hexadecimal 1267 = octal 11147 = 1001001100111 binary"
 *! > write(#"Formatting examples:
 *! Left adjusted  [%-10d]
 *! Centered       [%|10d]
 *! Right adjusted [%10d]
 *! Zero padded    [%010d]
 *! ", n, n, n, n);
 *! Formatting examples:
 *! Left adjusted  [4711      ]
 *! Centered       [   4711   ]
 *! Right adjusted [      4711]
 *! Zero padded    [0000004711]
 *! (5) Result: 142
 *! int screen_width=70;
 *! > write("%-=*s\n", screen_width,
 *! >> "This will wordwrap the specified string within the "+
 *! >> "specified field size, this is useful say, if you let "+
 *! >> "users specify their screen size, then the room "+
 *! >> "descriptions will automagically word-wrap as appropriate.\n"+
 *! >> "slosh-n's will of course force a new-line when needed.\n");
 *! This will wordwrap the specified string within the specified field
 *! size, this is useful say, if you let users specify their screen size,
 *! then the room descriptions will automagically word-wrap as
 *! appropriate.
 *! slosh-n's will of course force a new-line when needed.
 *! (6) Result: 355
 *! > write("%-=*s %-=*s\n", screen_width/2,
 *! >> "Two columns next to each other (any number of columns will "+
 *! >> "of course work) independantly word-wrapped, can be useful.",
 *! >> screen_width/2-1,
 *! >> "The - is to specify justification, this is in addherence "+
 *! >> "to std sprintf which defaults to right-justification, "+
 *! >> "this version also supports centre and right justification.");
 *! Two columns next to each other (any The - is to specify justification,
 *! number of columns will of course    this is in addherence to std
 *! work) independantly word-wrapped,   sprintf which defaults to
 *! can be useful.                      right-justification, this version
 *!                                     also supports centre and right
 *!                                     justification.
 *! (7) Result: 426
 *! > write("%-$*s\n", screen_width,
 *! >> "Given a\nlist of\nslosh-n\nseparated\n'words',\nthis option\n"+
 *! >> "creates a\ntable out\nof them\nthe number of\ncolumns\n"+
 *! >> "be forced\nby specifying a\npresision.\nThe most obvious\n"+
 *! >> "use is for\nformatted\nls output.");
 *! Given a          list of          slosh-n
 *! separated        'words',         this option
 *! creates a        table out        of them
 *! the number of    columns          be forced
 *! by specifying a  presision.       The most obvious
 *! use is for       formatted        ls output.
 *! (8) Result: 312
 *! > write("%-#*s\n", screen_width,
 *! >> "Given a\nlist of\nslosh-n\nseparated\n'words',\nthis option\n"+
 *! >> "creates a\ntable out\nof them\nthe number of\ncolumns\n"+
 *! >> "be forced\nby specifying a\npresision.\nThe most obvious\n"+
 *! >> "use is for\nformatted\nls output.");
 *! Given a          creates a        by specifying a
 *! list of          table out        presision.
 *! slosh-n          of them          The most obvious
 *! separated        the number of    use is for
 *! 'words',         columns          formatted
 *! this option      be forced        ls output.
 *! (9) Result: 312
 *! > sample = ([ "align":"left", "valign":"middle" ]);
 *! (10) Result: ([ @xml{/@}* 2 elements *@xml{/@}
 *!          "align":"left",
 *!          "valign":"middle"
 *!        ])
 *! > write("<td%{ %s='%s'%}>\n", (array)sample);
 *! <td valign='middle' align='left'>
 *! (11) Result: 34
 *! >  write("Of course all the simple printf options "+
 *! >> "are supported:\n %s: %d %x %o %c\n",
 *! >> "65 as decimal, hex, octal and a char",
 *! >> 65, 65, 65, 65);
 *! Of course all the simple printf options are supported:
 *!  65 as decimal, hex, octal and a char: 65 41 101 A
 *! (12) Result: 106
 *! > write("%[0]d, %[0]x, %[0]X, %[0]o, %[0]c\n", 75);
 *! 75, 4b, 4B, 113, K
 *! (13) Result: 19
 *! > write("%|*s\n",screen_width, "THE END");
 *!                                THE END
 *! (14) Result: 71
 *! > write("%|*s\n", ([ "width":screen_width ]), "ALTERNATIVE END");
 *!                            ALTERNATIVE END
 *! (15) Result: 71
 *! @endcode
 *!
 *! @seealso
 *!   @[lfun::_sprintf()], @[strict_sprintf_format], @[sprintf_format],
 *!   @[sprintf_args], @[String.__HAVE_SPRINTF_STAR_MAPPING__],
 *!   @[String.__HAVE_SPRINTF_NEGATIVE_F__].
 */
#include "global.h"
#include "pike_error.h"
#include "array.h"
#include "svalue.h"
#include "stralloc.h"
#include "dynamic_buffer.h"
#include "pike_types.h"
#include "constants.h"
#include "interpret.h"
#include "pike_memory.h"
#include "pike_macros.h"
#include "object.h"
#include "bignum.h"
#include "mapping.h"
#include "builtin_functions.h"
#include "operators.h"
#include "opcodes.h"
#include "cyclic.h"
#include "module.h"
#include "pike_float.h"
#include "stack_allocator.h"
#include <ctype.h>
#include "module_support.h"
#include "bitvector.h"

#define RETURN_SHARED_STRING

#define SPRINTF_UNDECIDED -1027

struct format_info
{
  char *fi_free_string;
  struct pike_string *to_free_string;
  PCHARP b;		/* Buffer to format */
  ptrdiff_t len;	/* Buffer length */
  ptrdiff_t width;	/* Field width (handled by fix_field) */
  int precision;	/* Field precision (handled by 'boduxXefgEGsq') */
  PCHARP pad_string;	/* Padding (handled by fix_field) */
  ptrdiff_t pad_length;	/* Padding length (handled by fix_field) */
  int column_entries;	/* Number of column entries (handled by do_one) */
  short flags;
  char pos_pad;		/* Positive padding (handled by fix_field) */
  int column_width;
  ptrdiff_t column_modulo;
};

struct format_stack
{
  struct stack_allocator a;
  struct format_info *fsp;
  struct format_info *format_info_stack;
  size_t size;
};

#define FIELD_LEFT	(1<<0)
#define FIELD_CENTER	(1<<1)
#define PAD_POSITIVE	(1<<2)
#define LINEBREAK	(1<<3)
#define COLUMN_MODE	(1<<4)
#define ZERO_PAD	(1<<5)
#define ROUGH_LINEBREAK	(1<<6)
#define DO_TRUNC	(1<<7)
#define REPEAT		(1<<8)
#define SNURKEL		(1<<9)
#define INVERSE_COLUMN_MODE (1<<10)
#define MULTI_LINE	(1<<11)
#define WIDTH_OF_DATA	(1<<12)
#define MULTI_LINE_BREAK (1<<13)

#define MULTILINE (LINEBREAK | COLUMN_MODE | ROUGH_LINEBREAK | \
		   INVERSE_COLUMN_MODE | MULTI_LINE | REPEAT)


/* Generate binary IEEE strings on a machine which uses a different kind
   of floating point internally */

#if !defined(NEED_CUSTOM_IEEE) && (SIZEOF_FLOAT_TYPE > 4)
#define NEED_CUSTOM_IEEE
#endif

#ifdef NEED_CUSTOM_IEEE

#ifndef HAVE_FPCLASS
#ifdef HAVE_FP_CLASS_D
#define fpclass fp_class_d
#define FP_NZERO FP_NEG_ZERO
#define FP_PZERO FP_POS_ZERO
#define FP_NINF FP_NEG_INF
#define FP_PINF FP_POS_INF
#define FP_NNORM FP_NEG_NORM
#define FP_PNORM FP_POS_NORM
#define FP_NDENORM FP_NEG_DENORM
#define FP_PDENORM FP_POS_DENORM
#define HAVE_FPCLASS
#endif
#endif

static void low_write_IEEE_float(char *b, double d, int sz)
{
  int maxexp;
  unsigned INT32 maxf;
  int s = 0, e = -1;
  unsigned INT32 f = 0, extra_f=0;

  if(sz==4) {
    maxexp = 255;
    maxf   = 0x7fffff;
  } else {
    maxexp = 2047;
    maxf   = 0x0fffff; /* This is just the high part of the mantissa... */
  }

#ifdef HAVE_FPCLASS
  switch(fpclass(d)) {
  case FP_SNAN:
    e = maxexp; f = 2; break;
  case FP_QNAN:
    e = maxexp; f = maxf; break;
  case FP_NINF:
    s = 1; /* FALLTHRU */
  case FP_PINF:
    e = maxexp; break;
  case FP_NZERO:
    s = 1; /* FALLTHRU */
  case FP_PZERO:
    e = 0; break;
  case FP_NNORM:
  case FP_NDENORM:
    s = 1; d = fabs(d); break;
  case FP_PNORM:
  case FP_PDENORM:
    break;
  default:
    if(d<0.0) {
      s = 1;
      d = fabs(d);
    }
    break;
  }
#else
  if(PIKE_ISINF(d))
    e = maxexp;
  else if(PIKE_ISNAN(d))
  {
    e = maxexp;
    f = maxf;
  }
#ifdef HAVE_ISZERO
  else if(iszero(d))
    e = 0;
#endif

#ifdef HAVE_SIGNBIT
  if((s = signbit(d)))
    d = fabs(d);
#else
  if(d<0.0) {
    s = 1;
    d = fabs(d);
  }
#endif
#endif

  if(e<0) {
    d = frexp(d, &e);
    if(d == 1.0) {
      d=0.5;
      e++;
    }
    if(d == 0.0) {
      e = 0;
      f = 0;
    } else if(sz==4) {
      e += 126;
      d *= 16777216.0;
      if(e<=0) {
	d = ldexp(d, e-1);
	e = 0;
      }
      f = ((INT32)floor(d))&maxf;
    } else {
      double d2;
      e += 1022;
      d *= 2097152.0;
      if(e<=0) {
	d = ldexp(d, e-1);
	e = 0;
      }
      d2 = floor(d);
      f = ((INT32)d2)&maxf;
      d -= d2;
      d += 1.0;
      extra_f = (unsigned INT32)(floor(d * 4294967296.0)-4294967296.0);
    }

    if(e>=maxexp) {
      e = maxexp;
      f = extra_f = 0;
    }
  }

  if(sz==4) {
    b[0] = (s? 128:0)|((e&0xfe)>>1);
    b[1] = ((e&1)<<7)|((f&0x7f0000)>>16);
    b[2] = (f&0xff00)>>8;
    b[3] = f&0xff;
  } else {
    b[0] = (s? 128:0)|((e&0x7f0)>>4);
    b[1] = ((e&0xf)<<4)|((f&0x0f0000)>>16);
    b[2] = (f&0xff00)>>8;
    b[3] = f&0xff;
    b[4] = (extra_f&0xff000000)>>24;
    b[5] = (extra_f&0xff0000)>>16;
    b[6] = (extra_f&0xff00)>>8;
    b[7] = extra_f&0xff;
  }
}
#endif

/* Position a string inside a field with fill */

static void fix_field(struct string_builder *r,
			     PCHARP b,
			     ptrdiff_t len,
			     int flags,
			     ptrdiff_t width,
			     PCHARP pad_string,
			     ptrdiff_t pad_length,
			     char pos_pad)
{
  ptrdiff_t e;
  if(!width || width==SPRINTF_UNDECIDED)
  {
    if(pos_pad && EXTRACT_PCHARP(b)!='-') string_builder_putchar(r,pos_pad);
    string_builder_append(r,b,len);
    return;
  }

  if(!(flags & DO_TRUNC) && len+(pos_pad && EXTRACT_PCHARP(b)!='-')>=width)
  {
    if(pos_pad && EXTRACT_PCHARP(b)!='-') string_builder_putchar(r,pos_pad);
    string_builder_append(r,b,len);
    return;
  }

  if (flags & (ZERO_PAD|FIELD_CENTER|FIELD_LEFT)) {
    /* Some flag is set. */

    if(flags & ZERO_PAD)		/* zero pad is kind of special... */
    {
      if(EXTRACT_PCHARP(b)=='-')
      {
  	string_builder_putchar(r,'-');
  	INC_PCHARP(b,1);
  	len--;
  	width--;
      }else{
  	if(pos_pad)
  	{
  	  string_builder_putchar(r,pos_pad);
  	  width--;
  	}
      }
#if 1
      string_builder_fill(r,width-len,MKPCHARP("0",0),1,0);
#else
      for(;width>len;width--) string_builder_putchar(r,'0');
#endif
      string_builder_append(r,b,len);
      return;
    }

    if (flags & (FIELD_CENTER|FIELD_LEFT)) {
      ptrdiff_t d=0;

      if(flags & FIELD_CENTER)
      {
  	e=len;
  	if(pos_pad && EXTRACT_PCHARP(b)!='-') e++;
  	e=(width-e)/2;
  	if(e>0)
  	{
  	  string_builder_fill(r, e, pad_string, pad_length, 0);
  	  width-=e;
  	}
      }

      /* Left adjust */
      if(pos_pad && EXTRACT_PCHARP(b)!='-')
      {
  	string_builder_putchar(r,pos_pad);
  	width--;
  	d++;
      }

#if 1
      len = MINIMUM(width, len);
      if (len) {
        d += len;
	string_builder_append(r, b, len);
	width -= len;
      }
#else /* 0 */
      d+=MINIMUM(width,len);
      while(len && width)
      {
  	string_builder_putchar(r,EXTRACT_PCHARP(b));
  	INC_PCHARP(b,1);
  	len--;
  	width--;
      }
#endif /* 1 */

      if(width>0)
      {
	d%=pad_length;
	string_builder_fill(r, width, pad_string, pad_length, d);
      }

      return;
    }
  }

  /* Right-justification */

  if(pos_pad && EXTRACT_PCHARP(b)!='-') len++;
  e=width-len;
  if(e>0)
  {
    string_builder_fill(r, e, pad_string, pad_length, 0);
    width-=e;
  }

  if(pos_pad && EXTRACT_PCHARP(b)!='-' && len==width)
  {
    string_builder_putchar(r,pos_pad);
    len--;
    width--;
  }
  INC_PCHARP(b,len-width);
  string_builder_append(r,b,width);
}

static void free_sprintf_strings(struct format_stack *fs)
{
  struct format_info * f, * fend;
  f = fs->fsp;
  fend = fs->format_info_stack;
  for(;f>=fend;f--)
  {
    if(f->fi_free_string) free(f->fi_free_string);
#ifdef PIKE_DEBUG
    f->fi_free_string=0;
#endif
    if(f->to_free_string) free_string(f->to_free_string);
#ifdef PIKE_DEBUG
    f->to_free_string=0;
#endif
  }
  fs->fsp = f;
}

static void sprintf_error(struct format_stack *fs,
			  char *s,...) ATTRIBUTE((noinline,noreturn,format (printf, 2, 3)));
static void sprintf_error(struct format_stack *fs,
			  char *s,...)
{
  char buf[100];
  va_list args;
  va_start(args,s);
  free_sprintf_strings(fs);

  sprintf(buf,"sprintf: %s",s);
  va_error(buf,args);
  va_end(args);
}

static void mapping_to_format_info(struct mapping * m, struct format_info * fsp, int * indent) {
  struct svalue *sval;

  if ((sval = simple_mapping_string_lookup(m, "precision")) &&
      (TYPEOF(*sval) == T_INT)) {
    fsp->precision = sval->u.integer;
  }
  if ((sval = simple_mapping_string_lookup(m, "width")) &&
      (TYPEOF(*sval) == T_INT) && (sval->u.integer >= 0)) {
    fsp->width = sval->u.integer;
  }
  if ((sval = simple_mapping_string_lookup(m, "flag_left")) &&
      (TYPEOF(*sval) == T_INT)) {
    if (sval->u.integer) {
      fsp->flags |= FIELD_LEFT;
    } else {
      fsp->flags &= ~FIELD_LEFT;
    }
  }
  if ((sval = simple_mapping_string_lookup(m, "indent")) &&
      (TYPEOF(*sval) == T_INT) && (sval->u.integer >= 0)) {
    *indent = sval->u.integer;
  }
}

static int call_object_sprintf(int mode, struct object * o, ptrdiff_t fun, struct format_stack * fs) {
  int n = 0;
  struct format_info *fsp = fs->fsp;
  DECLARE_CYCLIC();

  if (BEGIN_CYCLIC(o, fun)) {
      END_CYCLIC();
      return 0;
  }

  push_int(mode);

  if (fsp->precision!=SPRINTF_UNDECIDED)
  {
     push_constant_text("precision");
     push_int(fsp->precision);
     n+=2;
  }
  if (fsp->width!=SPRINTF_UNDECIDED)
  {
     push_constant_text("width");
     push_int64(fsp->width);
     n+=2;
  }
  if ((fsp->flags&FIELD_LEFT))
  {
     push_constant_text("flag_left");
     push_int(1);
     n+=2;
  }
  f_aggregate_mapping(n);

  SET_CYCLIC_RET(1);

  apply_low(o, fun, 2);

  END_CYCLIC();

  if(TYPEOF(Pike_sp[-1]) == T_STRING)
  {
    DO_IF_DEBUG( if(fsp->to_free_string)
                 Pike_fatal("OOps in sprintfn"); )
    fsp->to_free_string = (--Pike_sp)->u.string;

    fsp->b = MKPCHARP_STR(fsp->to_free_string);
    fsp->len = fsp->to_free_string->len;
    return 1;
  }

  if (!SAFE_IS_ZERO(Pike_sp-1))
    sprintf_error(fs,"(object) returned illegal value from _sprintf()\n");
  pop_stack();

  return 0;
}

/* This is called once for every '%' on every outputted line
 * it takes care of linebreak and column mode. It returns 1
 * if there is more for next line.
 */

static int do_one(struct format_stack *fs,
                  struct string_builder *r,
                  struct format_info *f)
{
  PCHARP rest;
  ptrdiff_t e, d, lastspace;

  rest.ptr=0;
  if(f->flags & (LINEBREAK|ROUGH_LINEBREAK))
  {
    if(f->width==SPRINTF_UNDECIDED)
      sprintf_error(fs, "Must have field width for linebreak.\n");
    lastspace=-1;
    for(e=0;e<f->len && e<=f->width;e++)
    {
      switch(INDEX_PCHARP(f->b,e))
      {
	case '\n':
	  lastspace=e;
	  rest=ADD_PCHARP(f->b,e+1);
	  break;

	case ' ':
	  if(f->flags & LINEBREAK)
	  {
	    lastspace=e;
	    rest=ADD_PCHARP(f->b,e+1);
	  }
	  /* FALL_THROUGH */

	default:
	  continue;
      }
      break;
    }
    if(e==f->len && f->len<=f->width)
    {
      lastspace=e;
      rest=ADD_PCHARP(f->b,lastspace);
    }else if(lastspace==-1){
      lastspace=MINIMUM(f->width,f->len);
      rest=ADD_PCHARP(f->b,lastspace);
    }
    fix_field(r,
	      f->b,
	      lastspace,
	      f->flags,
	      f->width,
	      f->pad_string,
	      f->pad_length,
	      f->pos_pad);
  }
  else if(f->flags & INVERSE_COLUMN_MODE)
  {
    if(f->width==SPRINTF_UNDECIDED)
      sprintf_error(fs, "Must have field width for column mode.\n");
    e=f->width/(f->column_width+1);
    if(!f->column_width || e<1) e=1;

    rest=f->b;
    for(d=0;INDEX_PCHARP(rest,d) && e;d++)
    {

      while(INDEX_PCHARP(rest,d) && INDEX_PCHARP(rest,d)!='\n')
	d++;

      fix_field(r,
		rest,
		d,
		f->flags,
		f->column_width,
		f->pad_string,
		f->pad_length,
		f->pos_pad);

      e--;
      INC_PCHARP(rest,d);
      d=-1;
      if(EXTRACT_PCHARP(rest)) INC_PCHARP(rest,1);
    }
  }
  else if(f->flags & COLUMN_MODE)
  {
    ptrdiff_t mod;
    ptrdiff_t col;
    PCHARP end;

    if(f->width==SPRINTF_UNDECIDED)
      sprintf_error(fs, "Must have field width for column mode.\n");
    mod=f->column_modulo;
    col=f->width/(f->column_width+1);
    if(!f->column_width || col<1) col=1;
    rest=f->b;
    end=ADD_PCHARP(rest,f->len);

    for(d=0;rest.ptr && d<col;d++)
    {

      /* Find end of entry */
      for(e=0;COMPARE_PCHARP(ADD_PCHARP(rest, e),<,end) &&
	    INDEX_PCHARP(rest,e)!='\n';e++);

      fix_field(r,rest,e,f->flags,f->column_width,
		f->pad_string,f->pad_length,f->pos_pad);

      f->column_entries--;

      /* Advance to after entry */
      INC_PCHARP(rest,e);
      if(!COMPARE_PCHARP(rest,<,end)) break;
      INC_PCHARP(rest,1);

      for(e=1;e<mod;e++)
      {
	PCHARP s=MEMCHR_PCHARP(rest,'\n',SUBTRACT_PCHARP(end,rest));
	if(s.ptr)
	{
	  rest=ADD_PCHARP(s,1);
	}else{
	  rest.ptr=0;
	  break;
	}
      }
    }
    if(f->column_entries>0)
    {
      for(rest=f->b;COMPARE_PCHARP(rest,<,end) &&
	    EXTRACT_PCHARP(rest)!='\n';INC_PCHARP(rest,1));
      if(COMPARE_PCHARP(rest,<,end)) INC_PCHARP(rest,1);
    }else{
      rest.ptr=0;
    }
  }
  else
  {
    fix_field(r,f->b,f->len,f->flags,f->width,
	      f->pad_string,f->pad_length,f->pos_pad);
  }

  if(f->flags & REPEAT) return 0;
  if(rest.ptr)
  {
    f->len-=SUBTRACT_PCHARP(rest,f->b);
    f->b=rest;
  }else{
    f->len=0;
    f->b=MKPCHARP("",0);
  }
  return f->len>0;
}

#define POP_ARGUMENT() do { \
    if (arg) arg = NULL;      \
    else if (argument < num_arg) {\
        lastarg=argp+(argument++); \
    } \
    else sprintf_error(fs, "Too few arguments to sprintf.\n"); \
} while (0)

#define GET_SVALUE(VAR) \
  if(arg) \
  { \
    VAR=arg; \
    arg=0; \
  }else{ \
    if(argument >= num_arg) \
    { \
      sprintf_error(fs, "Too few arguments to sprintf.\n"); \
      UNREACHABLE(break); \
    } \
    VAR=lastarg=argp+(argument++); \
  }

#define PEEK_SVALUE(VAR) \
  if(arg) \
  { \
    VAR=arg; \
  }else{ \
    if(argument >= num_arg) \
    { \
      sprintf_error(fs, "Too few arguments to sprintf.\n"); \
      UNREACHABLE(break); \
    } \
    VAR=argp+argument; \
  }

#define GET(VAR,PIKE_TYPE,TYPE_NAME,EXTENSION) \
  { \
    struct svalue *tmp_; \
    GET_SVALUE(tmp_); \
    if(TYPEOF(*tmp_) != PIKE_TYPE) \
    { \
      sprintf_error(fs, "Wrong type for argument %d: expected %s, got %s.\n",argument+1,TYPE_NAME, \
		    get_name_of_type(TYPEOF(*tmp_))); \
      UNREACHABLE(break); \
    } \
    VAR=tmp_->u.EXTENSION; \
  }

#define GET_INT(VAR) GET(VAR,T_INT,"integer",integer)
#define GET_STRING(VAR) GET(VAR,T_STRING,"string",string)
#define GET_FLOAT(VAR) GET(VAR,T_FLOAT,"float",float_number)
#define GET_ARRAY(VAR) GET(VAR,T_ARRAY,"array",array)
#define GET_OBJECT(VAR) GET(VAR,T_OBJECT,"object",object)


/* This is the main pike_sprintf function, note that it calls itself
 * recursively during the '%{ %}' parsing. The string is stored in
 * the buffer in save_objectII.c
 */
static void low_pike_sprintf(struct format_stack *fs,
			     struct string_builder *r,
			     PCHARP format,
			     ptrdiff_t format_len,
			     struct svalue *argp,
			     ptrdiff_t num_arg,
			     int nosnurkel)
{
  int argument=0;
  int tmp,setwhat,d,e,indent;
  char buffer[140];
  ptrdiff_t start;
  struct format_info *f, *fsp;
  double tf;
  struct svalue *arg=0;	/* pushback argument */
  struct svalue *lastarg=0;

  PCHARP a,begin;
  PCHARP format_end=ADD_PCHARP(format,format_len);

  check_c_stack(500);

  start=fs->fsp - fs->format_info_stack;
  for(a=format;COMPARE_PCHARP(a,<,format_end);INC_PCHARP(a,1))
  {
    int num_snurkel;

    if(fs->fsp + 1 == fs->format_info_stack + fs->size) {
      struct format_info * new;
      ptrdiff_t diff;
      fs->size *= 2;
      new = realloc(fs->format_info_stack, fs->size * sizeof(struct format_info));
      if (!new) sprintf_error(fs, "Cannot allocate more sprintf stack.\n");
      diff = new - fs->format_info_stack;
      fs->fsp = new + fs->size/2;
      fs->format_info_stack = new;
    } else fs->fsp++;
#ifdef PIKE_DEBUG
    if(fs->fsp < fs->format_info_stack)
      Pike_fatal("sprintf: fs->fsp out of bounds.\n");
#endif
    fsp = fs->fsp;
    fsp->pad_string=MKPCHARP(" ",0);
    fsp->pad_length=1;
    fsp->fi_free_string=0;
    fsp->to_free_string=0;
    fsp->column_width=0;
    fsp->pos_pad = 0;
    fsp->flags = 0;
    fsp->width=fsp->precision=SPRINTF_UNDECIDED;

    if(EXTRACT_PCHARP(a)!='%')
    {
      for(e=0;INDEX_PCHARP(a,e)!='%' &&
	    COMPARE_PCHARP(ADD_PCHARP(a,e),<,format_end);e++);
      fsp->b=a;
      fsp->len=e;
      fsp->width=e;
      INC_PCHARP(a,e-1);
      continue;
    }
    num_snurkel=0;
    arg=NULL;
    setwhat=0;
    begin=a;
    indent = 0;

    for(INC_PCHARP(a,1);;INC_PCHARP(a,1))
    {
      int mode = EXTRACT_PCHARP(a);

      switch (mode) {
      case '{':
      case 'n':
      case 't':
      case 'c':
      case 'H':
      case 'b':
      case 'o':
      case 'd':
      case 'u':
      case 'x':
      case 'X':
      case 'e':
      case 'f':
      case 'g':
      case 'E':
      case 'G':
      case 'F':
      case 'O':
      /* case 'p': */
      case 's':
      case 'q':
        if(UNLIKELY(fsp->flags & SNURKEL))
        {
          ONERROR _e;
          struct array *_v;
          struct string_builder _b;
          init_string_builder(&_b,0);
          SET_ONERROR(_e, free_string_builder, &_b);
          GET_ARRAY(_v);
          for(tmp=0;tmp<_v->size;tmp++)
          {
            struct svalue *save_sp=Pike_sp;
            array_index_no_free(Pike_sp,_v,tmp);
            Pike_sp++;
            low_pike_sprintf(fs, &_b,begin,SUBTRACT_PCHARP(a,begin)+1,
                             Pike_sp-1,1,nosnurkel+1);
            fsp = fs->fsp;
            if(save_sp < Pike_sp) pop_stack();
          }
          fsp->b=MKPCHARP_STR(_b.s);
          fsp->len=_b.s->len;
          fsp->to_free_string=_b.s;
          fsp->pad_string=MKPCHARP(" ",0);
          fsp->pad_length=1;
          fsp->column_width=0;
          fsp->pos_pad=0;
          fsp->flags=0;
          fsp->width=fsp->precision=SPRINTF_UNDECIDED;
          UNSET_ONERROR(_e);
          break;
        }
      default:
        goto cont_1;
      }
      break;

cont_1:

      switch (mode) {
      case 't':
      case 'c':
      case 'H':
      case 'b':
      case 'o':
      case 'd':
      case 'u':
      case 'x':
      case 'X':
      case 'e':
      case 'f':
      case 'g':
      case 'E':
      case 'G':
      case 'F':
        /* Its necessary to use CHECK_OBJECT_SPRINTF() here,
           because describe_svalue encodes \t and others
           when returned by _sprintf */
      case 'O':
      case 's':
      case 'q':
        {
          struct svalue *sv;
          PEEK_SVALUE(sv);
          if(TYPEOF(*sv) == T_OBJECT && sv->u.object->prog)
          {
            ptrdiff_t fun=FIND_LFUN(sv->u.object->prog, LFUN__SPRINTF);
            if (fun != -1) {
              if (call_object_sprintf(mode, sv->u.object, fun, fs)) {
                POP_ARGUMENT();
                break;
              }
            }
          }
        }
	/* FALL_THROUGH */

      default:
        goto cont_2;
      }
      break;

cont_2:

      switch(mode)
      {
      default:
	if(mode < 256 && isprint(mode))
	{
	  sprintf_error(fs, "Error in format string, %c is not a format.\n",
			mode);
	}else{
	  sprintf_error(fs,"Error in format string, U%08x is not a format.\n",
			mode);
	}
	break;

      /* First the modifiers */
      case '0':
	 if (setwhat<2)
	 {
	    fsp->flags|=ZERO_PAD;
	    continue;
	 }
	 /* FALL_THROUGH */
      case '1': case '2': case '3':
      case '4': case '5': case '6':
      case '7': case '8': case '9':
	tmp=STRTOL_PCHARP(a,&a,10);
	INC_PCHARP(a,-1);
	goto got_arg;

      case '*':
	{
	  struct svalue *sval;
	  struct mapping *m;
	  GET_SVALUE(sval);
	  if (TYPEOF(*sval) == T_INT) {
	    tmp = sval->u.integer;
	    goto got_arg;
	  } else if (TYPEOF(*sval) != T_MAPPING) {
	    sprintf_error(fs, "Wrong type for argument %d: "
			  "expected %s, got %s.\n",
			  argument+1, "int|mapping(string:int)",
			  get_name_of_type(TYPEOF(*sval)));
	  }
          mapping_to_format_info(sval->u.mapping, fsp, &indent);
	  continue;
	}

      got_arg:
	switch(setwhat)
	{
	case 0:
	case 1:
	  if(tmp < 0) sprintf_error(fs, "Illegal width %d.\n", tmp);
	  fsp->width=tmp;
	  if (!setwhat) break;
	case 2: fsp->precision=tmp; break;
	case 3: fsp->column_width=tmp; break;
	case 4: fsp->precision=-tmp; break;
	}
	continue;

      case ';': setwhat=3; continue;
      case '.': setwhat=2; continue;
      case ':': setwhat=1; continue;

      case '=': fsp->flags|=LINEBREAK;
	if (fsp->flags & ROUGH_LINEBREAK)
	  sprintf_error(fs,
			"Combining modifiers '=' and '/' is not allowed.\n");
	continue;
      case '/': fsp->flags|=ROUGH_LINEBREAK;
	if (fsp->flags & LINEBREAK)
	  sprintf_error(fs,
			"Combining modifiers '=' and '/' is not allowed.\n");
	continue;
      case '#': fsp->flags|=COLUMN_MODE; continue;
      case '$': fsp->flags|=INVERSE_COLUMN_MODE; continue;

      case '-':
	if(setwhat==2)
	  setwhat=4;
	else
	  fsp->flags|=FIELD_LEFT;
	continue;
      case '|': fsp->flags|=FIELD_CENTER; continue;
      case ' ': fsp->pos_pad=' '; continue;
      case '+': fsp->pos_pad='+'; continue;
      case '!': fsp->flags^=DO_TRUNC; continue;
      case '^': fsp->flags|=REPEAT; continue;
      case '>': fsp->flags|=MULTI_LINE_BREAK; continue;
      case '_': fsp->flags|=WIDTH_OF_DATA; continue;
      case '@':
	if(++num_snurkel > nosnurkel)
	  fsp->flags|=SNURKEL;
	continue;

      case '\'':
	tmp=0;
	for(INC_PCHARP(a,1);INDEX_PCHARP(a,tmp)!='\'';tmp++)
	{
#if 0
	  fprintf(stderr, "Sprinf-glop: %d (%c)\n",
		  INDEX_PCHARP(a,tmp), INDEX_PCHARP(a,tmp));
#endif
	  if(COMPARE_PCHARP(ADD_PCHARP(a, tmp),>=,format_end))
	    sprintf_error(fs, "Unfinished pad string in format string.\n");
	}
	if(tmp)
	{
	  fsp->pad_string=a;
	  fsp->pad_length=tmp;
	}
	INC_PCHARP(a,tmp);
	continue;

      case '~':
      {
	struct pike_string *s;
	GET_STRING(s);
	if (s->len) {
	  fsp->pad_string=MKPCHARP_STR(s);
	  fsp->pad_length=s->len;
	}
	continue;
      }

      case '<':
	if(!lastarg)
	  sprintf_error(fs, "No last argument.\n");
	arg=lastarg;
	continue;

      case '[':
	INC_PCHARP(a,1);
	if(EXTRACT_PCHARP(a)=='*') {
	  GET_INT(tmp);
	  INC_PCHARP(a,1);
	} else
	  tmp=STRTOL_PCHARP(a,&a,10);
	if(EXTRACT_PCHARP(a)!=']')
	  sprintf_error(fs, "Expected ] in format string, not %c.\n",
			EXTRACT_PCHARP(a));
	if(tmp >= num_arg)
	  sprintf_error(fs, "Not enough arguments to [%d].\n",tmp);
	arg = argp+tmp;
	continue;

        /* now the real operators */

      case '{':
      {
	struct array *w;
	struct string_builder b;
	for(e=1,tmp=1;tmp;e++)
	{
	  if (!INDEX_PCHARP(a,e) &&
	      !COMPARE_PCHARP(ADD_PCHARP(a,e),<,format_end)) {
	    sprintf_error(fs, "Missing %%} in format string.\n");
            UNREACHABLE(break);
	  } else if(INDEX_PCHARP(a,e)=='%') {
	    switch(INDEX_PCHARP(a,e+1))
	    {
	    case '%': e++; break;
	    case '}': tmp--; break;
	    case '{': tmp++; break;
	    }
	  }
	}

	GET_ARRAY(w);
	if(!w->size)
	{
	  fsp->b=MKPCHARP("",0);
	  fsp->len=0;
	}else{
	  ONERROR err;
	  init_string_builder(&b,0);
	  SET_ONERROR(err,free_string_builder,&b);
	  for(tmp=0;tmp<w->size;tmp++)
	  {
	    struct svalue *s;
	    union anything *q;

/*	    check_threads_etc(); */
	    q=low_array_get_item_ptr(w,tmp,T_ARRAY);
	    s=Pike_sp;
	    if(q)
	    {
	      add_ref(q->array);
	      push_array_items(q->array);
	    }else{
	      array_index_no_free(Pike_sp,w,tmp);
	      Pike_sp++;
	    }
	    low_pike_sprintf(fs, &b,ADD_PCHARP(a,1),e-2,s,Pike_sp-s,0);
            fsp = fs->fsp;
	    pop_n_elems(Pike_sp-s);
	  }
#ifdef PIKE_DEBUG
	  if(fs->fsp < fs->format_info_stack)
	    Pike_fatal("sprintf: fs->fsp out of bounds.\n");
#endif
	  fsp->b=MKPCHARP_STR(b.s);
	  fsp->len=b.s->len;
	  fsp->to_free_string=b.s;
	  UNSET_ONERROR(err);
	}

	INC_PCHARP(a,e);
	break;
      }

      case '%':
	fsp->b=MKPCHARP("%",0);
	fsp->len=fsp->width=1;
	break;

      case 'n':
	fsp->b=MKPCHARP("",0);
	fsp->len=0;
	break;

      case 't':
      {
	struct svalue *t;
	GET_SVALUE(t);
	fsp->b = MKPCHARP(get_name_of_type(TYPEOF(*t)),0);
	fsp->len=strlen((char *)fsp->b.ptr);
	break;
      }

      case 'c':
      {
        INT_TYPE tmp;
	ptrdiff_t l,n;
	char *x;
	if(fsp->width == SPRINTF_UNDECIDED)
	{
	  GET_INT(tmp);
          x=(char *)sa_alloc(&fs->a, 4);
	  if(tmp<256) fsp->b=MKPCHARP(x,0);
	  else if(tmp<65536) fsp->b=MKPCHARP(x,1);
	  else  fsp->b=MKPCHARP(x,2);
	  SET_INDEX_PCHARP(fsp->b,0,tmp);
	  fsp->len=1;
	}
	else if ( (fsp->flags&FIELD_LEFT) )
	{
	  l=1;
	  if(fsp->width > 0) l=fsp->width;
          x=(char *)sa_alloc(&fs->a, l);
	  fsp->b=MKPCHARP(x,0);
	  fsp->len=l;
	  GET_INT(tmp);
	  n=0;
	  while(n<l)
	  {
	    x[n++]=tmp & 0xff;
	    tmp>>=8;
	  }
	}
	else
	{
	  l=1;
	  if(fsp->width > 0) l=fsp->width;
	  x=(char *)sa_alloc(&fs->a, l);
	  fsp->b=MKPCHARP(x,0);
	  fsp->len=l;
	  GET_INT(tmp);
	  while(--l>=0)
	  {
	    x[l]=tmp & 0xff;
	    tmp>>=8;
	  }
	}
	break;
      }

      case 'H':
      {
	struct string_builder buf;
	struct pike_string *s;
        ptrdiff_t tmp;
	ptrdiff_t l,n;
	char *x;

	GET_STRING(s);
	if( s->size_shift )
	  sprintf_error(fs, "%%H requires all characters in the string "
			"to be at most eight bits large\n");

	tmp = s->len;
        l=1;
        if(fsp->width > 0)
          l=fsp->width;
        else if(fsp->flags&ZERO_PAD)
          sprintf_error(fs, "Length of string to %%H is 0.\n");

	/* Note: The >>-operator performs an implicit % (sizeof(tmp)*8)
	 *       on the shift operand on some architectures. */
        if( (l < (ptrdiff_t) sizeof(tmp)) && (tmp>>(l*8)))
	  sprintf_error(fs, "Length of string to %%%"PRINTPTRDIFFT"dH "
			"too large.\n", l);


        x=(char *)sa_alloc(&fs->a, l);
        fsp->b=MKPCHARP(x,0);
        fsp->len=l;

        if ( (fsp->flags&FIELD_LEFT) )
	{
	  n=0;
	  while(n<l)
	  {
	    x[n++]=tmp & 0xff;
	    tmp>>=8;
	  }
	}
	else
	{
	  while(--l>=0)
	  {
	    x[l]=tmp & 0xff;
	    tmp>>=8;
	  }
	}

	init_string_builder_alloc(&buf, s->len+fsp->len, 0);
	string_builder_append(&buf,fsp->b,fsp->len);
	string_builder_shared_strcat(&buf,s);

	fsp->b = MKPCHARP_STR(buf.s);
	fsp->len = buf.s->len;
	buf.s->len = buf.malloced;
	fsp->to_free_string = buf.s;
	break;
      }

      case 'x':
      case 'X':
      {
        struct svalue *v;
        PEEK_SVALUE(v);
        if(TYPEOF(*v)==T_STRING)
        {
          struct pike_string *str;
          push_svalue(v);
          if( mode=='X' )
          {
            push_int(1);
            f_string2hex(2);
          }
          else
            f_string2hex(1);

          str = Pike_sp[-1].u.string;
          fsp->b = MKPCHARP_STR(str);
          fsp->len = str->len;
          fsp->to_free_string = str;

          add_ref(str);
          pop_stack();
          POP_ARGUMENT();
          break;
        }
      }
      case 'b':
      case 'o':
      case 'd':
      case 'u':
      {
	int base = 0, mask_size = 0;
       char *x;
	INT_TYPE val;

	GET_INT(val);

	if(fsp->precision != SPRINTF_UNDECIDED && fsp->precision > 0)
	  mask_size = fsp->precision;

	x=(char *)sa_alloc(&fs->a, sizeof(val)*CHAR_BIT + 4 + mask_size);
	fsp->b=MKPCHARP(x,0);

	switch(mode)
	{
	  case 'b': base = 1; break;
	  case 'o': base = 3; break;
	  case 'x': base = 4; break;
	  case 'X': base = 4; break;
	}

	if(base)
	{
	  char *p = x;
	  ptrdiff_t l;

	  if(mask_size || val>=0)
	  {
	    do {
	      if((*p++ = '0'|(val&((1<<base)-1)))>'9')
		p[-1] += (mode=='X'? 'A'-'9'-1 : 'a'-'9'-1);
	      val >>= base;
	    } while(--mask_size && val);
	    l = p-x;
	  }
	  else
	  {
	    *p++ = '-';
	    val = -val;
	    do {
	      if((*p++ = '0'|(val&((1<<base)-1)))>'9')
		p[-1] += (mode=='X'? 'A'-'9'-1 : 'a'-'9'-1);
	      val = ((unsigned INT_TYPE)val) >> base;
	    } while(val);
	    l = p-x-1;
	  }
	  *p = '\0';
	  while(l>1) {
	    char t = p[-l];
	    p[-l] = p[-1];
	    p[-1] = t;
	    --p;
	    l -= 2;
	  }
	}
	else if(mode == 'u')
	  sprintf(x, "%"PRINTPIKEINT"u", (unsigned INT_TYPE) val);
	else
	  sprintf(x, "%"PRINTPIKEINT"d", val);

	fsp->len=strlen(x);
	break;
      }

      case 'e':
      case 'f':
      case 'g':
      case 'E':
      case 'G':
      {
	char *x;
	GET_FLOAT(tf);

	/* Special casing for infinity and not a number,
	 * since many libc's forget about them...
	 */
	if (PIKE_ISNAN(tf)) {
	  /* NaN */
	  fsp->b = MKPCHARP("nan", 0);
	  fsp->len = 3;
	  break;
	} else if (PIKE_ISINF(tf)) {
	  /* Infinity. */
	  if (tf > 0.0) {
	    fsp->b = MKPCHARP("inf", 0);
	    fsp->len = 3;
	  } else {
	    fsp->b = MKPCHARP("-inf", 0);
	    fsp->len = 4;
	  }
	  break;
	}

	if (fsp->precision==SPRINTF_UNDECIDED) fsp->precision=3;

	/* FIXME: The constant (320) is good for IEEE double precision
	 * float, but will definitely fail for bigger precision! --aldem
	 */
	x=(char *)xalloc(320+MAXIMUM(fsp->precision,3));
	fsp->fi_free_string=x;
	fsp->b=MKPCHARP(x,0);
	sprintf(buffer,"%%*.*%c", mode);

	if(fsp->precision<0) {
	  double m=pow(10.0, (double)fsp->precision);
	  tf = rint(tf*m)/m;
	} else if (fsp->precision==0) {
	  tf = rint(tf);
        }

	debug_malloc_touch(x);
	sprintf(x,buffer,1,fsp->precision<0?0:fsp->precision,tf);
	debug_malloc_touch(x);
	fsp->len=strlen(x);

	/* Make sure that the last digits really are zero. */
	if(fsp->precision<0)
	{
	  ptrdiff_t i, j;
	  /* Find the ending of the number.  Yes, this can be made
	     simpler now when the alignment bug for floats is fixed. */
	  for(i=fsp->len-1; i>=0; i--)
 	    if('0'<=x[i] && x[i]<='9')
	    {
	      i+=fsp->precision+1;
	      if(i>=0 && '0'<=x[i] && x[i]<='9')
		for(j=0; j<-fsp->precision; j++)
		  x[i+j]='0';
	      break;
	    }
	}
	break;
      }

      case 'F':
      {
        ptrdiff_t l;
	char *x;
        l=4;
        if(fsp->width > 0) l=fsp->width;
	if(l != 4 && l != 8)
          sprintf_error(fs, "Invalid IEEE width %ld.\n", (long)l);
        x=(char *)sa_alloc(&fs->a, l);
	fsp->b=MKPCHARP(x,0);
	fsp->len=l;
	GET_FLOAT(tf);
	switch(l) {
	case 4:
	  {
            float f = (float)tf;
#if SIZEOF_FLOAT_TYPE > 4
	    /* Some paranoia in case libc doesn't handle
	     * conversion to denormalized floats. */
	    if ((f != 0.0) || (tf == 0.0)) {
#endif
#ifdef FLOAT_IS_IEEE_BIG
	      memcpy(x, &f, 4);
#elif defined(FLOAT_IS_IEEE_LITTLE)
	      x[0] = ((char *)&f)[3];
	      x[1] = ((char *)&f)[2];
	      x[2] = ((char *)&f)[1];
	      x[3] = ((char *)&f)[0];
#else
	      low_write_IEEE_float(x, tf, 4);
#endif /* IEEE */
#if SIZEOF_FLOAT_TYPE > 4
	      break;
	    }
	    low_write_IEEE_float(x, tf, 4);
#endif
	  }
	  break;
	case 8:
#ifdef DOUBLE_IS_IEEE_BIG
	  memcpy(x, &tf, 8);
#elif defined(DOUBLE_IS_IEEE_LITTLE)
	  x[0] = ((char *)&tf)[7];
	  x[1] = ((char *)&tf)[6];
	  x[2] = ((char *)&tf)[5];
	  x[3] = ((char *)&tf)[4];
	  x[4] = ((char *)&tf)[3];
	  x[5] = ((char *)&tf)[2];
	  x[6] = ((char *)&tf)[1];
	  x[7] = ((char *)&tf)[0];
#else
	  low_write_IEEE_float(x, tf, 8);
#endif
	}
	if (fsp->flags & FIELD_LEFT) {
	  /* Reverse the byte order. */
	  int i;
	  char c;
	  l--;
	  for (i=0; i < (l-i); i++) {
	    c = x[i];
	    x[i] = x[l-i];
	    x[l-i] = c;
	  }
	}
	break;
      }

      case 'O':
      {
	struct svalue *t;
	GET_SVALUE(t);
	{
	  dynamic_buffer save_buf;
	  dynbuf_string s;

	  init_buf(&save_buf);
	  describe_svalue(t,indent,0);
	  s=complex_free_buf(&save_buf);
	  fsp->b=MKPCHARP(buffer_ptr(&s),0);
	  fsp->len=buffer_content_length(&s);
	  fsp->fi_free_string=buffer_ptr(&s);
	  break;
	}
      }

#if 0
      /* This can be useful when doing low level debugging. */
      case 'p':
      {
	dynamic_buffer save_buf;
	dynbuf_string s;
	char buf[50];
	struct svalue *t;
	GET_SVALUE(t);
	init_buf(&save_buf);
	sprintf (buf, "%p", t->u.refs);
	my_strcat (buf);
	s=complex_free_buf(&save_buf);
	fsp->b=MKPCHARP(s.str,0);
	fsp->len=s.len;
	fsp->fi_free_string=s.str;
	break;
      }
#endif

      case 's':
      {
	struct pike_string *s;
	GET_STRING(s);
	fsp->b=MKPCHARP_STR(s);
	fsp->len=s->len;
	if(fsp->precision != SPRINTF_UNDECIDED && fsp->precision < fsp->len)
	  fsp->len = (fsp->precision < 0 ? 0 : fsp->precision);
	break;
      }

      case 'q':
      {
	struct string_builder buf;
	struct pike_string *s;

	GET_STRING(s);

	init_string_builder_alloc(&buf, s->len+2, 0);
	string_builder_putchar(&buf, '"');
	string_builder_quote_string(&buf, s, 0,
				    (fsp->precision == SPRINTF_UNDECIDED)?
				    0x7fffffff:fsp->precision-1,
				    QUOTE_NO_STRING_CONCAT);
	string_builder_putchar(&buf, '"');

	fsp->b = MKPCHARP_STR(buf.s);
	fsp->len = buf.s->len;
	/* NOTE: We need to do this since we're not
	 *       using free_string_builder(). */
	buf.s->len = buf.malloced;
	fsp->to_free_string = buf.s;
	break;
      }
      }
      break;
    }
  }

  for(f=fs->fsp;f>fs->format_info_stack + start;f--)
  {
    if(f->flags & WIDTH_OF_DATA)
      f->width=f->len;

    if(((f->flags & INVERSE_COLUMN_MODE) && !f->column_width) ||
       (f->flags & COLUMN_MODE))
    {
      int max_len,nr;
      ptrdiff_t columns;
      tmp=1;
      for(max_len=nr=e=0;e<f->len;e++)
      {
	if(INDEX_PCHARP(f->b,e)=='\n')
	{
	  nr++;
	  if(max_len<tmp) max_len=tmp;
	  tmp=0;
	}
	tmp++;
      }
      nr++;
      if(max_len<tmp)
	max_len=tmp;
      if(!f->column_width)
	f->column_width=max_len;
      f->column_entries=nr;
      columns=f->width/(f->column_width+1);

      if(f->column_width<1 || columns<1)
	columns=1;
      f->column_modulo=(nr+columns-1)/columns;
    }
  }

  /* Here we do some DWIM */
  for(f=fs->fsp-1;f>fs->format_info_stack + start;f--)
  {
    if((f[1].flags & MULTILINE) &&
       !(f[0].flags & (MULTILINE|MULTI_LINE_BREAK)))
    {
      if(! MEMCHR_PCHARP(f->b, '\n', f->len).ptr ) f->flags|=MULTI_LINE;
    }
  }

  for(f=fs->format_info_stack + start+1;f<=fs->fsp;)
  {
    for(;f<=fs->fsp && !(f->flags&MULTILINE);f++)
      do_one(fs, r, f);

    do {
      d=0;
      for(e=0;f+e<=fs->fsp && (f[e].flags & MULTILINE);e++)
	d |= do_one(fs, r, f+e);
      if(d)
	string_builder_putchar(r,'\n');
    } while(d);

    for(;f<=fs->fsp && (f->flags&MULTILINE); f++);
  }

  f = fs->fsp;
  fsp = fs->format_info_stack + start;
  while(f > fsp)
  {
#ifdef PIKE_DEBUG
    if(f < fs->format_info_stack)
      Pike_fatal("sprintf: fsp out of bounds.\n");
#endif

    if(f->fi_free_string)
      free(f->fi_free_string);
#ifdef PIKE_DEBUG
    f->fi_free_string=0;
#endif

    if(f->to_free_string)
      free_string(f->to_free_string);
#ifdef PIKE_DEBUG
    f->to_free_string=0;
#endif

    f--;
  }
  fs->fsp = f;
}

static void free_f_sprintf_data (struct format_stack *fs)
{
  free_sprintf_strings (fs);
  stack_alloc_destroy(&fs->a);
  free (fs->format_info_stack);
}

/* The efun */
void low_f_sprintf(INT32 args, struct string_builder *r)
{
  ONERROR uwp;
  struct pike_string *ret;
  struct svalue *argp;
  struct format_stack fs;

  argp=Pike_sp-args;

  if(TYPEOF(argp[0]) != T_STRING) {
    if (TYPEOF(argp[0]) == T_OBJECT) {
      /* Try checking if we can cast it to a string... */
      ref_push_object(argp[0].u.object);
      o_cast(string_type_string, PIKE_T_STRING);
      if (TYPEOF(Pike_sp[-1]) != T_STRING) {
	/* We don't accept objects... */
	Pike_error("Cast to string failed.\n");
      }
      /* Replace the original object with the new string. */
      assign_svalue(argp, Pike_sp-1);
      /* Clean up the stack. */
      pop_stack();
    } else {
      SIMPLE_ARG_TYPE_ERROR("sprintf", 1, "string|object");
    }
  }

  fs.size = round_up32(args*2);
  stack_alloc_init(&fs.a, 128); /* this should scale with fs.size */
  fs.format_info_stack = xalloc(fs.size*sizeof(struct format_info));
  fs.fsp = fs.format_info_stack - 1;

  SET_ONERROR(uwp, free_f_sprintf_data, &fs);
  low_pike_sprintf(&fs,
		   r,
		   MKPCHARP_STR(argp->u.string),
		   argp->u.string->len,
		   argp+1,
		   args-1,
		   0);
  UNSET_ONERROR(uwp);
  stack_alloc_destroy(&fs.a);
  free (fs.format_info_stack);
}

void f_sprintf(INT32 args)
{
  ONERROR uwp;
  struct string_builder r;
  SET_ONERROR(uwp, free_string_builder, &r);
  init_string_builder(&r,0);
  low_f_sprintf(args, &r);
  UNSET_ONERROR(uwp);
  pop_n_elems(args);
  push_string(finish_string_builder(&r));
}

#define PSAT_INVALID	1
#define PSAT_MARKER	2

/* Push the types corresponding to the %-directives in the format string.
 *
 *   severity is the severity level if any syntax errors
 *            are encountered in the format string.
 *
 * Returns -1 on syntax error.
 *         LSB 1 on unhandled (ie position-dependent args).
 *         LSB 0 on success.
 *
 *         PSAT_MARKER is set if the marker is in use.
 *
 *         *min_charp is <= *max_charp if a static character range
 *         has been detected (eg constant string segment, etc).
 */
static int push_sprintf_argument_types(PCHARP format,
				       ptrdiff_t format_len,
				       int ret,
				       int int_marker,
				       p_wchar2 *min_charp,
				       p_wchar2 *max_charp,
				       int severity)
{
  int tmp, setwhat, e;
  int uses_marker = 0;
  struct svalue *arg=0;	/* pushback argument */
  struct svalue *lastarg=0;

  PCHARP a,begin;
  PCHARP format_end=ADD_PCHARP(format,format_len);

  p_wchar2 min_char = *min_charp;
  p_wchar2 max_char = *max_charp;

  check_c_stack(500);

  /* if (num_arg < 0) num_arg = MAX_INT32; */

  for(a=format;COMPARE_PCHARP(a,<,format_end);INC_PCHARP(a,1))
  {
    int num_snurkel, column;
    ptrdiff_t width = 0;
    p_wchar2 c;

    if(EXTRACT_PCHARP(a)!='%')
    {
      for(e=0;(c = INDEX_PCHARP(a,e)) != '%' &&
	    COMPARE_PCHARP(ADD_PCHARP(a,e),<,format_end);e++) {
	if (c < min_char) min_char = c;
	if (c > max_char) max_char = c;
      }
      INC_PCHARP(a,e-1);
      continue;
    }
    num_snurkel=0;
    column=0;
    arg=NULL;
    setwhat=0;
    begin=a;

    for(INC_PCHARP(a,1);;INC_PCHARP(a,1))
    {
      switch(c = EXTRACT_PCHARP(a))
      {
      default:
	if(c < 256 && isprint(c))
	{
	  yyreport(severity, type_check_system_string,
		   0, "Error in format string, %c is not a format.",
		   c);
	}else{
	  yyreport(severity, type_check_system_string,
		   0, "Error in format string, U%08x is not a format.",
		   c);
	}
	ret = -1;
	num_snurkel = 0;
	break;

      /* First the modifiers */
      case '*':
	if (setwhat < 2) {
	  push_int_type(0, MAX_INT32);
	} else {
	  push_int_type(MIN_INT32, MAX_INT32);
	}
	/* Allow a mapping in all cases. */
	push_int_type(MIN_INT32, MAX_INT32);
	push_type(T_STRING);
	push_int_type(MIN_INT32, MAX_INT32);
	push_reverse_type(T_MAPPING);
	push_type(T_OR);
	continue;

      case '0':
	if (setwhat<2) continue;
	/* FALL_THROUGH */
      case '1': case '2': case '3':
      case '4': case '5': case '6':
      case '7': case '8': case '9':
	tmp=STRTOL_PCHARP(a,&a,10);
	INC_PCHARP(a,-1);
	switch(setwhat)
	{
	case 0: case 1:
	  if(tmp < 0) {
	    yyreport(severity, type_check_system_string,
		     0, "Illegal width %d.", tmp);
	    ret = -1;
	  }
	  width = tmp;
	  if (width) {
	    if (' ' < min_char) min_char = ' ';
	    if (' ' > max_char) max_char = ' ';
	  }
	/* FALL_THROUGH */
	case 2: case 3: case 4: break;
	}
	setwhat++;
	continue;

      case ';': setwhat=3; continue;
      case '.': setwhat=2; continue;
      case ':': setwhat=1; continue;
      case '-':
	if(setwhat==2) setwhat=4;
	continue;

      case '/':
        column |= ROUGH_LINEBREAK;
        if( column & LINEBREAK ) {
	  yyreport(severity, type_check_system_string,
		   0, "Can not use both the modifiers / and =.");
	  ret = -1;
	}
        continue;

      case '=':
        column |= LINEBREAK;
        if( column & ROUGH_LINEBREAK ) {
	  yyreport(severity, type_check_system_string,
		   0, "Can not use both the modifiers / and =.");
	  ret = -1;
	}
	if ('\n' < min_char) min_char = '\n';
	if (' ' > max_char) max_char = ' ';
        continue;

      case '#': case '$': case '>':
	if ('\n' < min_char) min_char = '\n';
	/* FALL_THROUGH */
      case '|': case ' ': case '+':
	if (' ' > max_char) max_char = ' ';
	if (' ' < min_char) min_char = ' ';
	/* FALL_THROUGH */
      case '!': case '^': case '_':
	continue;

      case '@':
	++num_snurkel;
	continue;

      case '\'':
	tmp=0;
	for(INC_PCHARP(a,1);
	    COMPARE_PCHARP(ADD_PCHARP(a, tmp),<,format_end)
	      && (c = INDEX_PCHARP(a,tmp)) != '\'';tmp++) {
	  if (c < min_char) min_char = c;
	  if (c > max_char) max_char = c;
	}

	if (COMPARE_PCHARP(ADD_PCHARP(a, tmp),<,format_end)) {
	    INC_PCHARP(a,tmp);
	    continue;
	} else {
	    INC_PCHARP(a,tmp);
	    yyreport(severity, type_check_system_string,
		     0, "Unfinished pad string in format string.");
	    ret = -1;
	    break;
	}

      case '~':
      {
	push_finished_type(int_type_string);
	if (int_marker) {
	  push_assign_type(int_marker);
	  uses_marker = 1;
	}
	push_type(T_STRING);
	continue;
      }

      case '<':
	/* FIXME: !!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
	ret |= PSAT_INVALID;	/* FAILURE! */
#if 0
	if(!lastarg) {
	  yyreport(severity, type_check_system_string,
		   0, "No last argument.");
	  ret = -1;
	}
	arg=lastarg;
#endif /* 0 */
	continue;

      case '[':
	/* FIXME: !!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
	ret |= PSAT_INVALID;	/* FAILURE! */
	INC_PCHARP(a,1);
	if(EXTRACT_PCHARP(a)=='*') {
	  push_int_type(0, /*num_arg*/ MAX_INT32);
	  INC_PCHARP(a,1);
	}
        else
	  STRTOL_PCHARP(a,&a,10);
	if((c = EXTRACT_PCHARP(a))!=']')  {
	  yyreport(severity, type_check_system_string,
		   0, "Expected ] in format string, not %c.",
		   c);
	  ret = -1;
	}
	continue;

        /* now the real operators */

      case '{':
      {
	ptrdiff_t cnt;
	for(e=1,tmp=1;tmp;e++)
	{
	  if (!INDEX_PCHARP(a,e) &&
	      !COMPARE_PCHARP(ADD_PCHARP(a,e),<,format_end)) {
	    yyreport(severity, type_check_system_string,
		     0, "Missing %%} in format string.");
	    ret = -1;
	    break;
	  } else if((c = INDEX_PCHARP(a,e)) == '%') {
	    switch(INDEX_PCHARP(a,e+1))
	    {
	    case '%': e++; break;
	    case '}': tmp--; break;
	    case '{': tmp++; break;
	    }
	  } else {
	    if (c < min_char) min_char = c;
	    if (c > max_char) max_char = c;
	  }
	}

	type_stack_mark();
	/* Note: No need to check the return value, since we
	 *       simply or all the types together. Thus the
	 *       argument order isn't significant.
	 *
	 * ... Unless there was a syntax error...
	 */
	ret = push_sprintf_argument_types(ADD_PCHARP(a,1), e-2, ret,
					  int_marker, &min_char, &max_char,
					  severity);
	/* Join the argument types for our array. */
	push_type(PIKE_T_ZERO);
	for (cnt = pop_stack_mark(); cnt > 1; cnt--) {
	  push_type(T_OR);
	}
	push_finished_type(peek_type_stack());	/* dup() */
	push_type(PIKE_T_ARRAY);
	push_type(T_OR);

	push_type(PIKE_T_ARRAY);

	INC_PCHARP(a,e);
	break;
      }

      case '%':
	num_snurkel = 0;
	if ('%' < min_char) min_char = '%';
	if ('%' > max_char) max_char = '%';
	break;

      case 'n':
	if (num_snurkel) {
	  /* NOTE: Does take an argument if '@' is active! */
	  push_type(PIKE_T_ZERO);
	}
	break;

      case 't':
      {
	push_type(T_MIXED);

	/* Lower-case ASCII */
	if ('a' < min_char) min_char = 'a';
	if ('z' > max_char) max_char = 'z';
	break;
      }

      case 'c':
      {
	push_object_type(0, 0);
	push_int_type(MIN_INT32, MAX_INT32);
	push_type(T_OR);
	if (!width) {
	  if (int_marker) {
	    push_assign_type(int_marker);
	    ret |= PSAT_MARKER;
	  }
	} else {
	  if (min_char > 0) min_char = 0;
	  if (max_char < 255) max_char = 255;
	}
	break;
      }

      case 'x':
	if ('f' > max_char) max_char = 'f';
        /* FALL_THROUGH */
      case 'X':
        if ('F' > max_char) max_char = 'F';
        if ('+' < min_char) min_char = '+';
        push_int_type(0, 255);
	push_type(T_STRING);
        push_object_type(0, 0);
        push_type(T_OR);
        push_int_type(MIN_INT32, MAX_INT32);
        push_type(T_OR);
        break;

      case 'd':
      case 'u':
	if ('9' > max_char) max_char = '9';
	/* FALL_THROUGH */
      case 'o':
	if ('7' > max_char) max_char = '7';
	/* FALL_THROUGH */
      case 'b':
	if ('1' > max_char) max_char = '1';
        if ('+' < min_char) min_char = '+';
      {
        push_object_type(0, 0);
        push_int_type(MIN_INT32, MAX_INT32);
	push_type(T_OR);
	break;
      }

      case 'e':
      case 'f':
      case 'g':
      case 'E':
      case 'F':
      case 'G':
      {
	push_object_type(0, 0);
	push_type(PIKE_T_FLOAT);
	push_type(T_OR);
	if ('e' > max_char) max_char = 'e';
	if ('+' < min_char) min_char = '+';
	break;
      }

      case 'O':
      {
	push_type(T_MIXED);
	max_char = 0x7fffffff;
	min_char = -0x80000000;
	break;
      }

#if 0
      /* This can be useful when doing low level debugging. */
      case 'p':
      {
	push_type(T_MIXED);
	break;
      }
#endif
      case 'H':
      {
	push_object_type(0, 0);
	push_int_type(0, 255);
	push_type(T_STRING);
	push_type(T_OR);
	if (min_char > 0) min_char = 0;
	if (max_char < 255) max_char = 255;
	break;
      }

      case 'q':
      case 's':
      {
	push_object_type(0, 0);
	if (int_marker) {
	  push_assign_type(int_marker);
	}
	push_finished_type(int_type_string);
	if (int_marker) {
	  push_assign_type(int_marker);
	  ret |= PSAT_MARKER;
	}
	push_type(T_STRING);
	push_type(T_OR);
	break;
      }

      }
      break;
    }
    while (num_snurkel--) push_type(T_ARRAY);
  }

  *min_charp = min_char;
  *max_charp = max_char;
  return ret;
}

static node *optimize_sprintf(node *n)
{
  node **arg0 = my_get_arg(&_CDR(n), 0);
  node **arg1 = my_get_arg(&_CDR(n), 1);
  node *ret = NULL;
  int num_args=count_args(CDR(n));
  if(arg0 &&
     (*arg0)->token == F_CONSTANT &&
     TYPEOF((*arg0)->u.sval) == T_STRING)
  {
    /* First argument is a constant string. */
    struct pike_string *fmt = (*arg0)->u.sval.u.string;

    if(arg1 && num_args == 2 &&
       fmt->size_shift == 0 && fmt->len == 2 && STR0(fmt)[0]=='%')
    {
      /* First argument is a two character format string. */
      switch(STR0(fmt)[1])
      {
      case 'c':
        ADD_NODE_REF2(*arg1,
		      ret = mkefuncallnode("int2char",*arg1);
          );
	return ret;

      case 't':
        ADD_NODE_REF2(*arg1,
		      ret = mkefuncallnode("basetype",*arg1);
	  );
	return ret;

      case 'x':
        if(TYPEOF((*arg1)->u.sval) == T_STRING)
          ADD_NODE_REF2(*arg1,
                        ret = mkefuncallnode("string2hex",*arg1);
          );
        else
          ADD_NODE_REF2(*arg1,
                        ret = mkefuncallnode("int2hex",*arg1);
	  );
        return ret;
      case '%':
	{
	  /* FIXME: This code can be removed when the generic
	   *        argument check is in place.
	   */
	  struct pike_string *percent_string;
	  /* yywarning("Ignoring second argument to sprintf."); */
	  MAKE_CONST_STRING(percent_string, "%");
	  ADD_NODE_REF2(*arg1,
			ret = mknode(F_COMMA_EXPR, *arg1,
				     mkstrnode(percent_string)));
	  return ret;
	}

      default: break;
      }
    }
  }
  /* FIXME: Convert into compile_sprintf(args[0])->format(@args[1..])? */
  return ret;
}

/*! @decl type(mixed) __handle_sprintf_format(string attr, string fmt, @
 *!                                           type arg_type, type cont_type)
 *!
 *!   Type attribute handler for @expr{"sprintf_format"@}.
 *!
 *! @param attr
 *!   Attribute to handle, either @expr{"sprintf_format"@}
 *!   or @expr{"strict_sprintf_format"@}.
 *!
 *! @param fmt
 *!   Sprintf-style formatting string to generate type information from.
 *!
 *! @param arg_type
 *!   Declared type of the @[fmt] argument (typically @expr{string@}).
 *!
 *! @param cont_type
 *!   Continuation function type after the @[fmt] argument. This is
 *!   scanned for the type attribute @expr{"sprintf_args"@} to
 *!   determine where the remaining arguments to @[sprintf()] will
 *!   come from.
 *!
 *! This function is typically called from
 *! @[PikeCompiler()->apply_attribute_constant()] and is used to perform
 *! stricter compile-time argument checking of @[sprintf()]-style functions.
 *!
 *! It currently implements two operating modes depending on the value of
 *! @[attr]:
 *! @string
 *!   @value "strict_sprintf_format"
 *!     The formatting string @[fmt] is known to always be passed to
 *!     @[sprintf()].
 *!   @value "sprintf_format"
 *!     The formatting string @[fmt] is passed to @[sprintf()] only
 *!     if there are @expr{"sprintf_args"@}.
 *! @endstring
 *!
 *! @returns
 *!   Returns @[cont_type] with @expr{"sprintf_args"@} replaced by the
 *!   arguments required by the @[fmt] formatting string, and
 *!   @expr{"sprintf_result"@} replaced by the resulting string type.
 *!
 *! @seealso
 *!   @[PikeCompiler()->apply_attribute_constant()], @[sprintf()]
 */
void f___handle_sprintf_format(INT32 args)
{
  struct pike_type *res;
  struct pike_type *tmp;
  struct pike_string *attr;
  struct pike_string *fmt;
  int severity = REPORT_ERROR;
  int found = 0;
  int fmt_count;
  int marker;
  int marker_mask;

  if (args != 4)
    SIMPLE_WRONG_NUM_ARGS_ERROR("__handle_sprintf_format", 4);
  if (TYPEOF(Pike_sp[-4]) != PIKE_T_STRING)
    SIMPLE_ARG_TYPE_ERROR("__handle_sprintf_format", 1, "string");
  if (TYPEOF(Pike_sp[-3]) != PIKE_T_STRING)
    SIMPLE_ARG_TYPE_ERROR("__handle_sprintf_format", 2, "string");
  if (TYPEOF(Pike_sp[-2]) != PIKE_T_TYPE)
    SIMPLE_ARG_TYPE_ERROR("__handle_sprintf_format", 3, "type");
  if (TYPEOF(Pike_sp[-1]) != PIKE_T_TYPE)
    SIMPLE_ARG_TYPE_ERROR("__handle_sprintf_format", 4, "type");

  tmp = Pike_sp[-1].u.type;
  if ((tmp->type != PIKE_T_FUNCTION) && (tmp->type != T_MANY)) {
    SIMPLE_ARG_TYPE_ERROR("__handle_sprintf_format", 4, "type(function)");
  }

#if 0
  fprintf(stderr, "__handle_sprintf_format(\"%s\", \"%s\", ...)\n",
	  Pike_sp[-4].u.string->str, Pike_sp[-3].u.string->str);
#endif /* 0 */

  MAKE_CONST_STRING(attr, "sprintf_format");
  if (Pike_sp[-4].u.string == attr) {
    /* Don't complain so loud about syntax errors in
     * relaxed mode.
     */
    severity = REPORT_NOTICE;
  } else {
    MAKE_CONST_STRING(attr, "strict_sprintf_format");
    if (Pike_sp[-4].u.string != attr) {
      Pike_error("Bad argument 1 to __handle_sprintf_format(), expected "
		 "\"sprintf_format\" or \"strict_sprintf_format\", "
		 "got \"%S\".\n", Pike_sp[-4].u.string);
    }
  }

  /* Allocate a marker for accumulating the result type. */
  marker = '0';
  marker_mask = PT_FLAG_MARKER_0 | PT_FLAG_ASSIGN_0;
  while (tmp->flags & marker_mask) {
    marker++;
    marker_mask <<= 1;
  }
  if (marker > '9') marker = 0;

  fmt = Pike_sp[-3].u.string;
  MAKE_CONST_STRING(attr, "sprintf_args");

  type_stack_mark();
  type_stack_mark();
  for (; tmp; tmp = tmp->cdr) {
    struct pike_type *arg = tmp->car;
    int array_cnt = 0;
    while(arg) {
      switch(arg->type) {
      case PIKE_T_ATTRIBUTE:
	if (arg->car == (struct pike_type *)attr)
	  break;
	/* FALL_THROUGH */
      case PIKE_T_NAME:
	arg = arg->cdr;
	continue;
      case PIKE_T_ARRAY:
	array_cnt++;
	arg = arg->car;
	continue;
      default:
	arg = NULL;
	break;
      }
      break;
    }
    if (arg) {
      p_wchar2 min_char = 0x7fffffff;
      p_wchar2 max_char = -0x80000000;
      int ret;

      type_stack_mark();
      ret = push_sprintf_argument_types(MKPCHARP(fmt->str, fmt->size_shift),
					fmt->len, 0, marker,
					&min_char, &max_char, severity);
      switch(ret) {
      case 0:
      case PSAT_MARKER:
	/* Ok. */
	if (!array_cnt) {
	  struct pike_type *trailer;
	  MAKE_CONST_STRING(attr, "sprintf_result");
	  pop_stack_mark();
	  push_type(T_VOID);	/* No more args */
	  while (tmp->type == PIKE_T_FUNCTION) {
	    tmp = tmp->cdr;
	  }
	  trailer = tmp->cdr;	/* Return type */
	  while (trailer->type == PIKE_T_NAME) {
	    trailer = trailer->cdr;
	  }
	  if ((trailer->type == PIKE_T_ATTRIBUTE) &&
	      (trailer->car == (struct pike_type *)attr)) {
	    /* Push the derived return type. */
	    if (min_char <= max_char) {
	      push_int_type(min_char, max_char);
	      if (ret & PSAT_MARKER) {
		push_type(marker);
		push_type(T_OR);
	      }
	    } else if (ret & PSAT_MARKER) {
	      push_type(marker);
	    } else {
	      push_type(PIKE_T_ZERO);
	    }
	    push_type(PIKE_T_STRING);
	  } else {
	    push_finished_type(tmp->cdr);	/* Return type */
	  }
	  push_reverse_type(T_MANY);
	  fmt_count = pop_stack_mark();
	  while (fmt_count > 1) {
	    push_reverse_type(T_FUNCTION);
	    fmt_count--;
	  }
	  if (severity < REPORT_ERROR) {
	    /* Add the type where the fmt isn't sent to sprintf(). */
	    type_stack_mark();
	    for (arg = Pike_sp[-1].u.type; arg != tmp; arg = arg->cdr) {
	      push_finished_type(arg->car);
	    }
	    push_type(T_VOID);			/* No more args */
	    push_finished_type(tmp->cdr);	/* Return type */
	    push_reverse_type(T_MANY);
	    fmt_count = pop_stack_mark();
	    while (fmt_count > 1) {
	      push_reverse_type(T_FUNCTION);
	      fmt_count--;
	    }
	    push_type(T_OR);
	  }
	  res = pop_unfinished_type();
	  pop_n_elems(args);
	  push_type_value(res);
	  return;
	} else {
	  /* Join the argument types into the array. */
	  push_type(PIKE_T_ZERO);
	  for (fmt_count = pop_stack_mark(); fmt_count > 1; fmt_count--) {
	    push_type(T_OR);
	  }
	  while (array_cnt--) {
	    push_type(PIKE_T_ARRAY);
	  }
	  if (severity < REPORT_ERROR) {
	    push_type(T_VOID);
	    push_type(T_OR);
	  }
	  found = 1;
	}
	break;
      case -1:
	/* Syntax error. */
	if (severity < REPORT_ERROR) {
	  /* Add the type where the fmt isn't sent to sprintf(). */
	  type_stack_pop_to_mark();
	  if (array_cnt--) {
	    push_type(PIKE_T_ZERO);	/* No args */
	    while (array_cnt--) {
	      push_type(PIKE_T_ARRAY);
	      push_type(PIKE_T_ZERO);
	      push_type(T_OR);
	    }
	    push_type(T_VOID);
	    push_type(T_OR);
	    push_finished_type(tmp->cdr);	/* Rest type */
	    push_reverse_type(tmp->type);
	  } else {
	    push_type(T_VOID);	/* No more args */
	    while (tmp->type == PIKE_T_FUNCTION) {
	      tmp = tmp->cdr;
	    }
	    push_finished_type(tmp->cdr);	/* Return type */
	    push_reverse_type(T_MANY);
	  }
	  fmt_count = pop_stack_mark();
	  while (fmt_count > 1) {
	    push_reverse_type(T_FUNCTION);
	    fmt_count--;
	  }
	  res = pop_unfinished_type();
	  pop_n_elems(args);
	  push_type_value(res);
	  return;
	}
	/* FALL_THROUGH */
      case PSAT_INVALID:
      case PSAT_INVALID|PSAT_MARKER:
	/* There was a position argument or a parse error in strict mode. */
	pop_stack_mark();
	pop_stack_mark();
	type_stack_pop_to_mark();
	pop_n_elems(args);
	if (ret & PSAT_MARKER) {
	  /* Error or marker that we can't trust. */
	  push_undefined();
	} else {
	  /* Position argument, but we don't need to look at the marker. */
	  type_stack_mark();
	  if (min_char <= max_char) {
	    push_int_type(min_char, max_char);
	  } else {
	    push_type(T_ZERO);
	  }
	  push_type(T_STRING);
	  push_type(T_MIXED);
	  push_type(T_MANY);
	  push_type_value(pop_unfinished_type());
	}
	return;
      }
    } else {
      push_finished_type(tmp->car);
    }
    if (tmp->type == T_MANY) {
      tmp = tmp->cdr;
      break;
    }
  }
  if (found) {
    /* Found, but inside an array, so we need to build the function
     * type here.
     */
    push_finished_type(tmp);	/* Return type. */
    push_reverse_type(T_MANY);
    fmt_count = pop_stack_mark();
    while (fmt_count > 1) {
      push_reverse_type(T_FUNCTION);
      fmt_count--;
    }
    res = pop_unfinished_type();
    pop_n_elems(args);
    push_type_value(res);
  } else {
    /* No marker found. */
#if 0
    simple_describe_type(Pike_sp[-1].u.type);
    fprintf(stderr, " ==> No marker found.\n");
#endif /* 0 */
    pop_stack_mark();
    type_stack_pop_to_mark();
    pop_n_elems(args);
    push_undefined();
  }
}

/*! @decl constant sprintf_format = __attribute__("sprintf_format")
 *!
 *!   Type constant used for typing arguments that are optionally
 *!   sent to @[sprintf()] depending on the presence of extra arguments.
 *!
 *! @seealso
 *!   @[strict_sprintf_format], @[sprintf_args], @[sprintf()]
 */

/*! @decl constant strict_sprintf_format = @
 *!         __attribute__("strict_sprintf_format")
 *!
 *!   Type constant used for typing arguments that are always
 *!   sent to @[sprintf()] regardless of the presence of extra arguments.
 *!
 *! @seealso
 *!   @[sprintf_format], @[sprintf_args], @[sprintf()]
 */

/*! @decl constant sprintf_args = __attribute__("sprintf_args")
 *!
 *!   Type constant used for typing extra arguments that are
 *!   sent to @[sprintf()].
 *!
 *! @seealso
 *!   @[strict_sprintf_format], @[sprintf_format], @[sprintf()]
 */

/*! @decl constant sprintf_result = __attribute__("sprintf_result")
 *!
 *!   Type constant used for typing the return value from @[sprintf()].
 *!
 *! @seealso
 *!   @[strict_sprintf_format], @[sprintf_format], @[sprintf()]
 */

/*! @module String
 */

/*! @decl constant __HAVE_SPRINTF_STAR_MAPPING__ = 1
 *!
 *!   Presence of this symbol indicates that @[sprintf()] supports
 *!   mappings for the @tt{'*'@}-modifier syntax.
 *!
 *! @seealso
 *!   @[sprintf()], @[lfun::_sprintf()]
 */

/*! @decl constant __HAVE_SPRINTF_NEGATIVE_F__ = 1
 *!
 *!   Presence of this symbol indicates that @[sprintf()] supports
 *!   little endian output for the @tt{'F'@}-format specifier.
 *!
 *! @seealso
 *!   @[sprintf()], @[lfun::_sprintf()]
 */

/*! @endmodule
 */

void init_sprintf(void)
{
  struct pike_string *attr;
  struct svalue s;

  ADD_EFUN("__handle_sprintf_format", f___handle_sprintf_format,
	   tFunc(tStr tStr tType(tMix) tType(tMix), tType(tMix)),
	   0);

  MAKE_CONST_STRING(attr, "sprintf_format");
  SET_SVAL(s, T_TYPE, 0, type,
	   make_pike_type(tAttr("sprintf_format", tOr(tStr, tObj))));
  low_add_efun(attr, &s);
  free_type(s.u.type);

  MAKE_CONST_STRING(attr, "strict_sprintf_format");
  SET_SVAL(s, T_TYPE, 0, type,
	   make_pike_type(tAttr("strict_sprintf_format", tOr(tStr, tObj))));
  low_add_efun(attr, &s);
  free_type(s.u.type);

  MAKE_CONST_STRING(attr, "sprintf_args");
  s.u.type = make_pike_type(tAttr("sprintf_args", tMix));
  low_add_efun(attr, &s);
  free_type(s.u.type);

  MAKE_CONST_STRING(attr, "sprintf_result");
  s.u.type = make_pike_type(tAttr("sprintf_result", tStr));
  low_add_efun(attr, &s);
  free_type(s.u.type);

  /* function(string|object, mixed ... : string) */
  ADD_EFUN2("sprintf",
	    f_sprintf,
	    tFuncV(tAttr("strict_sprintf_format", tOr(tStr, tObj)),
		   tAttr("sprintf_args", tMix), tAttr("sprintf_result", tStr)),
	    OPT_TRY_OPTIMIZE,
	    optimize_sprintf,
	    0);
}

void exit_sprintf(void)
{
}
