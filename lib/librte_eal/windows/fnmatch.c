/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 1989, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Guido van Rossum.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char sccsid[] = "@(#)fnmatch.c	8.2 (Berkeley) 4/16/94";
#endif /* LIBC_SCCS and not lint */

/*
 * Function fnmatch() as specified in POSIX 1003.2-1992, section B.6.
 * Compares a filename or pathname to a pattern.
 */

#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "fnmatch.h"

#define EOS	'\0'

static const char *rangematch(const char *, char, int);

int
fnmatch(const char *pattern, const char *string, int flags)
{
	const char *stringstart;
	char c, test;

	for (stringstart = string;;)
		switch (c = *pattern++) {
		case EOS:
			if ((flags & FNM_LEADING_DIR) && *string == '/')
				return (0);
			return (*string == EOS ? 0 : FNM_NOMATCH);
		case '?':
			if (*string == EOS)
				return (FNM_NOMATCH);
			if (*string == '/' && (flags & FNM_PATHNAME))
				return (FNM_NOMATCH);
			if (*string == '.' && (flags & FNM_PERIOD) &&
			    (string == stringstart ||
			    ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
				return (FNM_NOMATCH);
			++string;
			break;
		case '*':
			c = *pattern;
			/* Collapse multiple stars. */
			while (c == '*')
				c = *++pattern;

			if (*string == '.' && (flags & FNM_PERIOD) &&
			    (string == stringstart ||
			    ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
				return (FNM_NOMATCH);

			/* Optimize for pattern with * at end or before /. */
			if (c == EOS)
				if (flags & FNM_PATHNAME)
					return ((flags & FNM_LEADING_DIR) ||
					    strchr(string, '/') == NULL ?
					    0 : FNM_NOMATCH);
				else
					return (0);
			else if (c == '/' && flags & FNM_PATHNAME) {
				string = strchr(string, '/');
				if (string == NULL)
					return (FNM_NOMATCH);
				break;
			}

			/* General case, use recursion. */
			while ((test = *string) != EOS) {
				if (!fnmatch(pattern, string,
					flags & ~FNM_PERIOD))
					return (0);
				if (test == '/' && flags & FNM_PATHNAME)
					break;
				++string;
			}
			return (FNM_NOMATCH);
		case '[':
			if (*string == EOS)
				return (FNM_NOMATCH);
			if (*string == '/' && flags & FNM_PATHNAME)
				return (FNM_NOMATCH);
			pattern = rangematch(pattern, *string, flags);
			if (pattern == NULL)
				return (FNM_NOMATCH);
			++string;
			break;
		case '\\':
			if (!(flags & FNM_NOESCAPE)) {
				c = *pattern++;
				if (c == EOS) {
					c = '\\';
					--pattern;
				}
			}
			/* FALLTHROUGH */
		default:
			if (c == *string)
				;
			else if ((flags & FNM_CASEFOLD) &&
				 (tolower((unsigned char)c) ==
				  tolower((unsigned char)*string)))
				;
			else if ((flags & FNM_PREFIX_DIRS) && *string == EOS &&
			     ((c == '/' && string != stringstart) ||
			     (string == stringstart+1 && *stringstart == '/')))
				return (0);
			else
				return (FNM_NOMATCH);
			string++;
			break;
		}
	/* NOTREACHED */
}

static const char *
rangematch(const char *pattern, char test, int flags)
{
	int negate, ok;
	char c, c2;

	/*
	 * A bracket expression starting with an unquoted circumflex
	 * character produces unspecified results (IEEE 1003.2-1992,
	 * 3.13.2).  This implementation treats it like '!', for
	 * consistency with the regular expression syntax.
	 * J.T. Conklin (conklin@ngai.kaleida.com)
	 */
	negate = (*pattern == '!' || *pattern == '^');
	if (negate)
		++pattern;

	if (flags & FNM_CASEFOLD)
		test = tolower((unsigned char)test);

	for (ok = 0; (c = *pattern++) != ']';) {
		if (c == '\\' && !(flags & FNM_NOESCAPE))
			c = *pattern++;
		if (c == EOS)
			return (NULL);

		if (flags & FNM_CASEFOLD)
			c = tolower((unsigned char)c);

		c2 = *(pattern + 1);
		if (*pattern == '-' && c2 != EOS && c2 != ']') {
			pattern += 2;
			if (c2 == '\\' && !(flags & FNM_NOESCAPE))
				c2 = *pattern++;
			if (c2 == EOS)
				return (NULL);

			if (flags & FNM_CASEFOLD)
				c2 = tolower((unsigned char)c2);

			if ((unsigned char)c <= (unsigned char)test &&
			    (unsigned char)test <= (unsigned char)c2)
				ok = 1;
		} else if (c == test)
			ok = 1;
	}
	return (ok == negate ? NULL : pattern);
}
