
BUILT_SOURCES =		\
	addr.h		\
	sieve.h

include_HEADERS = sieve_interface.h sieve_err.h

AM_YFLAGS = -d

lib_LTLIBRARIES = libyasieve.la

libyasieve_la_LIBADD = \
	$(top_srcdir)/libutil/libutil.la

libyasieve_la_CFLAGS =		\
	-I$(top_srcdir)/libutil	\
	-I$(top_srcdir)

libyasieve_la_SOURCES =	\
	addr-lex.l	\
	addr.y		\
	bc_dump.c	\
	bc_emit.c	\
	bc_eval.c	\
	bc_generate.c	\
	comparator.c	\
	interp.c	\
	message.c	\
	script.c	\
	sieve-lex.l	\
	sieve.y		\
	sieve_err.c	\
	tree.c

addr.c: addr.y
	$(YACC) $(AM_YFLAGS) -p $(basename $@ .c) -o $@ $^

sieve.c: sieve.y
	$(YACC) $(AM_YFLAGS) -o $@ $^

bin_PROGRAMS = sievec sieved

sievec_SOURCE =		\
	sievec.c

sievec_LDADD =		\
	libyasieve.la

sievec_CFLAGS =		\
	-I$(top_srcdir)/libutil	\
	-I$(top_srcdir)

sieved_SOURCE =		\
	sieved.c

sieved_LDADD =		\
	libyasieve.la

sieved_CFLAGS =		\
	-I$(top_srcdir)/libutil	\
	-I$(top_srcdir)

CLEANFILES = sieve.c sieve.h addr.c addr.h addr-lex.c sieve-lex.c
