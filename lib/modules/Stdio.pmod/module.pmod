// $Id: module.pmod,v 1.110 2001/07/19 18:52:49 david%hedbor.org Exp $
#pike __REAL_VERSION__


import String;

inherit files;

//! The Stdio.Stream API.
//!
//! This class exists purely for typing reasons.
//!
//! Use in types in place of @[Stdio.File] where only blocking stream-oriented
//! I/O is done with the object.
//!
//! @seealso
//! @[NonblockingStream], @[BlockFile], @[File], @[FILE]
//!
class Stream
{
  string read(int nbytes);
  int write(string data);
  void close();

#if constant(files.__HAVE_OOB__)
  optional string read_oob(int nbytes);
  optional int write_oob(string data);
#endif

  static string _sprintf( int type )
  {
    switch( type )
    {
     case 'O':
       return sprintf("%t()", this_object());
     case 't':
       return "Stdio.Stream";
    }
  }
}

//! The Stdio.NonblockingStream API.
//!
//! This class exists purely for typing reasons.
//!
//! Use in types in place of @[Stdio.File] where nonblocking and/or blocking
//! stream-oriented I/O is done with the object.
//! 
//! @seealso
//! @[Stream], @[BlockFile], @[File], @[FILE]
//!
class NonblockingStream
{
  inherit Stream;
  NonblockingStream set_read_callback( function f, mixed ... rest );
  NonblockingStream set_write_callback( function f, mixed ... rest );
  NonblockingStream set_close_callback( function f, mixed ... rest );

#if constant(files.__HAVE_OOB__)
  NonblockingStream set_read_oob_callback(function f, mixed ... rest)
  {
    error("OOB not implemented for this stream type\n");
  }
  
  NonblockingStream set_write_oob_callback(function f, mixed ... rest)
  {
    error("OOB not implemented for this stream type\n");
  }
#endif

  void set_nonblocking( function a, function b, function c,
                        function|void d, function|void e);
  void set_blocking();

  static string _sprintf( int type )
  {
    switch( type )
    {
     case 't': return "Stdio.NonblockingStream";
    }
    return ::_sprintf( type );
  }
}

//! The Stdio.BlockFile API.
//!
//! This class exists purely for typing reasons.
//!
//! Use in types in place of @[Stdio.File] where only blocking
//! I/O is done with the object.
//! 
//! @seealso
//! @[Stream], @[NonblockingStream], @[File], @[FILE]
//!
class BlockFile
{
  inherit Stream;
  int seek(int to);
  int tell();
  static string _sprintf( int type )
  {
    switch( type )
    {
     case 't': return "Stdio.BlockFile";
    }
    return ::_sprintf( type );
  }
}

#ifdef TRACK_OPEN_FILES
// Debug tool to track down where a file is currently opened from.
// It's used primarily when debugging on NT since an opened file can't
// be renamed or removed there.

static mapping(string|int:array) open_files = ([]);
static int next_open_file_id = 1;

void register_open_file (string file, int id, array backtrace)
{
  file = combine_path (getcwd(), file);
  open_files[id] =
    ({file, describe_backtrace (backtrace[..sizeof (backtrace) - 2])});
  if (!open_files[file]) open_files[file] = ({id});
  else open_files[file] += ({id});
}

void register_close_file (int id)
{
  if (open_files[id]) {
    string file = open_files[id][0];
    open_files[file] -= ({id});
    if (!sizeof (open_files[file])) m_delete (open_files, file);
    m_delete (open_files, id);
  }
}

array(string) file_open_places (string file)
{
  file = combine_path (getcwd(), file);
  if (array(int) ids = open_files[file])
    return map (ids, lambda (int id) {
		       if (array ent = open_files[id])
			 return ent[1];
		     }) - ({0});
  return 0;
}

void report_file_open_places (string file)
{
  array(string) places = file_open_places (file);
  if (places)
    werror ("File " + file + " is currently opened from:\n" +
	    map (places,
		 lambda (string place) {
		   return " * " +
		     replace (place[..sizeof (place) - 2], "\n", "\n   ");
		 }) * "\n" + "\n");
  else
    werror ("File " + file + " is currently not opened anywhere.\n");
}
#else
#define register_open_file(file, id, backtrace)
#define register_close_file(id)
#endif

//! This is the basic I/O object, it provides socket communication as well
//! as file access. It does not buffer reads and writes or provide line-by-line
//! reading, that is done with @[Stdio.FILE] object.
class File
{
  inherit Fd_ref;

#ifdef TRACK_OPEN_FILES
  static int open_file_id = next_open_file_id++;
#endif

  int is_file;

  mixed ___read_callback;
  mixed ___write_callback;
  mixed ___close_callback;
#if constant(files.__HAVE_OOB__)
  mixed ___read_oob_callback;
  mixed ___write_oob_callback;
#endif
  mixed ___id;

#ifdef __STDIO_DEBUG
  string __closed_backtrace;
#define CHECK_OPEN()								\
    if(!_fd)									\
    {										\
      throw(({									\
	"Stdio.File(): line "+__LINE__+" on closed file.\n"+			\
	  (__closed_backtrace ? 						\
	   sprintf("File was closed from:\n    %-=200s\n",__closed_backtrace) :	\
	   "This file has never been open.\n" ),				\
	  backtrace()}));							\
      										\
    }
#else
#define CHECK_OPEN()
#endif

  //! Returns the error code for the last command on this file.
  //! Error code is normally cleared when a command is successful.
  //!
  int errno()
  {
    return _fd && ::errno();
  }

  static string debug_file;
  static string debug_mode;
  static int debug_bits;

  optional void _setup_debug( string f, string m, int|void b )
  {
    debug_file = f;
    debug_mode = m;
    debug_bits = b;
  }

  static string _sprintf( int type, mapping flags )
  {
    int do_query_fd( )
    {
      int fd = -1;
      catch{ fd = query_fd(); };
      return fd;
    };
    switch( type )
    {
     case 'O':
       return sprintf("%t(%O, %O, %o /* fd=%d */)", 
                      this_object(), 
                      debug_file, debug_mode,
                      debug_bits||0777,
                      do_query_fd() );
     case 't':
       return "Stdio.File";
    }
  }

  //! @decl int open(string filename, string mode)
  //! @decl int open(string filename, string mode, int mask)
  //!
  //! Open a file for read, write or append. The parameter @[mode] should
  //! contain one or more of the following letters:
  //! @string
  //!   @member 'r'
  //!   Open file for reading.
  //!   @member 'w'
  //!   Open file for writing.
  //!   @member 'a'
  //!   Open file for append (use with @tt{'w'@}).
  //!   @member 't'
  //!   Truncate file at open (use with @tt{'w'@}).
  //!   @member 'c'
  //!   Create file if it doesn't exist (use with @tt{'w'@}).
  //!   @member 'x'
  //!   Fail if file already exists (use with @tt{'c'@}).
  //! @endstring
  //!
  //! @[mode] should always contain at least one of the letters @tt{'r'@} or
  //! @tt{'w'@}.
  //!
  //! The parameter @[mask] is protection bits to use if the file is created.
  //! Default is @tt{0666@} (read+write for all in octal notation).
  //!
  //! @returns
  //! This function returns 1 for success, 0 otherwise.
  //!
  //! @seealso
  //! @[close()], @[create()]
  //!
  int open(string file, string mode, void|int bits)
  {
    _fd=Fd();
    register_open_file (file, open_file_id, backtrace());
    is_file = 1;
#ifdef __STDIO_DEBUG
    __closed_backtrace=0;
#endif
    if(query_num_arg()<3) bits=0666;
    debug_file = file;  debug_mode = mode;
    debug_bits = bits;
    return ::open(file,mode,bits);
  }

