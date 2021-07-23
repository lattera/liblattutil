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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <limits.h>

#include "liblattutil.h"

#define QUERY_GETLOGGER(q) ((q)->lsq_sql_ctx->lsq_logger)

static bool _lattutil_sqlite_add_row(lattutil_sqlite_query_t *);
static bool _lattutil_sqlite_add_column_names(lattutil_sqlite_query_t *, size_t);
static void _lattutil_sqlite_log_query(lattutil_sqlite_query_t *);

EXPORTED_SYM
lattutil_sqlite_ctx_t *
lattutil_sqlite_ctx_new(const char *path, lattutil_log_t *logger,
    uint64_t flags)
{
	lattutil_sqlite_ctx_t *ctx;

	if (path == NULL) {
		return (NULL);
	}

	ctx = calloc(1, sizeof(*ctx));
	if (ctx == NULL) {
		return (NULL);
	}

	ctx->lsq_version = LATTUTIL_VERSION;

	ctx->lsq_path = strdup(path);
	if (ctx->lsq_path == NULL) {
		free(ctx);
		return (NULL);
	}

	ctx->lsq_logger = logger;
	if (ctx->lsq_logger == NULL) {
		ctx->lsq_logger = lattutil_log_init(NULL, -1);
		if (ctx->lsq_logger == NULL) {
			free(ctx->lsq_path);
			free(ctx);
			return (NULL);
		}
	}

	if (sqlite3_open(path, &(ctx->lsq_sqlctx)) != SQLITE_OK) {
		lattutil_log_free(&(ctx->lsq_logger));
		free(ctx->lsq_path);
		free(ctx);
		return (NULL);
	}

	ctx->lsq_flags = flags;

	return (ctx);
}

EXPORTED_SYM
void
lattutil_sqlite_ctx_free(lattutil_sqlite_ctx_t **ctx)
{
	lattutil_sqlite_ctx_t *ctxp;

	if (ctx == NULL || *ctx == NULL) {
		return;
	}

	ctxp = *ctx;

	if (ctxp->lsq_sqlctx != NULL) {
		sqlite3_close(ctxp->lsq_sqlctx);
	}

	free(ctxp->lsq_path);
	memset(ctxp, 0, sizeof(*ctxp));
	free(ctxp);
	*ctx = NULL;
}

EXPORTED_SYM
uint64_t
lattutil_sqlite_ctx_get_flags(lattutil_sqlite_ctx_t *ctx)
{

	if (ctx == NULL) {
		return (0);
	}

	return (ctx->lsq_flags);
}

EXPORTED_SYM
uint64_t
lattutil_sqlite_ctx_set_flags(lattutil_sqlite_ctx_t *ctx, uint64_t flags)
{
	uint64_t old_flags;

	if (ctx == NULL) {
		return (0);
	}

	old_flags = ctx->lsq_flags;

	ctx->lsq_flags = flags;

	return (old_flags);
}

EXPORTED_SYM
uint64_t
lattutil_sqlite_ctx_set_flag(lattutil_sqlite_ctx_t *ctx, uint64_t flag)
{
	uint64_t old_flags;

	if (ctx == NULL) {
		return (0);
	}

	old_flags = ctx->lsq_flags;

	ctx->lsq_flags |= flag;

	return (old_flags);
}

EXPORTED_SYM
uint64_t
lattutil_sql_query_get_flags(lattutil_sqlite_query_t *query)
{

	if (query == NULL || query->lsq_sql_ctx == NULL) {
		return (0);
	}

	return (lattutil_sqlite_ctx_get_flags(query->lsq_sql_ctx));
}

EXPORTED_SYM
void
lattutil_sqlite_ctx_set_aux(lattutil_sqlite_ctx_t *ctx, void *aux, size_t sz)
{

	if (ctx == NULL) {
		return;
	}

	ctx->lsq_aux = aux;
	ctx->lsq_auxsz = sz;
}

EXPORTED_SYM
void *
lattutil_sqlite_ctx_get_aux(lattutil_sqlite_ctx_t *ctx, size_t *sz)
{

	if (ctx == NULL) {
		return (NULL);
	}

	if (sz != NULL) {
		*sz = ctx->lsq_auxsz;
	}

	return (ctx->lsq_aux);
}

EXPORTED_SYM
uint64_t
lattutil_sqlite_get_version(lattutil_sqlite_ctx_t *ctx)
{

	if (ctx == NULL) {
		return (0);
	}

	return (ctx->lsq_version);
}

