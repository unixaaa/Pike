/*\
||| This file a part of Pike, and is copyright by Fredrik Hubinette
||| Pike is distributed as GPL (General Public License)
||| See the files COPYING and DISCLAIMER for more information.
\*/
#ifndef FILE_H
#define FILE_H

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
#ifndef ARPA_INET_H
#include <arpa/inet.h>
#define ARPA_INET_H

/* Stupid patch to avoid trouble with Linux includes... */
#ifdef LITTLE_ENDIAN
#undef LITTLE_ENDIAN
#endif

#endif
#endif

struct my_file
{
  INT32 refs;
  int fd;
  short open_mode;
  struct svalue id;
  struct svalue read_callback;
  struct svalue write_callback;
  struct svalue close_callback;
};

/* Prototypes begin here */
struct object *file_make_object_from_fd(int fd, int mode);
int socketpair(int family, int type, int protocol, int sv[2]);
void get_inet_addr(struct sockaddr_in *addr,char *name);
void exit_files();
void init_files_programs();
/* Prototypes end here */

#define FILE_READ 1
#define FILE_WRITE 2
#define FILE_APPEND 4
#define FILE_CREATE 8
#define FILE_TRUNC 16
#define FILE_EXCLUSIVE 32

#endif
