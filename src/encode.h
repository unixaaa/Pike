/*\
||| This file is part of Pike. For copyright information see COPYRIGHT.
||| Pike is distributed under GPL, LGPL and MPL. See the file COPYING
||| for more information.
||| $Id: encode.h,v 1.6 2002/10/08 20:22:20 nilsson Exp $
\*/

#ifndef ENCODE_H
#define ENCODE_H

/* Prototypes begin here */
struct encode_data;
void f_encode_value(INT32 args);
void f_encode_value_canonic(INT32 args);
struct decode_data;
void f_decode_value(INT32 args);
/* Prototypes end here */

#endif