EXPORTED_SYM
lattutil_sqlite_query_t *
lattutil_sqlite_prepare(lattutil_sqlite_ctx_t *ctx, const char *query_string)
{
	lattutil_sqlite_query_t *query;
	bool bool_arg, sqlquery;
	char *str_arg;
	int int_arg;
	va_list args;
	size_t i;
	int res;

	if (ctx == NULL || query_string == NULL) {
		return (NULL);
	}

	query = calloc(1, sizeof(*query));
	if (query == NULL) {
		perror("calloc");
		return (NULL);
	}

	query->lsq_querystr = strdup(query_string);
	if (query->lsq_querystr == NULL) {
		perror("stdrdup");
		free(query);
		return (NULL);
	}

	query->lsq_result.lsr_rows = ucl_object_typed_new(UCL_ARRAY);
	if (query->lsq_result.lsr_rows == NULL) {
		free(query->lsq_querystr);
		free(query);
		return (NULL);
	}

	res = sqlite3_prepare(ctx->lsq_sqlctx, query_string, -1,
	    &(query->lsq_stmt), NULL);
	if (res != SQLITE_OK || query->lsq_stmt == NULL) {
		free(query);
		return (NULL);
	}

	query->lsq_sql_ctx = ctx;

	return (query);
}

EXPORTED_SYM
lattutil_sqlite_ctx_t *
lattutil_sqlite_query_get_ctx(lattutil_sqlite_query_t *query)
{

	if (query == NULL) {
		return (NULL);
	}

	return (query->lsq_sql_ctx);
}

EXPORTED_SYM
void
lattutil_sqlite_query_free(lattutil_sqlite_query_t **query)
{
	lattutil_sqlite_query_t *queryp;
	size_t i;

	if (query == NULL || *query == NULL) {
		return;
	}

	queryp = *query;
	free(queryp->lsq_querystr);

	if (queryp->lsq_result.lsr_column_names != NULL) {
		for (i = 0; i < queryp->lsq_result.lsr_ncolumns; i++) {
			free(queryp->lsq_result.lsr_column_names[i]);
		}

		free(queryp->lsq_result.lsr_column_names);
	}

	memset(queryp, 0, sizeof(*queryp));
	free(queryp);

	*query = NULL;
}

EXPORTED_SYM
lattutil_sql_res_t *
lattutil_sqlite_get_result(lattutil_sqlite_query_t *query)
{

	if (query == NULL) {
		return (NULL);
	}

	return (&(query->lsq_result));
}

EXPORTED_SYM
bool
lattutil_sqlite_bind_int(lattutil_sqlite_query_t *query, int paramno,
    int64_t val)
{

	if (query == NULL) {
		return (false);
	}

	return (sqlite3_bind_int64(query->lsq_stmt, paramno, val) ==
	    SQLITE_OK);
}

EXPORTED_SYM
bool
lattutil_sqlite_bind_string(lattutil_sqlite_query_t *query, int paramno,
    const char *val)
{

	if (query == NULL || val == NULL) {
		return (false);
	}

	return (sqlite3_bind_text(query->lsq_stmt, paramno, val, -1,
	    SQLITE_TRANSIENT) == SQLITE_OK);
}

EXPORTED_SYM
bool
lattutil_sqlite_bind_blob(lattutil_sqlite_query_t *query, int paramno,
   void *val, size_t sz)
{

	if (query == NULL || val == NULL || sz > INT_MAX) {
		return (false);
	}

	return (sqlite3_bind_blob(query->lsq_stmt, paramno, val,
	    (int)sz, NULL) == SQLITE_OK);
}

EXPORTED_SYM
bool
lattutil_sqlite_bind_time(lattutil_sqlite_query_t *query, int paramno,
    time_t val)
{

	if (query == NULL) {
		return (NULL);
	}

	return (sqlite3_bind_int64(query->lsq_stmt, paramno, val) ==
	    SQLITE_OK);
}

EXPORTED_SYM
bool
lattutil_sqlite_exec(lattutil_sqlite_query_t *query)
{
	lattutil_log_t *logger;
	bool ret;
	int res;

	if (query == NULL || query->lsq_stmt == NULL) {
		return (false);
	}

	logger = QUERY_GETLOGGER(query);

	_lattutil_sqlite_log_query(query);

	ret = true;

	while (true) {
		res = sqlite3_step(query->lsq_stmt);
		switch (res) {
		case SQLITE_DONE:
			goto end;
		case SQLITE_ROW:
			if (!_lattutil_sqlite_add_row(query)) {
				logger->ll_log_err( logger, -1,
				    "Unable to add row to sqlite object");
				ret = false;
				goto end;
			}
			break;
		default:
			logger->ll_log_err(logger, -1,
			    "Unhandled sqlite3_step result: %d", res);
			ret = false;
			goto end;
		}
	}

end:
	sqlite3_finalize(query->lsq_stmt);
	query->lsq_stmt = NULL;

	return (ret);
}

EXPORTED_SYM
const ucl_object_t *
lattutil_sqlite_get_row(lattutil_sqlite_query_t *query, size_t rowid)
{
	const ucl_object_t *obj;
	ucl_object_iter_t it;
	size_t i;

	if (query == NULL) {
		return (NULL);
	}

	it = NULL;
	i = 0;
	while ((obj = ucl_iterate_object(query->lsq_result.lsr_rows, &it,
	    true))) {
		if (i++ == rowid) {
			return (obj);
		}
	}

	return (NULL);
}

