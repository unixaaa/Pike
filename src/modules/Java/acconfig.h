/*\
||| This file is part of Pike. For copyright information see COPYRIGHT.
||| Pike is distributed under GPL, LGPL and MPL. See the file COPYING
||| for more information.
||| $Id: acconfig.h,v 1.3 2002/10/08 20:22:33 nilsson Exp $
\*/

/*
 * Config-file for the Pike Java module.
 *
 * Marcus Comstedt
 */

#ifndef PIKE_JAVA_CONFIG_H
#define PIKE_JAVA_CONFIG_H

@TOP@
@BOTTOM@

/* Define if you have Java */
#undef HAVE_JAVA

/* Define to home of Java */
#undef JAVA_HOME

/* Define to the library path for Java libraries */
#undef JAVA_LIBPATH

/* Define if you have a Sparc CPU */
#undef HAVE_SPARC_CPU

/* Define if you have an x86 CPU */
#undef HAVE_X86_CPU

/* Define if you have a PowerPC CPU */
#undef HAVE_PPC_CPU

#endif /* PIKE_JAVA_CONFIG_H */
