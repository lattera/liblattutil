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
bool
lattutil_log_syslog_init(lattutil_log_t *logp, int logopt, int facility)
{
	const char *name;

	logp->ll_log_close = lattutil_log_syslog_close;
	logp->ll_log_debug = lattutil_log_syslog_debug;
	logp->ll_log_err = lattutil_log_syslog_err;
	logp->ll_log_info = lattutil_log_syslog_info;
	logp->ll_log_warn = lattutil_log_syslog_warn;

	name = logp->ll_path;
	if (name == NULL) {
		name = getprogname();
		if (name == NULL) {
			name = LATTUTIL_LOG_DEFAULT_NAME;
		}
	}

	openlog(name, logopt, facility);

	return (true);
}

ssize_t
lattutil_log_syslog_debug(lattutil_log_t *logp, int verbose,
    const char *fmt, ...)
{
	va_list args;
	size_t len;
	char *msg;

	va_start(args, fmt);
	if (verbose == -1  || verbose >= logp->ll_verbosity) {
		msg = NULL;
		vasprintf(&msg, fmt, args);
		if (msg == NULL) {
			len = -1;
			goto end;
		}
		len = strlen(msg);
		syslog(LOG_DEBUG, "DEBUG: %s", msg);
	}
	va_end(args);

end:
	return (len);
}

ssize_t
lattutil_log_syslog_err(lattutil_log_t *logp, int verbose,
    const char *fmt, ...)
{
	va_list args;
	size_t len;
	char *msg;

	va_start(args, fmt);
	if (verbose == -1  || verbose >= logp->ll_verbosity) {
		msg = NULL;
		vasprintf(&msg, fmt, args);
		if (msg == NULL) {
			len = -1;
			goto end;
		}
		len = strlen(msg);
		syslog(LOG_ERR, "ERROR: %s", msg);
	}
	va_end(args);

end:
	return (len);
}

ssize_t
lattutil_log_syslog_info(lattutil_log_t *logp, int verbose,
    const char *fmt, ...)
{
	va_list args;
	size_t len;
	char *msg;

	va_start(args, fmt);
	if (verbose == -1  || verbose >= logp->ll_verbosity) {
		msg = NULL;
		vasprintf(&msg, fmt, args);
		if (msg == NULL) {
			len = -1;
			goto end;
		}
		len = strlen(msg);
		syslog(LOG_INFO, "INFO: %s", msg);
	}
	va_end(args);

end:
	return (len);
}

ssize_t
lattutil_log_syslog_warn(lattutil_log_t *logp, int verbose,
    const char *fmt, ...)
{
	va_list args;
	size_t len;
	char *msg;

	va_start(args, fmt);
	if (verbose == -1  || verbose >= logp->ll_verbosity) {
		msg = NULL;
		vasprintf(&msg, fmt, args);
		if (msg == NULL) {
			len = -1;
			goto end;
		}
		len = strlen(msg);
		syslog(LOG_WARNING, "WARNING: %s", msg);
	}
	va_end(args);

end:
	return (len);
}

void
lattutil_log_syslog_close(lattutil_log_t *logp)
{

	closelog();
}
