
noinst_LTLIBRARIES = libutil.la

libutil_la_SOURCES =		\
	assert.c		\
	bsearch.c		\
	byteorder64.c		\
	charset.c		\
	chartable.c		\
	ftruncate.c		\
	getdtablesize.c		\
	gmtoff_@WITH_GMTOFF@.c	\
	glob.c			\
	hash.c			\
	imapurl.c		\
	imparse.c		\
	iptostring.c		\
	libconfig.c		\
	lock_@WITH_LOCK@.c	\
	lsort.c			\
	map_@WITH_MAP@.c	\
	md5.c			\
	memmove.c		\
	mkchartable.c		\
	mkgmtime.c		\
	mpool.c			\
	nonblock_@WITH_NONBLOCK@.c	\
	parseaddr.c		\
	retry.c			\
	rfc822date.c		\
	strcasecmp.c		\
	strerror.c		\
	strhash.c		\
	stristr.c		\
	util.c			\
	wildmat.c		\
	xmalloc.c		\
	xstrlcat.c		\
	xstrlcpy.c

DIST_CLEANFILES = chartable.c

chartable.c: mkchartable 
	@echo "### Building chartables..."
	rm -f chartable.c
	./mkchartable \
	 -m $(top_srcdir)/libutil/charset/unifix.txt	\
	 -m $(top_srcdir)/libutil/charset/unidata2.txt \
	 $(top_srcdir)/libutil/charset/*.t \
	 > chartable.c \
	 || (rm -f chartable.c && exit 1)
	@echo "### Done building chartables."
#	./mkchartable -m $(srcdir)/charset/unicode.map $(srcdir)/charset/*.t >x-chartables.h
#	mv x-chartables.h chartables.h

mkchartable: mkchartable.o xstrlcpy.o xstrlcat.o xmalloc.o assert.o
	$(CC) $(LDFLAGS) -o mkchartable mkchartable.o xstrlcpy.o xstrlcat.o xmalloc.o assert.o
