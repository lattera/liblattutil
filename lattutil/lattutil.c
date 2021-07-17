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

int
main(int argc, char *argv[])
{
	lattutil_sqlite_query_t *query;
	const ucl_object_t *cur, *tmp;
	lattutil_sqlite_ctx_t *sqlctx;
	ucl_object_iter_t it, it_obj;
	lattutil_log_t *logp;
	const char *val;
	size_t i;

	logp = lattutil_log_init(NULL, -1);

	lattutil_log_syslog_init(logp, LOG_PID | LOG_NDELAY,
	    LOG_USER);

	sqlctx = lattutil_sqlite_ctx_new("/tmp/db.sqlite3", logp, 0);
	if (sqlctx == NULL) {
		logp->ll_log_err(logp, -1, "Unable to create sqlite3 object");
		return (1);
	}

	query = lattutil_sqlite_prepare(sqlctx,
	    "CREATE TABLE IF NOT EXISTS test_table ("
	    "    colname TEXT NOT NULL, "
	    "    colval TEXT NOT NULL"
	    ")");

	if (query == NULL) {
		logp->ll_log_err(logp, -1, "Unable to create query");
		return (1);
	}

	if (!lattutil_sqlite_exec(query)) {
		logp->ll_log_err(logp, -1, "Unable to exec query");
		return (1);
	}

	lattutil_sqlite_query_free(&query);

	query = lattutil_sqlite_prepare(sqlctx,
	    "INSERT INTO test_table (colname, colval) VALUES (?, ?)");
	if (query == NULL) {
		logp->ll_log_err(logp, -1, "Unable to create second query");
		return (1);
	}

	if (!lattutil_sqlite_bind_string(query, 1, "testname")) {
		logp->ll_log_err(logp, -1, "Unable to bind colname");
		return (1);
	}

	if (!lattutil_sqlite_bind_string(query, 2, "testval")) {
		logp->ll_log_err(logp, -1, "Unable to bind colval");
		return (1);
	}

	if (!lattutil_sqlite_exec(query)) {
		logp->ll_log_err(logp, -1, "Unable to exec second query");
		return (1);
	}

	lattutil_sqlite_query_free(&query);

	query = lattutil_sqlite_prepare(sqlctx,
	    "SELECT * FROM test_table");
	if (query == NULL) {
		logp->ll_log_err(logp, -1, "Unable to create third query");
		return (1);
	}

	if (!lattutil_sqlite_exec(query)) {
		logp->ll_log_err(logp, -1, "Unable to exec third query");
		return (1);
	}

	for (i = 0; i < query->lsq_result.lsr_ncolumns; i++) {
		printf("Column name: %s\n", query->lsq_result.lsr_column_names[i]);
	}

	it = NULL;
	while ((cur = ucl_iterate_object(query->lsq_result.lsr_rows, &it, true))) {
		it_obj = NULL;
		while ((tmp = ucl_iterate_object(cur, &it_obj, true))) {
			switch (ucl_object_type(tmp)) {
			case UCL_STRING:
				printf("%s\n", ucl_object_tostring(tmp));
				break;
			default:
				printf("Unkown type: %s\n",
				    ucl_object_type_to_string(ucl_object_type(tmp)));
			}
		}
	}

	printf("JSON blob:\n%s\n",
	    ucl_object_emit(query->lsq_result.lsr_rows, UCL_EMIT_JSON));

	cur = lattutil_sqlite_get_row(query, 0);
	if (cur == NULL) {
		printf("Could not get the row\n");
		return (1);
	}
	cur = lattutil_sqlite_get_column(cur, 0);
	if (cur == NULL) {
		printf("Could not get the column\n");
		return (1);
	}

	printf("row 0 col 0: %s\n", ucl_object_tostring(cur));

	lattutil_sqlite_query_free(&query);
	lattutil_log_free(&logp);

	return (0);
}
