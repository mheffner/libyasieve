/* libconfig.c -- imapd.conf handling
 *
 * Copyright (c) 1994-2008 Carnegie Mellon University.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The name "Carnegie Mellon University" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For permission or any legal
 *    details, please contact
 *      Carnegie Mellon University
 *      Center for Technology Transfer and Enterprise Creation
 *      4615 Forbes Avenue
 *      Suite 302
 *      Pittsburgh, PA  15213
 *      (412) 268-7393, fax: (412) 268-7395
 *      innovation@andrew.cmu.edu
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Computing Services
 *     at Carnegie Mellon University (http://www.cmu.edu/computing/)."
 *
 * CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: libconfig.c,v 1.21 2008/09/23 17:34:38 murch Exp $
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>

#include "assert.h"
#include "hash.h"
#include "libconfig.h"
#include "imapopts.h"
#include "xmalloc.h"
#include "xstrlcat.h"
#include "xstrlcpy.h"

#define CONFIGHASHSIZE 30 /* relatively small,
			   * because it is for overflow only */
#define INCLUDEHASHSIZE 5 /* relatively small,
			    * but how many includes are reasonable? */
static struct hash_table confighash, includehash;

/* cached configuration variables accessible to the external world */
const char *config_filename= NULL;       /* filename of configuration file */
const char *config_dir = NULL;		 /* ie /var/imap */
const char *config_defpartition = NULL;  /* /var/spool/imap */
const char *config_servername= NULL;	 /* gethostname() */
enum enum_value config_serverinfo;	 /* on */
const char *config_mupdate_server = NULL;/* NULL */
const char *config_defdomain = NULL;     /* NULL */
const char *config_ident = NULL;         /* the service name */
int config_hashimapspool;	  /* f */
enum enum_value config_virtdomains;	          /* f */
enum enum_value config_mupdate_config;	/* IMAP_ENUM_MUPDATE_CONFIG_STANDARD */
int config_maxword;
int config_maxquoted;

/* declared in each binary that uses libconfig */
extern const int config_need_data;
extern void fatal(const char *fatal_message, int fatal_code)
   __attribute__ ((noreturn));

/* prototype to allow for sane function ordering */
void config_read_file(const char *filename);

const char *config_getstring(enum imapopt opt)
{
    assert(opt > IMAPOPT_ZERO && opt < IMAPOPT_LAST);
    assert((imapopts[opt].t == OPT_STRING) ||
	   (imapopts[opt].t == OPT_STRINGLIST));
    
    return imapopts[opt].val.s;
}

int config_getint(enum imapopt opt)
{
    assert(opt > IMAPOPT_ZERO && opt < IMAPOPT_LAST);
    assert(imapopts[opt].t == OPT_INT);
#if (SIZEOF_LONG != 4)
    if ((imapopts[opt].val.i > 0x7fffffff)||
	(imapopts[opt].val.i < -0x7fffffff)) {
	syslog(LOG_ERR, "config_getint: %s: %lld too large for type",
	       imapopts[opt].optname, imapopts[opt].val.i);
    }
#endif    
    return imapopts[opt].val.i;
}

int config_getswitch(enum imapopt opt)
{
    assert(opt > IMAPOPT_ZERO && opt < IMAPOPT_LAST);
    assert(imapopts[opt].t == OPT_SWITCH);
#if (SIZEOF_LONG != 4)
    if ((imapopts[opt].val.b > 0x7fffffff)||
	(imapopts[opt].val.b < -0x7fffffff)) {
	syslog(LOG_ERR, "config_getswitch: %s: %lld too large for type", 
	       imapopts[opt].optname, imapopts[opt].val.b);
    }
#endif    
    return imapopts[opt].val.b;
}

enum enum_value config_getenum(enum imapopt opt)
{
    assert(opt > IMAPOPT_ZERO && opt < IMAPOPT_LAST);
    assert(imapopts[opt].t == OPT_ENUM);
    