  //! This makes this file into a socket ready for connections. The reason
  //! for this function is so that you can set the socket to nonblocking
  //! or blocking (default is blocking) before you call @[connect()].
  //!
  //! If you give a @[port] number to this function, the socket will be
  //! bound to this @[port] locally before connecting anywhere. This is
  //! only useful for some silly protocols like @b{FTP@}. You may also
  //! specify an @[address] to bind to if your machine has many IP numbers.
  //!
  //! @returns
  //! This function returns 1 for success, 0 otherwise.
  //!
  //! @seealso
  //! @[connect()], @[set_nonblocking()], @[set_blocking()]
  //!
  int open_socket(int|void port, string|void address)
  {
    _fd=Fd();
    is_file = 0;
#ifdef __STDIO_DEBUG
    __closed_backtrace=0;
#endif
    debug_file="socket";
    debug_mode=0; debug_bits=0;
    switch(query_num_arg()) {
    case 0:
      return ::open_socket();
    case 1:
      return ::open_socket(port);
    default:
      return ::open_socket(port, address);
    }
  }

  //! This function connects a socket previously created with
  //! @[open_socket()] to a remote socket. The @[host] argument is the
  //! hostname or IP number for the remote machine.
  //!
  //! @returns
  //! This function returns 1 for success, 0 otherwise.
  //!
  //! @note
  //! If the socket is in nonblocking mode, you have to wait for a write
  //! or close callback before you know if the connection failed or not.
  //!
  //! @seealso
  //! @[query_address()]
  //!
  int connect(string host, int port)
  {
    if(!_fd) _fd=Fd();
#ifdef __STDIO_DEBUG
    __closed_backtrace=0;
#endif
    is_file = 0;
    debug_file = "socket";
    debug_mode = host+":"+port; 
    debug_bits = 0;
    return ::connect(host, port);
  }

  static private function(int, mixed ...:void) _async_cb;
  static private array(mixed) _async_args;
  static private void _async_connected(mixed|void ignored)
  {
    // Copy the args to avoid races.
    if(_async_cb) return;
    function(int, mixed ...:void) cb = _async_cb;
    array(mixed) args = _async_args;
    _async_cb = 0;
    _async_args = 0;
    set_nonblocking(0,0,0);
    cb(1, @args);
  }
  static private void _async_failed(mixed|void ignored)
  {
    // Copy the args to avoid races.
    if(_async_cb) return;
    function(int, mixed ...:void) cb = _async_cb;
    array(mixed) args = _async_args;
    _async_cb = 0;
    _async_args = 0;
    set_nonblocking(0,0,0);
    cb(0, @args);
  }


  //! FIXME:
  //!   Document this function.
  //!
  //! @note
  //! Zaps nonblocking-state.
  int async_connect(string host, int port,
		    function(int, mixed ...:void) callback,
		    mixed ... args)
  {
    if (!_fd && !open_socket()) {
      // Out of sockets?
      return 0;
    }
    _async_cb = callback;
    _async_args = args;
    set_nonblocking(0, _async_connected, _async_failed, _async_failed, 0);
    mixed err;
    int res;
    if (err = catch(res = connect(host, port))) {
      // Illegal format. -- Bad hostname?
      call_out(_async_failed, 0);
    } else if (!res) {
      // Connect failed.
      call_out(_async_failed, 0);
    }
    return(1);	// OK so far. (Or rather the callback will be used).
  }

  //! This function creates a bi-directional pipe between the object it
  //! was called in and an object that is returned. The two ends of the
  //! pipe are indistinguishable. If the File object this function is
  //! called in was open to begin with, it will be closed before the pipe
  //! is created.
  //!
  //! FIXME:
  //! Document the @tt{PROP_@} properties.
  //!
  //! @seealso
  //! @[Process.create_process()]
  //!
  File pipe(void|int how)
  {
    _fd=Fd();
#ifdef __STDIO_DEBUG
    __closed_backtrace=0;
#endif
    is_file = 0;
    if(query_num_arg()==0)
      how=PROP_NONBLOCK | PROP_BIDIRECTIONAL;
    if(Fd fd=[object(Fd)]::pipe(how))
    {
      File o=File();
      o->_fd=fd;
      o->_setup_debug( "pipe", 0 );
      return o;
    }else{
      return 0;
    }
  }

  //! @decl void create()
  //! @decl void create(string filename)
  //! @decl void create(string filename, string mode)
  //! @decl void create(string filename, string mode, int mask)
  //! @decl void create(string descriptorname)
  //! @decl void create(int fd)
  //! @decl void create(int fd, string mode)
  //!
  //! There are four basic ways to create a Stdio.File object.
  //! The first is calling it without any arguments, in which case the you
  //! have to call @[open()], @[connect()] or some other method which connects
  //! the File object with a stream.
  //!
  //! The second way is calling it with a @[filename] and open @[mode]. This is
  //! the same thing as cloning and then calling @[open()], except shorter and
  //! faster.
  //!
  //! The third way is to call it with @[descriptorname] of @tt{"stdin"@},
  //! @tt{"stdout"@} or @tt{"stderr"@}. This will open the specified
  //! standard stream.
  //!
  //! For the advanced users, you can use the file descriptors of the
  //! systems (note: emulated by pike on some systems - like NT). This is
  //! only useful for streaming purposes on unix systems. This is @b{not
  //! recommended at all@} if you don't know what you're into. Default
  //! @[mode] for this is @tt{"rw"@}.
  //!
  //! @note
  //! Open mode will be filtered through the system UMASK. You
  //! might need to use @[chmod()] later.
  //!
  //! @seealso
  //! @[open()], @[connect()], @[Stdio.FILE],
  void create(int|string|void file,void|string mode,void|int bits)
  {
    if (zero_type(file)) return;
    debug_file = file;  
    debug_mode = mode;
    debug_bits = bits;
    switch(file)
    {
      case "stdin":
	_fd=_stdin;
#ifdef __STDIO_DEBUG
	__closed_backtrace=0;
#endif
	break; /* ARGH, this missing break took 6 hours to find! /Hubbe */

      case "stdout":
	_fd=_stdout;
#ifdef __STDIO_DEBUG
	__closed_backtrace=0;
#endif
	break;
	
      case "stderr":
	_fd=_stderr;
#ifdef __STDIO_DEBUG
	__closed_backtrace=0;
#endif
	break;

      case 0..0x7fffffff:
	 if (!mode) mode="rw";
	_fd=Fd(file,mode);
	register_open_file ("", open_file_id, backtrace());
#ifdef __STDIO_DEBUG
	__closed_backtrace=0;
#endif
	break;

      default:
	_fd=Fd();
	register_open_file (file, open_file_id, backtrace());
	is_file = 1;
#ifdef __STDIO_DEBUG
	__closed_backtrace=0;
#endif
	if(query_num_arg()<3) bits=0666;
	if(!mode) mode="r";
	if (!::open(file,mode,bits))
	   error("Failed to open %O mode %O : %s\n",
		 file,mode,strerror(errno()));
    }
  }

