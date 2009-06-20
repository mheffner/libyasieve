#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(libyasieve, libyasieve-0.0.1, mikeh@fesnel.com)
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([src/interp.c])
AC_CONFIG_HEADER([config.h])
AC_CANONICAL_SYSTEM
AM_MAINTAINER_MODE
AM_INIT_AUTOMAKE([1.9])

# Checks for programs.
AC_PROG_CC
AC_PROG_YACC
AC_PROG_INSTALL
AC_PROG_LEX
AC_PROG_LIBTOOL

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lcom_err':
AC_CHECK_LIB([com_err], [main])

# Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h libintl.h limits.h malloc.h netdb.h netinet/in.h stdarg.h stddef.h stdlib.h string.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_STDBOOL
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
#AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([dup2 endgrent fdatasync ftruncate gethostbyaddr gethostbyname gethostname getpass gettimeofday inet_ntoa isascii memchr memmove memset mkdir munmap regcomp select socket strcasecmp strchr strcspn strdup strerror strncasecmp strrchr strstr strtol strtoul])

# nonblocking check
AC_MSG_CHECKING(nonblocking method)
AC_CACHE_VAL(cyrus_cv_sys_nonblock,AC_TRY_LINK([#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#ifndef	FNDELAY
#define FNDELAY		O_NDELAY
#endif],[fcntl(0, F_GETFL, 0)&FNDELAY],
cyrus_cv_sys_nonblock=fcntl,cyrus_cv_sys_nonblock=ioctl))
WITH_NONBLOCK=$cyrus_cv_sys_nonblock
AC_SUBST(WITH_NONBLOCK)
AC_MSG_RESULT($WITH_NONBLOCK)

AC_MSG_CHECKING(timezone GMT offset method)
AC_CACHE_VAL(cyrus_cv_struct_sys_gmtoff,AC_TRY_COMPILE([
#include <time.h>],[struct tm tm;
tm.tm_gmtoff = 0;
],cyrus_cv_struct_sys_gmtoff=tm,cyrus_cv_struct_sys_gmtoff=gmtime))
WITH_GMTOFF=$cyrus_cv_struct_sys_gmtoff
AC_SUBST(WITH_GMTOFF)
AC_MSG_RESULT($WITH_GMTOFF)
AC_MSG_CHECKING(for shared mmap)
AC_CACHE_VAL(cyrus_cv_func_mmap_shared,AC_TRY_RUN([
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
main() {
char *base;
int fd = open("conftestmmap", O_RDWR|O_CREAT|O_TRUNC, 0666);
if (fd == -1) exit(1);
if (write(fd, "test", 4) != 4) exit(1);
fsync(fd);
base = mmap((caddr_t)0, 100, PROT_READ, MAP_SHARED
#ifdef MAP_FILE
| MAP_FILE
#endif
#ifdef MAP_VARIABLE
| MAP_VARIABLE
#endif
, fd, 0L);
if (base == (caddr_t)-1) exit(1);
if (strncmp(base, "test", 4) != 0) exit(1);
if (write(fd, "test", 4) != 4) exit(1);
fsync(fd);
if (strncmp(base+4, "test", 4) != 0) exit(1);
exit(0);}
],cyrus_cv_func_mmap_shared=yes,cyrus_cv_func_mmap_shared=no,
cyrus_cv_func_mmap_shared=no))
AC_MSG_RESULT($cyrus_cv_func_mmap_shared)
if test $cyrus_cv_func_mmap_shared = yes; then
	WITH_MAP="shared"
else
AC_MSG_CHECKING(for stupid shared mmap)
AC_CACHE_VAL(cyrus_cv_func_mmap_stupidshared,AC_TRY_RUN([
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
main() {
char *base;
int fd = open("conftestmmap", O_RDWR|O_CREAT|O_TRUNC, 0666);
if (fd == -1) exit(1);
if (write(fd, "test", 4) != 4) exit(1);
fsync(fd);
base = mmap((caddr_t)0, 4, PROT_READ, MAP_SHARED
#ifdef MAP_FILE
| MAP_FILE
#endif
#ifdef MAP_VARIABLE
| MAP_VARIABLE
#endif
, fd, 0L);
if (base == (caddr_t)-1) exit(1);
if (strncmp(base, "test", 4) != 0) exit(1);
lseek(fd, 0L, 0);	
if (write(fd, "xyzz", 4) != 4) exit(1);
fsync(fd);
if (strncmp(base, "xyzz", 4) != 0) exit(1);
exit(0);}
],cyrus_cv_func_mmap_stupidshared=yes,cyrus_cv_func_mmap_stupidshared=no,
cyrus_cv_func_mmap_stupidshared=no))
AC_MSG_RESULT($cyrus_cv_func_mmap_stupidshared)
if test $cyrus_cv_func_mmap_stupidshared = yes; then
	WITH_MAP="stupidshared"
else
	AC_MSG_WARN([*** This system does not have a working mmap()])
	AC_MSG_WARN(*** Expect a considerable performance penalty)
	WITH_MAP=nommap
fi
fi

AC_SUBST(WITH_MAP)

# Determine locking type to use
AC_ARG_WITH(lock,
  [  --with-lock=METHOD      force use of METHOD for locking (flock or fcntl)],
  WITH_LOCK="$withval", [
  AC_CHECK_FUNC(fcntl,WITH_LOCK="fcntl",[
                AC_CHECK_FUNC(flock,WITH_LOCK="flock",[
                                AC_ERROR(unable to detect locking method)
                              ])
                ])
  ])

AC_SUBST(WITH_LOCK)

# Defines
AC_DEFINE([FNAME_PTSDB], ["/ptclient/ptscache.db"], [Desc])
AC_DEFINE([FNAME_DBDIR], ["/db"], [Desc])
AC_DEFINE([FNAME_DOMAINDIR], ["/domain/"], [Desc])

AC_DEFINE([CONFIG_DB_VERBOSE], , [Desc])

AC_DEFINE([CYRUS_USER], ["cyrus"], [Desc])

AC_CONFIG_FILES([
	Makefile
	libutil/Makefile
	src/Makefile
	])
AC_OUTPUT