    return imapopts[opt].val.e;
}

unsigned long config_getbitfield(enum imapopt opt)
{
    assert(opt > IMAPOPT_ZERO && opt < IMAPOPT_LAST);
    assert(imapopts[opt].t == OPT_BITFIELD);
    
    return imapopts[opt].val.x;
}

const char *config_getoverflowstring(const char *key, const char *def)
{
    char buf[256];
    char *ret = NULL;

    /* First lookup <ident>_key, to see if we have a service-specific
     * override */

    if(config_ident) {
	if(snprintf(buf,sizeof(buf),"%s_%s",config_ident,key) == -1)
	    fatal("key too long in config_getoverflowstring", EC_TEMPFAIL);
    
	ret = hash_lookup(buf, &confighash);
    }
    
    /* No service-specific override, check the actual key */
    if(!ret)
	ret = hash_lookup(key, &confighash);

    /* Return what we got or the default */
    return ret ? ret : def;
}

void config_foreachoverflowstring(void (*func)(const char *, const char *, void *),
				  void *rock)
{
    hash_enumerate(&confighash, (void (*)(char *, void *, void *)) func, rock);
}

const char *config_partitiondir(const char *partition)
{
    char buf[80];

    if(strlcpy(buf, "partition-", sizeof(buf)) >= sizeof(buf))
	return 0;
    if(strlcat(buf, partition, sizeof(buf)) >= sizeof(buf))
	return 0;

    return config_getoverflowstring(buf, NULL);
}

const char *config_metapartitiondir(const char *partition)
{
    char buf[80];

    if(strlcpy(buf, "metapartition-", sizeof(buf)) >= sizeof(buf))
	return 0;
    if(strlcat(buf, partition, sizeof(buf)) >= sizeof(buf))
	return 0;

    return config_getoverflowstring(buf, NULL);
}

void config_read(const char *alt_config)
{
    enum imapopt opt = IMAPOPT_ZERO;
    char buf[4096];
    char *p;

    /* xxx this is leaked, this may be able to be better in 2.2 (cyrus_done) */
    if(alt_config) config_filename = xstrdup(alt_config);
    else config_filename = xstrdup(CONFIG_FILENAME);

    if(!construct_hash_table(&confighash, CONFIGHASHSIZE, 1)) {
	fatal("could not construct configuration hash table", EC_CONFIG);
    }

    if(!construct_hash_table(&includehash, INCLUDEHASHSIZE, 1)) {
	fatal("could not construct include file  hash table", EC_CONFIG);
    }

    config_read_file(config_filename);

    free_hash_table(&includehash, NULL);

    /* Check configdirectory config option */
    if (!config_dir) {
	fatal("configdirectory option not specified in configuration file",
	      EC_CONFIG);
    }

    /* Scan options to see if we need to replace {configdirectory} */
    /* xxx need to scan overflow options as well! */
    for(opt = IMAPOPT_ZERO; opt < IMAPOPT_LAST; opt++) {
	if(!imapopts[opt].val.s ||
	   imapopts[opt].t != OPT_STRING ||
	   opt == IMAPOPT_CONFIGDIRECTORY) {
	    /* Skip options that have a NULL value, aren't strings, or
	     * are the configdirectory option */
	    continue;
	}
	
	/* We use some magic numbers here,
	 * 17 is the length of "{configdirectory}",
	 * 16 is one less than that length, so that the replacement string
	 *    that is malloced has room for the '\0' */
	if(!strncasecmp(imapopts[opt].val.s,"{configdirectory}",17)) {
	    const char *str = imapopts[opt].val.s;
	    char *newstring =
		xmalloc(strlen(config_dir) + strlen(str) - 16);
	    char *freeme = NULL;
	    
	    /* we need to replace this string, will we need to free
	     * the current value?  -- only if we've actually seen it in
	     * the config file. */
	    if(imapopts[opt].seen)
		freeme = (char *)str;

	    /* Build replacement string from configdirectory option */
	    strcpy(newstring, config_dir);
	    strcat(newstring, str + 17);

	    imapopts[opt].val.s = newstring;

	    if(freeme) free(freeme);
	}
    }

    /* Look up default partition */
    config_defpartition = config_getstring(IMAPOPT_DEFAULTPARTITION);
    for (p = (char *)config_defpartition; *p; p++) {
	if (!isalnum((unsigned char) *p))
	  fatal("defaultpartition option contains non-alphanumeric character",
		EC_CONFIG);
	if (isupper((unsigned char) *p)) *p = tolower((unsigned char) *p);
    }
    if ((config_need_data & CONFIG_NEED_PARTITION_DATA) &&
	(!config_defpartition || !config_partitiondir(config_defpartition))) {
	snprintf(buf, sizeof(buf),
		"partition-%s option not specified in configuration file",
		config_defpartition);
	fatal(buf, EC_CONFIG);
    }

    /* look up mailbox hashing */
    config_hashimapspool = config_getswitch(IMAPOPT_HASHIMAPSPOOL);

    /* are we supporting virtual domains?  */
    config_virtdomains = config_getenum(IMAPOPT_VIRTDOMAINS);
    config_defdomain = config_getstring(IMAPOPT_DEFAULTDOMAIN);

    /* look up the hostname and info we should present to the user */
    config_servername = config_getstring(IMAPOPT_SERVERNAME);
    if (!config_servername) {
	config_servername = xmalloc(sizeof(char) * 256);
	gethostname((char *) config_servername, 256);
    }
    config_serverinfo = config_getenum(IMAPOPT_SERVERINFO);

    config_mupdate_server = config_getstring(IMAPOPT_MUPDATE_SERVER);

    if (config_mupdate_server) {
	config_mupdate_config = config_getenum(IMAPOPT_MUPDATE_CONFIG);
    }

    /* set some limits */
    config_maxquoted = config_getint(IMAPOPT_MAXQUOTED);
    config_maxword = config_getint(IMAPOPT_MAXWORD);
}