  //! This function takes a clone of Stdio.File and assigns all
  //! variables of this file from it. It can be used together with @[dup()]
  //! to move files around.
  //!
  //! @seealso
  //! @[dup()]
  //!
  int assign(File|Fd o)
  {
    is_file = o->is_file;
    if((program)Fd == (program)object_program(o))
    {
      _fd = o->dup();
    }else{
      File _o = [object(File)]o;
      _fd = _o->_fd;
      if(___read_callback = _o->___read_callback)
	_fd->_read_callback=__stdio_read_callback;

      if(___write_callback = _o->___write_callback)
	_fd->_write_callback=__stdio_write_callback;

      ___close_callback = _o->___close_callback;
#if constant(files.__HAVE_OOB__)
      if(___read_oob_callback = _o->___read_oob_callback)
	_fd->_read_oob_callback = __stdio_read_oob_callback;

      if(___write_oob_callback = _o->___write_oob_callback)
	_fd->_write_oob_callback = __stdio_write_oob_callback;
#endif
      ___id = _o->___id;
      
    }
    return 0;
  }

  //! This function returns a clone of Stdio.File with all variables
  //! copied from this file.
  //!
  //! @note
  //! All variables, even @tt{id@}, are copied.
  //!
  //! @seealso
  //! @[assign()]
  File dup()
  {
    File to = File();
    to->is_file = is_file;
    to->_fd = _fd;
    if(to->___read_callback = ___read_callback)
      _fd->_read_callback=to->__stdio_read_callback;

    if(to->___write_callback = ___write_callback)
      _fd->_write_callback=to->__stdio_write_callback;

    to->___close_callback = ___close_callback;
#if constant(files.__HAVE_OOB__)
    if(to->___read_oob_callback = ___read_oob_callback)
      _fd->_read_oob_callback=to->__stdio_read_oob_callback;

    if(to->___write_oob_callback = ___write_oob_callback)
      _fd->_write_oob_callback=to->__stdio_write_oob_callback;
#endif
    to->_setup_debug( debug_file, debug_mode, debug_bits );
    to->___id = ___id;
    return to;
  }


  //! @decl int close()
  //! @decl int close(string how)
  //!
  //! Close the file. Optionally, specify "r", "w" or "rw" to close just
  //! the read, just the write or both read and write part of the file
  //! respectively.
  //!
  //! @note
  //! This function will not call the @tt{close_callback@}.
  //!
  //! @seealso
  //! @[open]
  //!
  int close(void|string how)
  {
    if(::close(how||"rw"))
    {
#define FREE_CB(X) if(___##X && query_##X == __stdio_##X) ::set_##X(0)
      FREE_CB(read_callback);
      FREE_CB(write_callback);
#if constant(files.__HAVE_OOB__)
      FREE_CB(read_oob_callback);
      FREE_CB(write_oob_callback);
#endif
      _fd=0;
      register_close_file (open_file_id);
#ifdef __STDIO_DEBUG
      __closed_backtrace=master()->describe_backtrace(backtrace());
#endif
    }
    return 1;
  }

  static private int peek_file_before_read_callback=0;

  this_program set_peek_file_before_read_callback(int(0..1) to)
  {      
     peek_file_before_read_callback=to;
     return this_object();
  }

  // FIXME: No way to specify the maximum to read.
  static void __stdio_read_callback()
  {

/*
** 
** nothing to read happens if you do:
**  o open a socket
**  o set_read_callback
**  o make sure something is to read on the socket
**    and on another socket
** in that sockets read callback, do
**  o read all from the first socket
**  o finish callback
** 
** We still need to read 0 bytes though, to get the next callback.
** 
** But peek lowers performance significantly.
** Check if we need it first, and set this flag manually.
** 
** FIXME for NT or internally? /Mirar
** 
*/

#if !defined(__NT__)
    if (peek_file_before_read_callback)
       if (!::peek()) 
       {
//  	  werror("bwah\n");
//  	  _exit(1);
	  ::read(0,1);
	  return; // nothing to read
       }
#endif

#if defined(__STDIO_DEBUG) && !defined(__NT__)
    if(!::peek())
      throw( ({"Read callback with no data to read!\n",backtrace()}) );
#endif

    string s=::read(8192,1);
    if(s)
    {
      if(strlen(s))
      {
        ___read_callback(___id, s);
        return;
      }
    }else{
      switch(errno())
      {
#if constant(system.EINTR)
         case system.EINTR:
#endif

#if constant(system.EWOULDBLOCK)
	 case system.EWOULDBLOCK:
#endif
	   ::set_read_callback(__stdio_read_callback);
           return;
      }
    }
    ::set_read_callback(0);
    if (___close_callback) {
      ___close_callback(___id);
    }
  }

  static void __stdio_write_callback() { ___write_callback(___id); }

#if constant(files.__HAVE_OOB__)
  static void __stdio_read_oob_callback()
  {
    string s=::read_oob(8192,1);
    if(s && strlen(s))
    {
      ___read_oob_callback(___id, s);
    }else{
      ::set_read_oob_callback(0);
      if (___close_callback) {
	___close_callback(___id);
      }
    }
  }

