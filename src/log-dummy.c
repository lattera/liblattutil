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

#include "liblattutil.h"

EXPORTED_SYM
void
lattutil_log_dummy_init(lattutil_log_t *res)
{

	res->ll_log_close = lattutil_log_dummy_close;
	res->ll_log_debug = lattutil_log_dummy_debug;
	res->ll_log_err = lattutil_log_dummy_err;
	res->ll_log_info = lattutil_log_dummy_info;
	res->ll_log_warn = lattutil_log_dummy_warn;
}

ssize_t
lattutil_log_dummy_debug(lattutil_log_t *logp, int verbose,
    const char *fmt, ...)
{

	return (0);
}

ssize_t
lattutil_log_dummy_err(lattutil_log_t *logp, int verbose,
    const char *fmt, ...)
{

	return (0);
}

ssize_t
lattutil_log_dummy_info(lattutil_log_t *logp, int verbose,
    const char *fmt, ...)
{

	return (0);
}

ssize_t
lattutil_log_dummy_warn(lattutil_log_t *logp, int verbose,
    const char *fmt, ...)
{

	return (0);
}

void
lattutil_log_dummy_close(lattutil_log_t *logp)
{

	return;
}