#define GROWSIZE 4096

void config_read_file(const char *filename)
{
    FILE *infile;
    enum imapopt opt = IMAPOPT_ZERO;
    int lineno = 0;
    char *buf, errbuf[1024];
    unsigned bufsize, len;
    char *p, *q, *key, *fullkey, *srvkey, *val, *newval;
    int service_specific;
    int idlen = (config_ident ? strlen(config_ident) : 0);

    bufsize = GROWSIZE;
    buf = xmalloc(bufsize);

    /* read in config file */
    infile = fopen(filename, "r");
    if (!infile) {
	strlcpy(buf, CYRUS_PATH, bufsize);
	strlcat(buf, filename, bufsize);
	infile = fopen(buf, "r");
    }
    if (!infile) {
	snprintf(buf, bufsize, "can't open configuration file %s: %s",
		 filename, error_message(errno));
	fatal(buf, EC_CONFIG);
    }

    /* check to see if we've already read this file */
    if (hash_lookup(filename, &includehash)) {
	snprintf(buf, bufsize, "configuration file %s included twice",
		 filename);
	fatal(buf, EC_CONFIG);
    }
    else {
	hash_insert(filename, (void*) 0xDEADBEEF, &includehash);
    }
    
    len = 0;
    while (fgets(buf+len, bufsize-len, infile)) {
	if (buf[len]) {
	    len = strlen(buf);
	    if (buf[len-1] == '\n') {
		/* end of line */
		buf[--len] = '\0';

		if (len && buf[len-1] == '\\') {
		    /* line continuation */
		    len--;
		    lineno++;
		    continue;
		}
	    }
	    else if (!feof(infile) && len == bufsize-1) {
		/* line is longer than the buffer */
		bufsize += GROWSIZE;
		buf = xrealloc(buf, bufsize);
		continue;
	    }
	}
	len = 0;
	lineno++;

	service_specific = 0;

	/* remove leading whitespace */
	for (p = buf; *p && isspace((int) *p); p++);

	/* skip comments */
	if (!*p || *p == '#') continue;

	fullkey = key = p;
	if (*p == '@') p++;  /* allow @ as the first char (for directives) */
	while (*p && (isalnum((int) *p) || *p == '-' || *p == '_')) {
	    if (isupper((unsigned char) *p)) *p = tolower((unsigned char) *p);
	    p++;
	}
	if (*p != ':') {
	    snprintf(errbuf, sizeof(errbuf),
		    "invalid option name on line %d of configuration file %s",
		    lineno, filename);
	    fatal(errbuf, EC_CONFIG);
	}
	*p++ = '\0';
	
	/* remove leading whitespace */
	while (*p && isspace((int) *p)) p++;
	
	/* remove trailing whitespace */
	for (q = p + strlen(p) - 1; q > p && isspace((int) *q); q--) {
	    *q = '\0';
	}
	
	if (!*p) {
	    snprintf(errbuf, sizeof(errbuf),
		    "empty option value on line %d of configuration file",
		    lineno);
	    fatal(errbuf, EC_CONFIG);
	}
	
	srvkey = NULL;

	/* Look for directives */
	if (key[0] == '@') {
	    if (!strcasecmp(key, "@include")) {
		config_read_file(p);
		continue;
	    }
	    else {
		snprintf(errbuf, sizeof(errbuf),
			 "invalid directive on line %d of configuration file %s",
			 lineno, filename);
		fatal(errbuf, EC_CONFIG);
	    }
	}

	/* Find if there is a <service>_ prefix */
	if(config_ident && !strncasecmp(key, config_ident, idlen) 
	   && key[idlen] == '_') {
	    /* skip service_ prefix */
	    srvkey = key + idlen + 1;
	}
	
	/* look for a service_ prefix match in imapopts */
	if(srvkey) {
	    for (opt = IMAPOPT_ZERO; opt < IMAPOPT_LAST; opt++) {
		if (!strcasecmp(imapopts[opt].optname, srvkey)) {
		    key = srvkey;
		    service_specific = 1;
		    break;
		}
	    }
	}
	
	/* Did not find a service_ specific match, try looking for an
	 * exact match */
	if(!service_specific) {
	    for (opt = IMAPOPT_ZERO; opt < IMAPOPT_LAST; opt++) {
		if (!strcasecmp(imapopts[opt].optname, key)) {
		    break;
		}
	    }
	}

	/* If both of those loops failed, it goes verbatim into the
	 * overflow hash table. */
	
	if (opt < IMAPOPT_LAST) {
	    /* Okay, we know about this configure option.
	     * So first check that we have either
	     *  1. not seen it
	     *  2. seen its generic form, but this is a service specific form
	     *
	     *  If we have already seen a service-specific form, and this is
	     *  a generic form, just skip it and don't moan.
	     */
	    if((imapopts[opt].seen == 1 && !service_specific) 
	     ||(imapopts[opt].seen == 2 && service_specific)) {
		sprintf(errbuf,
			"option '%s' was specified twice in config file (second occurance on line %d)",
			fullkey, lineno);
		fatal(errbuf, EC_CONFIG);
	    } else if(imapopts[opt].seen == 2 && !service_specific) {
		continue;
	    }

	    /* If we've seen it already, we're replacing it, so we need
	     * to free the current string if there is one */
	    if(imapopts[opt].seen && imapopts[opt].t == OPT_STRING)
		free((char *)imapopts[opt].val.s);

            if(service_specific)
		imapopts[opt].seen = 2;
	    else
		imapopts[opt].seen = 1;
	    
	    /* this is a known option */
	    switch (imapopts[opt].t) {
	    case OPT_STRING: 
	    {		    
		imapopts[opt].val.s = xstrdup(p);

		if(opt == IMAPOPT_CONFIGDIRECTORY)
		    config_dir = imapopts[opt].val.s;

		break;
	    }
	    case OPT_INT:
	    {
		long val;
		char *ptr;
		
		val = strtol(p, &ptr, 0);
		if (!ptr || *ptr != '\0') {
		    /* error during conversion */
		    sprintf(errbuf, "non-integer value for %s in line %d",
			    imapopts[opt].optname, lineno);
		    fatal(errbuf, EC_CONFIG);
		}

		imapopts[opt].val.i = val;
		break;
	    }
	    case OPT_SWITCH:
	    {
		if (*p == '0' || *p == 'n' ||
		    (*p == 'o' && p[1] == 'f') || *p == 'f') {
		    imapopts[opt].val.b = 0;
		}
		else if (*p == '1' || *p == 'y' ||
			 (*p == 'o' && p[1] == 'n') || *p == 't') {
		    imapopts[opt].val.b = 1;
		}
		else {
		    /* error during conversion */
		    sprintf(errbuf, "non-switch value for %s in line %d",
			    imapopts[opt].optname, lineno);
		    fatal(errbuf, EC_CONFIG);
		}
		break;
	    }
	    case OPT_ENUM:
	    case OPT_STRINGLIST:
	    case OPT_BITFIELD:
	    {
		const struct enum_option_s *e;

		/* zero the value */
		memset(&imapopts[opt].val, 0, sizeof(imapopts[opt].val));

		/* q is already at EOS so we'll process entire the string
		   as one value unless told otherwise */

		if (imapopts[opt].t == OPT_ENUM) {
		    /* normalize on/off values */
		    if (!strcmp(p, "1") || !strcmp(p, "yes") ||
			!strcmp(p, "t") || !strcmp(p, "true")) {
			p = "on";
		    } else if (!strcmp(p, "0") || !strcmp(p, "no") ||
			       !strcmp(p, "f") || !strcmp(p, "false")) {
			p = "off";
		    }
		} else if (imapopts[opt].t == OPT_BITFIELD) {
		    /* split the string into separate values */
		    q = p;
		}

		while (*p) {
		    /* find the end of the first value */
		    for (; *q && !isspace((int) *q); q++);
		    if (*q) *q++ = '\0';

		    /* see if its a legal value */
		    for (e = imapopts[opt].enum_options;
			 e->name && strcmp(e->name, p); e++);

		    if (!e->name) {
			/* error during conversion */
			sprintf(errbuf, "invalid value '%s' for %s in line %d",
				p, imapopts[opt].optname, lineno);
			fatal(errbuf, EC_CONFIG);
		    }
		    else if (imapopts[opt].t == OPT_STRINGLIST)
			imapopts[opt].val.s = e->name;
		    else if (imapopts[opt].t == OPT_ENUM)
			imapopts[opt].val.e = e->val;
		    else
			imapopts[opt].val.x |= e->val;

		    /* find the start of the next value */
		    for (p = q; *p && isspace((int) *p); p++);
		    q = p;
		}

		break;
	    }
	    case OPT_NOTOPT:
	    default:
		abort();
	    }
	} else {
	    /* check to make sure it's valid for overflow */
	    /* that is, partition names and anything that might be
	     * used by SASL */
/*
  xxx this would be nice if it wasn't for other services who might be
      sharing this config file and whose names we cannot predict

	    if(strncasecmp(key,"sasl_",5)
	    && strncasecmp(key,"partition-",10)) {
		sprintf(errbuf,
			"option '%s' is unknown on line %d of config file",
			fullkey, lineno);
		fatal(errbuf, EC_CONFIG);
	    }
*/

	    /* Put it in the overflow hash table */
	    newval = xstrdup(p);
	    val = hash_insert(key, newval, &confighash);
	    if(val != newval) {
		snprintf(errbuf, sizeof(errbuf), 
			"option '%s' was specified twice in config file (second occurance on line %d)",
			fullkey, lineno);
		fatal(errbuf, EC_CONFIG);
	    }
	}
    }
    fclose(infile);
    free(buf);
}