  static void __stdio_write_oob_callback() { ___write_oob_callback(___id); }
#endif

#define SET(X,Y) ::set_##X ((___##X = (Y)) && __stdio_##X)
#define _SET(X,Y) _fd->_##X=(___##X = (Y)) && __stdio_##X

#define CBFUNC(X)					\
  void set_##X (mixed l##X)				\
  {							\
    CHECK_OPEN();                                       \
    SET( X , l##X );					\
  }							\
							\
  mixed query_##X ()					\
  {							\
    return ___##X;					\
  }

  //! @decl void set_read_callback(function(mixed, string:void) read_cb)
  //!
  //! This function sets the @tt{read_callback@} for the file. The
  //! @tt{read_callback@} is called whenever there is data to read from
  //! the file.
  //!
  //! @note
  //! This function does not set the file nonblocking.
  //!
  //! @seealso
  //! @[set_nonblocking()], @[read()],
  //! @[query_read_callback()], @[set_write_callback()],
  //! @[set_close_callback()], @[set_read_oob_callback]
  //! @[set_write_oob_callback()]

  //! @decl function(mixed, string:void) query_read_callback()
  //!
  //! This function returns the @tt{read_callback@}, which has been set with
  //! @[set_nonblocking()] or @[set_read_callback()].
  //!
  //! @seealso
  //! @[set_nonblocking()], @[set_read_callback]

  CBFUNC(read_callback)

  //! @decl void set_write_callback(function(mixed:void) write_cb)
  //!
  //! This function sets the @tt{write_callback@} for the file. The
  //! @tt{write_callback@} is called whenever there is buffer space
  //! available to write to for the file.
  //!
  //! @note
  //! This function does not set the file nonblocking.
  //!
  //! @seealso
  //! @[set_nonblocking()], @[write()],
  //! @[query_write_callback()], @[set_read_callback()],
  //! @[set_close_callback()], @[set_read_oob_callback]
  //! @[set_write_oob_callback()]

  //! @decl function(mixed:void) query_write_callback()
  //!
  //! This function returns the @tt{write_callback@}, which has been set with
  //! @[set_nonblocking()] or @[set_write_callback()].
  //!
  //! @seealso
  //! @[set_nonblocking()], @[set_write_callback]

  CBFUNC(write_callback)

  //! @decl void set_read_oob_callback(function(mixed, string:void) read_oob_cb)
  //!
  //! FIXME:
  //!   Document this function.

  //! @decl function(mixed, string:void) query_read_oob_callback()
  //!
  //! FIXME:
  //!   Document this function.

  //! @decl void set_write_oob_callback(function(mixed:void) write_oob_cb)
  //!
  //! FIXME:
  //!   Document this function.

  //! @decl function(mixed:void) query_write_oob_callback()
  //!
  //! FIXME:
  //!   Document this function.

#if constant(files.__HAVE_OOB__)
  CBFUNC(read_oob_callback)
  CBFUNC(write_oob_callback)
#endif

  //! @decl void set_close_callback(function(mixed:void) close_cb)
  //!
  //! This function sets the @tt{close_callback@} for the file. The
  //! @tt{close callback@} is called when the remote end of a socket or
  //! pipe is closed.
  //!
  //! @note
  //! This function does not set the file nonblocking.
  //!
  //! @seealso
  //! @[set_nonblocking()], @[close]
  //! @[query_close_callback()], @[set_read_callback()],
  //! @[set_write_callback()]
  //!
  void set_close_callback(mixed c)  { ___close_callback=c; }

  //! @decl function(mixed:void) query_close_callback()
  //!
  //! This function returns the @tt{close_callback@}, which has been set with
  //! @[set_nonblocking()] or @[set_close_callback()].
  //!
  //! @seealso
  //! @[set_nonblocking()], @[set_close_callback()]
  //!
  mixed query_close_callback()  { return ___close_callback; }

  //! This function sets the @tt{id@} of this file. The @tt{id@} is mainly
  //! used as an identifier that is sent as the first argument to all
  //! callbacks. The default @tt{id@} is @tt{0@} (zero). Another possible
  //! use of the @tt{id@} is to hold all data related to this file in a
  //! mapping or array.
  //!
  //! @seealso
  //! @[query_id()]
  //!
  void set_id(mixed id) { ___id=id; }

  //! This function returns the @tt{id@} that has been set with @[set_id()].
  //!
  //! @seealso
  //! @[set_id()]
  //!
  mixed query_id() { return ___id; }

  //! @decl void set_nonblocking(function(mixed, string:void) read_callback,
  //!                            function(mixed:void) write_callback,
  //!                            function(mixed:void) close_callback)
  //! @decl void set_nonblocking(function(mixed, string:void) read_callback,
  //!                            function(mixed:void) write_callback,
  //!                            function(mixed:void) close_callback,
  //!                            function(mixed, string:void) read_oob_callback,
  //!                            function(mixed:void) write_oob_callback)
  //! @decl void set_nonblocking()
  //!
  //! This function sets a stream to nonblocking mode. When data arrives on
  //! the stream, @[read_callback] will be called with some or all of this
  //! data. When the stream has buffer space over for writing,
  //! @[write_callback] will be called so that you can write more data to it.
  //! If the stream is closed at the other end, @[close_callback] will be
  //! called. 
  //!
  //! When out-of-band data arrives on the stream, @[read_oob_callback] will
  //! be called with some or all of this data. When the stream allows
  //! out-of-band data to be sent, @[write_oob_callback] will be called so that
  //! you can write out-of-band data to it.
  //!
  //! All callbacks will have the id of file as first argument when called
  //! (see @[set_id()]).
  //!
  //! If no arguments are given, the callbacks will not be changed. The
  //! stream will just be set to nonblocking mode.
  //!
  //! @note
  //! Out-of-band data will note be supported if Pike was compiled with the
  //! option @tt{'--without-oob'@}.
  //!
  //! @seealso
  //! @[set_blocking()], @[set_id()]
  //!
  void set_nonblocking(mixed|void rcb,
		       mixed|void wcb,
		       mixed|void ccb,
#if constant(files.__HAVE_OOB__)
		       mixed|void roobcb,
		       mixed|void woobcb
#endif
    )
  {
    CHECK_OPEN();
    ::_disable_callbacks(); // Thread safing

    _SET(read_callback,rcb);
    _SET(write_callback,wcb);
    ___close_callback=ccb;

#if constant(files.__HAVE_OOB__)
    _SET(read_oob_callback,roobcb);
    _SET(write_oob_callback,woobcb);
#endif
#ifdef __STDIO_DEBUG
    if(mixed x=catch { ::set_nonblocking(); })
    {
      x[0]+=(__closed_backtrace ? 
	   sprintf("File was closed from:\n    %-=200s\n",__closed_backtrace) :
	   "This file has never been open.\n" )+
	(_fd?"_fd is nonzero\n":"_fd is zero\n");
      throw(x);
    }
#else
    ::set_nonblocking();
#endif
    ::_enable_callbacks();

  }

  //! This function sets a stream to blocking mode. i.e. all reads and writes
  //! will wait until data has been transferred before returning.
  //!
  //! @seealso
  //! @[set_nonblocking()]
  //!
  void set_blocking()
  {
    CHECK_OPEN();
    ::_disable_callbacks(); // Thread safing
    SET(read_callback,0);
    SET(write_callback,0);
    ___close_callback=0;
#if constant(files.__HAVE_OOB__)
    SET(read_oob_callback,0);
    SET(write_oob_callback,0);
#endif
    ::set_blocking();
    ::_enable_callbacks();
  }

  static void destroy()
  {
    if(_fd)
    { 
      FREE_CB(read_callback);
      FREE_CB(write_callback);
#if constant(files.__HAVE_OOB__)
      FREE_CB(read_oob_callback);
      FREE_CB(write_oob_callback);
#endif
    }
    register_close_file (open_file_id);
  }
};

class Port
{
  inherit _port;

  static int|string debug_port;
  static string debug_ip;

  static string _sprintf( int f )
  {
    switch( f )
    {
     case 't':
       return "Stdio.Port";
     case 'O':
       return sprintf( "%t(%s:%O)", this_object(), debug_ip, debug_port );
    }
  }

  //! @decl void create()
  //! @decl void create(int port)
  //! @decl void create(int port, function accept_callback)
  //! @decl void create(int port, function accept_callback, string ip)
  //! @decl void create("stdin")
  //! @decl void create("stdin", function accept_callback)
  //!
  //! If the first argument is other than @tt{"stdin"@} the arguments will
  //! be passed to @[bind()].
  //!
  //! When create is called with @tt{"stdin"@} as the first argument, a
  //! socket is created out of the file descriptor @tt{0@}. This is only
  //! useful if that actually is a socket to begin with.
  //!
  //! @seealso
  //! @[bind]
  void create( string|int|void p,
               void|mixed cb,
               string|void ip )
  {
    debug_ip = (ip||"ANY");
    debug_port = p;

    if( cb )
      if( ip )
        ::create( p, cb, ip );
      else
        ::create( p, cb );
    else
      ::create( p );
  }