EXPORTED_SYM
const ucl_object_t *
lattutil_sqlite_get_column(const ucl_object_t *row, size_t colid)
{
	const ucl_object_t *obj;
	ucl_object_iter_t it;
	size_t i;

	if (row == NULL) {
		return (NULL);
	}

	it = NULL;
	i = 0;
	while ((obj = ucl_iterate_object(row, &it, true))) {
		if (i++ == colid) {
			return (obj);
		}
	}

	return (NULL);
}

EXPORTED_SYM
const char *
lattutil_sqlite_get_column_string(const ucl_object_t *row, size_t colid)
{
	const ucl_object_t *obj;

	if (row == NULL) {
		return (NULL);
	}

	obj = lattutil_sqlite_get_column(row, colid);
	if (obj != NULL) {
		return (ucl_object_tostring(obj));
	}

	return (NULL);
}

EXPORTED_SYM
int64_t
lattutil_sqlite_get_column_int(const ucl_object_t *row, size_t colid, int64_t def)
{
	const ucl_object_t *obj;

	if (row == NULL) {
		return (def);
	}

	obj = lattutil_sqlite_get_column(row, colid);
	if (obj != NULL) {
		return (ucl_object_toint(obj));
	}

	return (def);
}

static bool
_lattutil_sqlite_add_row(lattutil_sqlite_query_t *query)
{
	ucl_object_t *colobj, *rowobj;
	size_t blobsz, i, ncols;
	lattutil_log_t *logger;
	const char *sval;
	int *ival, res;
	void *pval;
	bool ret;

	if (query == NULL) {
		return (false);
	}

	ret = true;
	colobj = NULL;
	rowobj = NULL;

	logger = QUERY_GETLOGGER(query);

	ncols = sqlite3_column_count(query->lsq_stmt);
	if (ncols <= 0) {
		/* This should not happen */
		ret = false;
		goto end;
	}

	if (query->lsq_result.lsr_column_names == NULL) {
		if (!_lattutil_sqlite_add_column_names(query, ncols)) {
			ret = false;
			goto end;
		}
	}

	rowobj = ucl_object_typed_new(UCL_ARRAY);
	if (rowobj == NULL) {
		ret = false;
		goto end;
	}


	for (i = 0; i < ncols; i++) {
		switch (sqlite3_column_type(query->lsq_stmt, i)) {
		case SQLITE_INTEGER:
			colobj = ucl_object_fromint(
			    sqlite3_column_int64(query->lsq_stmt, i));
			break;
		case SQLITE_TEXT:
			sval = (const char *)sqlite3_column_text(
			    query->lsq_stmt, i);
			colobj = ucl_object_fromstring(sval);
			break;
		case SQLITE_BLOB:
			blobsz = sqlite3_column_bytes(query->lsq_stmt, i);
			if (blobsz > 0) {
				colobj = ucl_object_fromlstring(
				    sqlite3_column_blob(query->lsq_stmt, i),
				    blobsz);
			} else {
				colobj = ucl_object_typed_new(UCL_NULL);
			}
			break;
		case SQLITE_NULL:
			colobj = ucl_object_typed_new(UCL_NULL);
			break;
		case SQLITE_FLOAT:
			/* TODO: Support float */
			break;
		default:
			colobj = NULL;
			break;
		}

		if (colobj == NULL) {
			ret = false;
			logger->ll_log_err(logger, -1,
			    "Unable to determine columnal data format");
			goto end;
		}

		if (!ucl_array_append(rowobj, colobj)) {
			ret = false;
			logger->ll_log_err(logger, -1,
			    "Unable to append column to row");
			goto end;
		}
	}

	if (!ucl_array_append(query->lsq_result.lsr_rows, rowobj)) {
		logger->ll_log_err(logger, -1,
		    "Unable to append row to rows object");
	}

end:
	if (ret == false) {
		if (rowobj != NULL) {
			ucl_object_unref(rowobj);
			rowobj = NULL;
		}
	}
	return (ret);
}

static bool
_lattutil_sqlite_add_column_names(lattutil_sqlite_query_t *query, size_t ncols)
{
	size_t i;

	if (query == NULL || ncols == 0) {
		return (false);
	}

	query->lsq_result.lsr_ncolumns = ncols;
	query->lsq_result.lsr_column_names = calloc(ncols, sizeof(char **));
	if (query->lsq_result.lsr_column_names == NULL) {
		return (false);
	}

	for (i = 0; i < ncols; i++) {
		query->lsq_result.lsr_column_names[i] = strdup(
		    sqlite3_column_name(query->lsq_stmt, i));
		if (query->lsq_result.lsr_column_names[i] == NULL) {
			return (false);
		}
	}

	return (true);
}

static void
_lattutil_sqlite_log_query(lattutil_sqlite_query_t *query)
{
	lattutil_log_t *logger;

	if (query == NULL) {
		return;
	}

	logger = QUERY_GETLOGGER(query);

	if (!LATTUTIL_SQL_FLAG_ISSET(query->lsq_sql_ctx,
	    LATTUTIL_SQL_FLAG_LOG_QUERY)) {
		return;
	}

	logger->ll_log_debug(logger, -1, "SQL query: %s", query->lsq_querystr);
}
