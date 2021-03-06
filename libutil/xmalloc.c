/* xmalloc.c -- Allocation package that calls fatal() when out of memory
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
 * $Id: xmalloc.c,v 1.32 2008/03/24 17:43:10 murch Exp $
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xmalloc.h"

#include "exitcodes.h"

void* xmalloc(unsigned size)
{
    void *ret;

    ret = malloc(size);
    if (ret != NULL) return ret;

    fatal("Virtual memory exhausted", EC_TEMPFAIL);
    return 0; /*NOTREACHED*/
}

void* xzmalloc(unsigned size)
{
    void *ret;

    ret = malloc(size);
    if (ret != NULL) {
	memset(ret, 0, size);
	return ret;
    }

    fatal("Virtual memory exhausted", EC_TEMPFAIL);
    return 0; /*NOTREACHED*/
}

void *xrealloc (void* ptr, unsigned size)
{
    void *ret;

    /* xrealloc (NULL, size) behaves like xmalloc (size), as in ANSI C */
    ret = (!ptr ? malloc (size) : realloc (ptr, size));
    if (ret != NULL) return ret;

    fatal("Virtual memory exhausted", EC_TEMPFAIL);
    return 0; /*NOTREACHED*/
}

char *xstrdup(const char* str)
{
    char *p = xmalloc(strlen(str)+1);
    strcpy(p, str);
    return p;
}

char *xstrndup(const char* str, unsigned len)
{
    char *p = xmalloc(len+1);
    strncpy(p, str, len);
    p[len] = '\0';
    return p;
}