  //! This function completes a connection made from a remote machine to
  //! this port. It returns a two-way stream in the form of a clone of
  //! Stdio.File. The new file is by default set to blocking mode.
  //!
  //! @seealso
  //! @[Stdio.File]
  //!
  File accept()
  {
    if(object x=::accept())
    {
      File y=File();
      y->_fd=x;
      y->_setup_debug( "socket", x->query_address() );
      return y;
    }
    return 0;
  }
}

object stderr=File("stderr");
object stdout=File("stdout");

#define error(X) throw( ({ (X), backtrace()[0..sizeof(backtrace())-2] }) )
class FILE
{
#define BUFSIZE 8192
  inherit File : file;

  /* Private functions / buffers etc. */

  private string b="";
  private int bpos=0, lp, do_lines;
  private array cached_lines = ({});

  static string _sprintf( int type, mapping flags )
  {
    if( type == 't' )
      return "Stdio.FILE";
    return ::_sprintf( type, flags );
  }

  inline private static nomask int get_data()
  {
    if( bpos )
    {
      b = b[ bpos .. ];
      bpos=0;
    }
    string s = file::read(BUFSIZE,1);
    if(s && strlen(s))
      b+=s;
    else
      s = 0;
    if( do_lines && (!sizeof( cached_lines ) || s) )
    {
      cached_lines = b/"\n";
      lp = 0;
      return 1;
    } 
    return s&&1;
  }

  inline private static nomask string extract(int bytes, int|void skip)
  {
    string s;
    s=b[bpos..bpos+bytes-1];
    bpos += bytes+skip;
    return s;
  }

  /* Public functions. */

  //! Read one line of input.
  //!
  //! @returns
  //! This function returns the line read if successful, and @tt{0@} if
  //! no more lines are available.
  //!
  //! @seealso
  //! @[ngets()], @[read()]
  //!
  string gets()
  {
    if( sizeof( cached_lines ) > lp+1 )
    {
      string r = cached_lines[ lp++ ];
      return (bpos += sizeof( r  )+1),r;
    }
    do_lines = 1;
    if( !get_data() )
    {
      if( sizeof( cached_lines ) > lp && cached_lines[lp] != "" )
	return cached_lines[lp++];
      cached_lines = ({});
      lp = 0;
      return 0;
    }
    return gets();
  }

  int seek(int pos)
  {
    bpos=0;  b=""; cached_lines = ({});
    return file::seek(pos);
  }

  int tell()
  {
    return file::tell()-sizeof(b)+bpos;
  }

  int close(void|string mode)
  {
    bpos=0; b="";
    if(!mode) mode="rw";
    file::close(mode);
  }

  int open(string file, void|string mode)
  {
    bpos=0; b="";
    if(!mode) mode="rwc";
    return file::open(file,mode);
  }

  int open_socket()
  {
    bpos=0;  b="";
    return file::open_socket();
  }

