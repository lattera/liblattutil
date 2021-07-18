/*-
 * Copyright (c) 2021 Shawn Webb <shawn.webb@hardenedbsd.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <syslog.h>

#include "liblattutil.h"

EXPORTED_SYM
lattutil_log_t *
lattutil_log_init(char *path, int verbosity)
{
	lattutil_log_t *res;

	res = calloc(1, sizeof(*res));
	if (res == NULL) {
		return (NULL);
	}

	res->ll_verbosity = (verbosity == -1) ? LATTUTIL_LOG_VERBOSITY_DEFAULT :
	    verbosity;
	if (path != NULL) {
		res->ll_path = strdup(path);
		if (res->ll_path == NULL) {
			free(res);
			return (NULL);
		}
	}

	lattutil_log_dummy_init(res);

	return (res);
}

EXPORTED_SYM
void
lattutil_log_free(lattutil_log_t **logp)
{
	lattutil_log_t *logp2;

	if (logp == NULL || *logp == NULL) {
		return;
	}

	logp2 = *logp;

	if (logp2->ll_log_close != NULL) {
		logp2->ll_log_close(logp2);
	}

	free(logp2->ll_path);
	memset(logp2, 0, sizeof(*logp2));
	*logp = NULL;
}

EXPORTED_SYM
bool
lattutil_log_ready(lattutil_log_t *logp)
{

	if (logp == NULL) {
		return (false);
	}

	if (logp->ll_log_debug == NULL ||
	    logp->ll_log_err == NULL ||
	    logp->ll_log_info == NULL ||
	    logp->ll_log_warn == NULL) {
		return (false);
	}

	return (true);
}

EXPORTED_SYM
int
lattutil_log_verbosity(lattutil_log_t *logp)
{

	if (logp == NULL) {
		return (-1);
	}

	return (logp->ll_verbosity);
}

EXPORTED_SYM
int
lattutil_log_set_verbosity(lattutil_log_t *logp, int verbosity)
{
	int old;

	if (logp == NULL) {
		return (-1);
	}

	old = logp->ll_verbosity;

	if (verbosity == -1) {
		verbosity = LATTUTIL_LOG_VERBOSITY_DEFAULT;
	}

	logp->ll_verbosity = verbosity;

	return (old);
}