  array(string) ngets(void|int(1..) n)
  {
    cached_lines = ({}); lp=0;
    if (!n) return read()/"\n";

    array res=b[bpos..]/"\n";
    bpos=strlen(b)-strlen(res[-1]);
    res=res[..sizeof(res)-2];

    while (sizeof(res)<n)
    {
      if (!get_data()) 
	if (string s=gets()) return res+({s});
	else if (!sizeof(res)) return 0;
	else return res;

      array a=b[bpos..]/"\n";
      bpos=strlen(b)-strlen(a[-1]);
      res+=a[..sizeof(a)-2];
    }
    if (sizeof(res)>n)
    {
      bpos-=`+(@map(res[n..],strlen))+(sizeof(res)-n);
      return res[..n-1];
    }
    return res;
  }

  object pipe(void|int flags)
  {
    bpos=0; cached_lines=({}); lp=0;
    b="";
    return query_num_arg() ? file::pipe(flags) : file::pipe();
  }

  
  int assign(object foo)
  {
    bpos=0; cached_lines=({}); lp=0;
    b="";
    return ::assign(foo);
  }

  FILE dup()
  {
    FILE o=FILE();
    o->assign(this_object());
    return o;
  }

  void set_nonblocking()
  {
    error("Cannot use nonblocking IO with buffered files.\n");
  }

  //! This function does approximately the same as:
  //! @tt{write(sprintf(format,@data))@}.
  //!
  //! @seealso
  //! @[write()], @[sprintf()]
  //!
  int printf(string format, mixed ... data)
  {
    return ::write(format,@data);
  }
    
  string read(int|void bytes,void|int(0..1) now)
  {
    cached_lines = ({}); lp = do_lines = 0;
    if (!query_num_arg()) {
      bytes = 0x7fffffff;
    }

    /* Optimization - Hubbe */
    if(!strlen(b) && bytes > BUFSIZE)
      return ::read(bytes, now);

    while(strlen(b) - bpos < bytes)
      if(!get_data()) {
	// EOF.
	string res = b[bpos..];
	b = "";
	bpos = 0;
	return res;
      }
      else if (now) break;

    return extract(bytes);
  }

  //! This function puts a string back in the input buffer. The string
  //! can then be read with eg @[read()], @[gets()] or @[getchar()].
  //!
  //! @seealso
  //! @[read()], @[gets()], @[getchar()]
  //!
  void ungets(string s)
  {
     cached_lines = ({}); lp=0;
     b=s+"\n"+b[bpos..];
     bpos=0;
  }

  //! This function returns one character from the input stream.
  //!
  //! @returns
  //! Returns the ASCII value of the character.
  //!
  //! @note
  //! Returns an @tt{int@} and not a @tt{string@} of length 1.
  //!
  int getchar()
  {
    cached_lines = ({});lp=0;
    if(strlen(b) - bpos < 1)
      if(!get_data())
	return -1;

    return b[bpos++];
  }
};

FILE stdin=FILE("stdin");

//! @decl string read_file(string filename)
//! @decl string read_file(string filename, int start, int len)
//!
//! Read @[len] lines from a file @[filename] after skipping @[start] lines
//! and return those lines as a string. If both @[start] and @[len] are omitted
//! the whole file is read.
//!
//! @seealso
//! @[read_bytes()], @[write_file()]
//!
string read_file(string filename,void|int start,void|int len)
{
  FILE f;
  string ret, tmp;
  f=FILE();
  if(!f->open(filename,"r")) return 0;

  // Disallow devices and directories.
  Stat st;
  if (f->stat && (st = f->stat()) && !st->isreg) {
    throw(({ sprintf("Stdio.read_file(): File %O is not a regular file!\n",
		     filename),
	     backtrace()
    }));
  }

  switch(query_num_arg())
  {
  case 1:
    ret=f->read(0x7fffffff);
    break;

  case 2:
    len=0x7fffffff;
  case 3:
    while(start-- && f->gets());
    object(String_buffer) buf=String_buffer();
    while(len-- && (tmp=f->gets()))
    {
      buf->append(tmp);
      buf->append("\n");
    }
    ret=buf->get_buffer();
    destruct(buf);
  }
  f->close();

  return ret;
}

//! @decl string read_bytes(string filename, int start, int len)
//! @decl string read_bytes(string filename, int start)
//! @decl string read_bytes(string filename)
//!
//! Read @[len] number of bytes from the file @[filename] starting at byte
//! @[start], and return it as a string.
//!
//! If @[len] is omitted, the rest of the file will be returned.
//!
//! If @[start] is also omitted, the entire file will be returned.
//!
//! @seealso
//! @[read_file], @[write_file()], @[append_file()]
//!
string read_bytes(string filename, void|int start,void|int len)
{
  string ret;
  File f = File();

  if(!f->open(filename,"r"))
    return 0;
  
  // Disallow devices and directories.
  Stat st;
  if (f->stat && (st = f->stat()) && !st->isreg) {
    throw(({sprintf("Stdio.read_bytes(): File \"%s\" is not a regular file!\n",
		    filename),
	    backtrace()
    }));
  }

  switch(query_num_arg())
  {
  case 1:
  case 2:
    len=0x7fffffff;
  case 3:
    if(start)
      f->seek(start);
  }
  ret=f->read(len);
  f->close();
  return ret;
}

//! Append the string @[str] onto the file @[filename].
//!
//! @returns
//! Returns number of bytes written.
//!
//! @seealso
//! @[read_bytes()]
//!
int write_file(string filename, string str, int|void access)
{
  int ret;
  File f = File();

  if (query_num_arg() < 3) {
    access = 0666;
  }

  if(!f->open(filename, "twc", access))
    error("Couldn't open file "+filename+".\n");
  
  ret=f->write(str);
  f->close();
  return ret;
}

int append_file(string filename, string what, int|void access)
{
  int ret;
  File f = File();

  if (query_num_arg() < 3) {
    access = 0666;
  }

  if(!f->open(filename, "awc", access))
    error("Couldn't open file "+filename+".\n");
  
  ret=f->write(what);
  f->close();
  return ret;
}

//! Give the size of a file. Size -1 indicates that the file either
//! does not exist, or that it is not readable by you. Size -2
//! indicates that it is a directory.
//!
//! @seealso
//! @[file_stat()], @[write_file()], @[read_bytes()]
//!
int file_size(string filename)
{
  Stat stat;
  stat = file_stat(filename);
  if(!stat) return -1;
  return stat[1]; 
}

//! Append @[relative] paths to an @[absolute] path and remove any
//! @tt{"//"@}, @tt{"../"@} or @tt{"/."@} to produce a straightforward
//! absolute path as a result.
//!
//! @tt{"../"@} is ignorded in the relative paths if it makes the
//! created path begin with something else than the absolute path
//! (or so far created path).
//!
//! @note
//! Warning: This does not work on NT.
//! (Consider paths like: k:/fnord)
//!
//! @seealso
//! @[combine_path()]
//!
string append_path(string absolute, string ... relative)
{
  return combine_path(absolute,
		      @map(relative, lambda(string s) {
				       return combine_path("/", s)[1..];
				     }));
}

//! This function prints a message to stderr along with a description
//! of what went wrong if available. It uses the system errno to find
//! out what went wrong, so it is only applicable to IO errors.
//!
//! @seealso
//! @[werror()]
//!
void perror(string s)
{
#if efun(strerror)
  stderr->write(s+": "+strerror(predef::errno())+"\n");
#else
  stderr->write(s+": errno: "+predef::errno()+"\n");
#endif
}

/*
 * Predicates.
 */

//! Check if a @[path] is a file.
//!
//! @returns
//! Returns true if the given path is a file, otherwise false.
//!
//! @seealso
//! @[exist()], @[is_dir()], @[is_link()], @[file_stat()]
//!
int is_file(string path)
{
  if (Stat s = file_stat (path)) return s->isreg;
  return 0;
}

//! Check if a @[path] is a directory.
//!
//! @returns
//! Returns true if the given path is a directory, otherwise false.
//!
//! @seealso
//! @[exist()], @[is_file()], @[is_link()], @[file_stat()]
//!
int is_dir(string path)
{
  if (Stat s = file_stat (path)) return s->isdir;
  return 0;
}

//! Check if a @[path] is a symbolic link.
//!
//! @returns
//! Returns true if the given path is a symbolic link, otherwise false.
//!
//! @seealso
//! @[exist()], @[is_dir()], @[is_file()], @[file_stat()]
//!
int is_link(string path)
{
  if (Stat s = file_stat (path, 1)) return s->islnk;
  return 0;
}

//! Check if a @[path] exists.
//!
//! @returns
//! Returns true if the given path exists (is a directory or file),
//! otherwise false.
//!
//! @seealso
//! @[is_dir()], @[is_file()], @[is_link()], @[file_stat()]
//!
int exist(string path)
{
   return !!file_stat(path);
}

mixed `[](string index)
{
  mixed x=`->(this_object(),index);
  if(x) return x;
  switch(index)
  {
  case "readline": return (master()->resolv("Stdio")["Readline"])->readline;
  default: return ([])[0];
  }
}

#if constant(system.cp)
constant cp=system.cp;
#else
#define BLOCK 65536
int cp(string from, string to)
{
  string data;
  File f=File(), t;
  if(!f->open(from,"r")) return 0;
  function(int,int|void:string) r=f->read;
  t=File();
  if(!t->open(to,"wct")) {
    f->close();
    return 0;
  }
  function(string:int) w=t->write;
  do
  {
    data=r(BLOCK);
    if(!data) return 0;
    if(w(data)!=strlen(data)) return 0;
  }while(strlen(data) == BLOCK);

  f->close();
  t->close();
  return 1;
}
#endif

static void call_cp_cb(int len,
		       function(int, mixed ...:void) cb, mixed ... args)
{
  // FIXME: Check that the lengths are the same?
  cb(0, @args);
}

void async_cp(string from, string to,
	      function(int, mixed...:void) cb, mixed ... args)
{
  object from_file = File();
  object to_file = File();

  if ((!(from_file->open(from, "r"))) ||
      (!(to_file->open(to, "wct")))) {
    call_out(cb, 0, 0, @args);
    return;
  }
  sendfile(0, from_file, 0, -1, 0, to_file, call_cp_cb, cb, @args);
}

//! Creates zero or more directories to ensure that the given @[pathname] is
//! a directory.
//!
//! If a @[mode] is given, it's used for the new directories after being &'ed
//! with the current umask (on OS'es that support this).
//!
//! @returns
//! Returns zero if it fails and nonzero if it is successful.
//!
//! @seealso
//! @[mkdir()]
//!
int mkdirhier (string pathname, void|int mode)
{
  if (zero_type (mode)) mode = 0777; // &'ed with umask anyway.
  if (!sizeof(pathname)) return 0;
  string path;
  if (pathname[0] == '/') pathname = pathname[1..], path = "/";
  else path = "";
  foreach (pathname / "/", string name) {
    path += name;
    mkdir(path, mode);
    path += "/";
  }
  return is_dir (path);
}

//! Remove a file or directory a directory tree.
//!
//! @returns
//! Returns 0 on failure, nonzero otherwise.
//!
//! @seealso
//! @[rm]
//!
int recursive_rm (string path)
{
  int res = 1;
  Stat a = file_stat(path, 1);
  if(!a)
    return 0;
  if(a[1] == -2)
    if (array(string) sub = get_dir (path))
      foreach( sub, string name )
        if (!recursive_rm (path + "/" + name)) 
          res = 0;
  return res && rm (path);
}

/*
 * Asynchronous sending of files.
 */

#define READER_RESTART 4
#define READER_HALT 32

// FIXME: Support for timeouts?
static class nb_sendfile
{
  static File from;
  static int len;
  static array(string) trailers;
  static File to;
  static function(int, mixed ...:void) callback;
  static array(mixed) args;

  // NOTE: Always modified from backend callbacks, so no need
  // for locking.
  static array(string) to_write = ({});
  static int sent;

  static int reader_awake;
  static int writer_awake;

  static int blocking_to;
  static int blocking_from;

  /* Reader */

  static string _sprintf( int f )
  {
    switch( f )
    {
     case 't':
       return "Stdio.Sendfile";
     case 'O':
       return sprintf( "%t()", this_object() );
    }
  }

  static void reader_done()
  {
#ifdef SENDFILE_DEBUG
    werror("Stdio.sendfile(): Reader done.\n");
#endif /* SENDFILE_DEBUG */

    from->set_blocking();
    from = 0;
    if (trailers) {
      to_write += trailers;
    }
    if (blocking_to) {
      while(sizeof(to_write)) {
	if (!do_write()) {
	  // Connection closed or Disk full.
	  writer_done();
	  return;
	}
      }
      if (!from) {
	writer_done();
	return;
      }
    } else {
      if (sizeof(to_write)) {
	start_writer();
      } else {
	writer_done();
	return;
      }
    }
  }

  static void close_cb(mixed ignored)
  {
#ifdef SENDFILE_DEBUG
    werror("Stdio.sendfile(): Input EOF.\n");
#endif /* SENDFILE_DEBUG */

    reader_done();
  }

  static void do_read()
  {
#ifdef SENDFILE_DEBUG
    werror("Stdio.sendfile(): Blocking read.\n");
#endif /* SENDFILE_DEBUG */
    if( sizeof( to_write ) > 2)
      return;
    string more_data = from->read(65536, 1);
    if (more_data == "") {
      // EOF.
#ifdef SENDFILE_DEBUG
      werror("Stdio.sendfile(): Blocking read got EOF.\n");
#endif /* SENDFILE_DEBUG */

      from = 0;
      if (trailers) {
	to_write += (trailers - ({ "" }));
	trailers = 0;
      }
    } else {
      to_write += ({ more_data });
    }
  }

  static void read_cb(mixed ignored, string data)
  {
#ifdef SENDFILE_DEBUG
    werror("Stdio.sendfile(): Read callback.\n");
#endif /* SENDFILE_DEBUG */
    if (len > 0) {
      if (sizeof(data) < len) {
	len -= sizeof(data);
	to_write += ({ data });
      } else {
	to_write += ({ data[..len-1] });
	from->set_blocking();
	reader_done();
	return;
      }
    } else {
      to_write += ({ data });
    }
    if (blocking_to) {
      while(sizeof(to_write)) {
	if (!do_write()) {
	  // Connection closed or Disk full.
	  writer_done();
	  return;
	}
      }
      if (!from) {
	writer_done();
	return;
      }
    } else {
      if (sizeof(to_write) > READER_HALT) {
	// Go to sleep.
	from->set_blocking();
	reader_awake = 0;
      }
      start_writer();
    }
  }

  static void start_reader()
  {
#ifdef SENDFILE_DEBUG
    werror("Stdio.sendfile(): Starting the reader.\n");
#endif /* SENDFILE_DEBUG */
    if (!reader_awake) {
      reader_awake = 1;
      from->set_nonblocking(read_cb, 0, close_cb);
    }
  }

  /* Writer */

  static void writer_done()
  {
#ifdef SENDFILE_DEBUG
    werror("Stdio.sendfile(): Writer done.\n");
#endif /* SENDFILE_DEBUG */

    // Disable any reader.
    if (from && from->set_nonblocking) {
      from->set_nonblocking(0,0,0);
    }

    // Disable any writer.
    if (to && to->set_nonblocking) {
      to->set_nonblocking(0,0,0);
    }

    // Make sure we get rid of any references...
    to_write = 0;
    trailers = 0;
    from = 0;
    to = 0;
    array(mixed) a = args;
    function(int, mixed ...:void) cb = callback;
    args = 0;
    callback = 0;
    if (cb) {
      cb(sent, @a);
    }
  }

  static int do_write()
  {
#ifdef SENDFILE_DEBUG
    werror("Stdio.sendfile(): Blocking writer.\n");
#endif /* SENDFILE_DEBUG */

    int bytes = to->write(to_write);

    if (bytes >= 0) {
      sent += bytes;

      int n;
      for (n = 0; n < sizeof(to_write); n++) {
	if (bytes < sizeof(to_write[n])) {
	  to_write[n] = to_write[n][bytes..];
	  to_write = to_write[n..];

	  return 1;
	} else {
	  bytes -= sizeof(to_write[n]);
	  if (!bytes) {
	    to_write = to_write[n+1..];
	    return 1;
	  }
	}
      }
      // Not reached, but...
      return 1;
    } else {
#ifdef SENDFILE_DEBUG
      werror("Stdio.sendfile(): Blocking writer got EOF.\n");
#endif /* SENDFILE_DEBUG */
      // Premature end of file!
      return 0;
    }
  }

  static void write_cb(mixed ignored)
  {
#ifdef SENDFILE_DEBUG
    werror("Stdio.sendfile(): Write callback.\n");
#endif /* SENDFILE_DEBUG */
    if (do_write()) {
      if (from) {
	if (sizeof(to_write) < READER_RESTART) {
	  if (blocking_from) {
	    do_read();
	    if (!sizeof(to_write)) {
	      // Done.
	      writer_done();
	    }
	  } else {
	    if (!sizeof(to_write)) {
	      // Go to sleep.
	      to->set_nonblocking(0,0,0);
	      writer_awake = 0;
	    }
	    start_reader();
	  }
	}
      } else if (!sizeof(to_write)) {
	// Done.
	writer_done();
      }
    } else {
      // Premature end of file!
      writer_done();
    }
  }

  static void start_writer()
  {
#ifdef SENDFILE_DEBUG
    werror("Stdio.sendfile(): Starting the writer.\n");
#endif /* SENDFILE_DEBUG */

    if (!writer_awake) {
      writer_awake = 1;
      to->set_nonblocking(0, write_cb, 0);
    }
  }

  /* Blocking */
  static void do_blocking()
  {
#ifdef SENDFILE_DEBUG
    werror("Stdio.sendfile(): Blocking I/O.\n");
#endif /* SENDFILE_DEBUG */

    if (from && (sizeof(to_write) < READER_RESTART)) {
      do_read();
    }
    if (sizeof(to_write) && do_write()) {
      call_out(do_blocking, 0);
    } else {
#ifdef SENDFILE_DEBUG
      werror("Stdio.sendfile(): Blocking I/O done.\n");
#endif /* SENDFILE_DEBUG */
      // Done.
      from = 0;
      to = 0;
      writer_done();
    }
  }

#ifdef SENDFILE_DEBUG
  void destroy()
  {
    werror("Stdio.sendfile(): Destructed.\n");
  }
#endif /* SENDFILE_DEBUG */

  /* Starter */

  void create(array(string) hd,
	      File f, int off, int l,
	      array(string) tr,
	      File t,
	      function(int, mixed ...:void)|void cb,
	      mixed ... a)
  {
    if (!l) {
      // No need for from.
      f = 0;

      // No need to differentiate between headers and trailers.
      if (tr) {
	if (hd) {
	  hd += tr;
	} else {
	  hd = tr;
	}
	tr = 0;
      }
    }

    if (!f && (!hd || !sizeof(hd - ({ "" })))) {
      // NOOP!
#ifdef SENDFILE_DEBUG
      werror("Stdio.sendfile(): NOOP!\n");
#endif /* SENDFILE_DEBUG */
      call_out(cb, 0, 0, @a);
      return;
    }

    to_write = (hd || ({})) - ({ "" });
    from = f;
    len = l;
    trailers = (tr || ({})) - ({ "" });
    to = t;
    callback = cb;
    args = a;

    blocking_to = to->is_file ||
      ((!to->set_nonblocking) ||
       (to->mode && !(to->mode() & PROP_NONBLOCK)));

    if (blocking_to && to->set_blocking) {
#ifdef SENDFILE_DEBUG
      werror("Stdio.sendfile(): Blocking to.\n");
#endif /* SENDFILE_DEBUG */
      to->set_blocking();
    }

    if (from) {
      blocking_from = from->is_file ||
	((!from->set_nonblocking) ||
	 (from->mode && !(from->mode() & PROP_NONBLOCK)));
	
      if (off >= 0) {
	from->seek(off);
      }
      if (blocking_from) {
#ifdef SENDFILE_DEBUG
	werror("Stdio.sendfile(): Blocking from.\n");
#endif /* SENDFILE_DEBUG */
	if (from->set_blocking) {
	  from->set_blocking();
	}
      } else {
#ifdef SENDFILE_DEBUG
	werror("Stdio.sendfile(): Starting reader.\n");
#endif /* SENDFILE_DEBUG */
	start_reader();
      }
    }

    if (blocking_to) {
      if (!from || blocking_from) {
	// Can't use the reader to push data.

	// Could have a direct call to do_blocking here,
	// but then the callback would be called from the wrong context.
#ifdef SENDFILE_DEBUG
	werror("Stdio.sendfile(): Using fully blocking I/O.\n");
#endif /* SENDFILE_DEBUG */
	call_out(do_blocking, 0);
      }
    } else {
      if (blocking_from) {
#ifdef SENDFILE_DEBUG
	werror("Stdio.sendfile(): Reading some data.\n");
#endif /* SENDFILE_DEBUG */
	do_read();
	if (!sizeof(to_write)) {
#ifdef SENDFILE_DEBUG
	  werror("Stdio.sendfile(): NOOP!\n");
#endif /* SENDFILE_DEBUG */
	  call_out(cb, 0, 0, @args);
	}
      }
      if (sizeof(to_write)) {
#ifdef SENDFILE_DEBUG
	werror("Stdio.sendfile(): Starting the writer.\n");
#endif /* SENDFILE_DEBUG */
	start_writer();
      }
    }
  }
}

//! @decl object sendfile(array(string) headers
//!                       File from, int offset, int len,
//!                       array(string) trailers,
//!                       File to)
//! @decl object sendfile(array(string) headers
//!                       File from, int offset, int len,
//!                       array(string) trailers,
//!                       File to,
//!                       function(int, mixed ...:void) callback,
//!                       mixed ... args)
//!
//! Sends @[headers] followed by @[len] bytes starting at @[offset]
//! from the file @[from] followed by @[trailers] to the file @[to].
//! When completed @[callback] will be called with the total number of
//! bytes sent as the first argument, followed by @[args].
//!
//! Any of @[headers], @[from] and @[trailers] may be left out
//! by setting them to @tt{0@}.
//!
//! Setting @[offset] to @tt{-1@} means send from the current position in
//! @[from].
//!
//! Setting @[len] to @tt{-1@} means send until @[from]'s end of file is
//! reached.
//!
//! @note
//! The sending is performed asynchronously, and may complete
//! before the function returns.
//!
//! For @[callback] to be called, the backend must be active (ie
//! @[main()] must have returned @tt{-1@}).
//!
//! In some cases, the backend must also be active for any sending to
//! be performed at all.
//!
//! @bugs
//! FIXME: Support for timeouts?
//!
//! @seealso
//! @[Stdio.File->set_nonblocking()]
//!
object sendfile(array(string) headers,
		object from, int offset, int len,
		array(string) trailers,
		object to,
		function(int, mixed ...:void)|void cb,
		mixed ... args)
{
#if constant(files.sendfile)
  // Try using files.sendfile().
  
  mixed err = catch {
    return files.sendfile(headers, from, offset, len,
			  trailers, to, cb, @args);
  };

#ifdef SENDFILE_DEBUG
  werror(sprintf("files.sendfile() failed:\n%s\n",
		 describe_backtrace(err)));
#endif /* SENDFILE_DEBUG */

#endif /* files.sendfile */

  // Use nb_sendfile instead.
  return nb_sendfile(headers, from, offset, len, trailers, to, cb, @args);
}


class UDP
{
  inherit files.UDP;

  private static array extra=0;
  private static function callback=0;

  static string _sprintf( int f )
  {
    switch( f )
    {
    case 't':
      return "Stdio.UDP";
    case 'O':
      return sprintf("%t()", this_object() );
    }
  }

  //! @decl UDP set_nonblocking()
  //! @decl UDP set_nonblocking(function(mapping(string:int|string),
  //!                                    mixed ...:void) read_cb,
  //!                           mixed ... extra_args)
  //!
  //! Set this object to nonblocking mode.
  //!
  //! If @[read_cb] and @[extra_args] are specified, they will be passed on
  //! to @[set_read_callback()].
  //!
  //! @returns
  //! The called object.
  //!
  this_program set_nonblocking(mixed ...stuff)
  {
    if (stuff!=({})) 
      set_read_callback(@stuff);
    return _set_nonblocking();
  }

  //! @decl UDP set_read_callback(function(mapping(string:int|string),
  //!                                      mixed...) read_cb,
  //!                             mixed ... extra_args);
  //!
  //! The @[read_cb] function will receive a mapping similar to the mapping
  //! returned by @[read()]:
  //! @mapping
  //!   @member "data" string
  //!     Received data.
  //!   @member "ip" string
  //!     Data was sent from this IP.
  //!   @member "port" int
  //!     Data was sent from this port.
  //! @endmapping
  //!
  //! @returns
  //! The called object.
  //!
  //! @seealso
  //! @[read()]
  //!
  this_program set_read_callback(function f,mixed ...ext)
  {
    extra=ext;
    callback=f;
    _set_read_callback(_read_callback);
    return this_object();
  }
   
  private static void _read_callback()
  {
    mapping i;
    if (i=read())
      callback(i,@extra);
  }
}

//! @decl void werror(string s)
//!
//! Write a message to stderr. Stderr is normally the console, even if
//! the process output has been redirected to a file or pipe.
//!
constant werror=predef::werror;
