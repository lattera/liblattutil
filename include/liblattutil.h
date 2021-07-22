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

#ifndef _LIBLATTUTIL_H
#define	_LIBLATTUTIL_H

#include <stdbool.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <stdarg.h>

#include <sqlite3.h>
#include <ucl.h>

#define LATTUTIL_VERSION	1

struct _lllog;
struct _sqlite_ctx;
typedef struct _sqlite_ctx sqlite_ctx_t;

#define	EXPORTED_SYM	__attribute__((visibility("default")))

#define LATTUTIL_LOG_VERBOSITY_DEFAULT	1000
#define LATTUTIL_LOG_DEFAULT_NAME	"lattutil"

#define LATTUTIL_SQL_FLAG_LOG_QUERY	0x1

#define	LATTUTIL_SQL_FLAG_ISSET(q, f) (((q)->lsq_flags & f) == f)

typedef ssize_t (*log_cb)(struct _lllog *, int, const char *, ...);
typedef void (*log_close)(struct _lllog *);

/*
 * Though we export the underlying data structure, using the API is
 * preferred over directly accessing the ABI.
 */
typedef struct _llconfig {
	uint64_t		 l_version;
	struct stat		 l_sb;
	char			*l_path;
	int			 l_fd;
	struct ucl_parser	*l_parser;
	const ucl_object_t	*l_rootobj;
	void			*l_aux;
	size_t			 l_auxsz;
} lattutil_config_path_t;

typedef struct _lllog {
	uint64_t	 ll_version;
	int		 ll_verbosity;
	char		*ll_path;
	void		*ll_aux;
	size_t		 ll_auxsz;
	void		*ll_internalaux;
	size_t		 ll_internalauxsz;

	log_cb		 ll_log_debug;
	log_cb		 ll_log_err;
	log_cb		 ll_log_info;
	log_cb		 ll_log_warn;

	log_close	 ll_log_close;
} lattutil_log_t;

typedef struct _lattutil_sql_ctx {
	uint64_t	 lsq_version;
	sqlite3		*lsq_sqlctx;
	char		*lsq_path;
	uint64_t	 lsq_flags;
	lattutil_log_t	*lsq_logger;
	void		*lsq_aux;
	size_t		 lsq_auxsz;
	void		*lsq_internalaux;
	size_t		 lsq_internalauxsz;
} lattutil_sqlite_ctx_t;

typedef struct _lattutil_sql_res {
	char			**lsr_column_names;
	ucl_object_t		*lsr_rows;
	size_t			 lsr_ncolumns;
} lattutil_sql_res_t;

typedef struct _lattutil_sqlite_query {
	lattutil_sqlite_ctx_t	*lsq_sql_ctx;
	sqlite3_stmt		*lsq_stmt;
	char			*lsq_querystr;
	lattutil_sql_res_t	 lsq_result;
	bool			 lsq_executed;
} lattutil_sqlite_query_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Find a configuration file in a set of given paths
 *
 * This function iterates through the set of paths, looking for the
 * passed-in configuration file name. If the configuration file is
 * found, a UCL parser instance is created and the file is loaded by
 * UCL.
 *
 * @param Set of paths
 * @param Number of paths in set
 * @param Configuration file name
 * @return Pointer to struct that contains information about the
 * 	config file on success, NULL on error.
 */
lattutil_config_path_t *lattutil_find_config(const char **, size_t, const char *,
    int);

/**
 * Free a given lattutil_config_path_t object
 *
 * In order to prevent UAF vulnerabilities, the structure is zeroed
 * after all its members have been freed. Additionally, the pointer to
 * the structure is set to NULL.
 *
 * @param A pointer to the to-be-freed object
 */
void lattutil_free_config_path(lattutil_config_path_t **);

/**
 * Set the auxiliary members of the config context object
 *
 * @param The config object
 * @param The auxiliary data
 * @param The size of the auxiliary data
 */
void lattutil_config_set_aux(lattutil_config_path_t *, void *, size_t);

/**
 * Get the auxiliary members of the config context object
 *
 * @param The config object
 * @param[out] Optional output variable to store the recorded size of
 *     the auxiliary member.
 * @return The auxiliary member
 */
void *lattutil_config_get_aux(lattutil_config_path_t *, size_t *);

/**
 * Get the ABI version of the config path object
 *
 * @param The config path object
 * @return The version number
 */
uint64_t lattutil_config_get_version(lattutil_config_path_t *);

/**
 * Return the fully-qualified path of the config file
 *
 * @param A pointer to the lattutil_config_path_t object
 * @return The path of the config file
 */
char *lattutil_config_path_path(lattutil_config_path_t *);

/**
 * Return the file descriptor of the opened config
 *
 * @param A pointer to the lattutil_config_path_t object
 * @return The file descriptor
 */
int lattutil_config_path_fd(lattutil_config_path_t *);

/**
 * Look for a string-typed variable in a UCL configuration tree.
 *
 * @param UCL object root
 * @param Path within the UCL object root
 * @param Default value to set if path is not found
 * @return The value if found, default value or zero-length string on
 * error.
 */
char *lattutil_find_config_string(const ucl_object_t *, const char *,
    const char *);

/**
 * Look for an integer-typed variable in a UCL configuration tree.
 *
 * @param UCL object root
 * @param Path within the UCL object root
 * @param Default value to set if path is not found
 * @return The value if found, default value if not
 */
int64_t find_config_int(const ucl_object_t *, const char *, int64_t);

/**
 * Initialize logger
 *
 * @param Path
 * @param Verbosity
 * @return Pointer to the logging object
 */
lattutil_log_t *lattutil_log_init(char *, int);

/**
 * Free logger
 *
 * The underlying variable is set to NULL.
 *
 * @param Double pointer to the logging object
 */
void lattutil_log_free(lattutil_log_t **);

/**
 * Get the ABI version number of the log object
 *
 * @param The log object
 * @return The ABI version number
 */
uint64_t lattutil_log_get_version(lattutil_log_t *);

/**
 * Initialize syslog-based logging
 *
 * @param Logging object
 * @param syslog(3) logopt
 * @param syslog(3) facility
 * @return True on success, False otherwise
 */
bool lattutil_log_syslog_init(lattutil_log_t *, int, int);

/**
 * Initialize NULL logging
 *
 * This is the default logger. It simply discards all logging
 * messages.
 *
 * @param Logging object
 */
void lattutil_log_dummy_init(lattutil_log_t *);

/**
 * Initialize stdio-based logging
 *
 * @param Logging object
 * @return True on success, False otherwise
 */
bool lattutil_log_stdio_init(lattutil_log_t *);

/**
 * Determine if the logging subsystem is ready to receive messages
 *
 * @param Pointer to the logging object
 */
bool lattutil_log_ready(lattutil_log_t *);

/**
 * Get the configured minimum verbosity level
 *
 * @param Logging object
 * @return Verbosity level
 */
int lattutil_log_verbosity(lattutil_log_t *);

/**
 * Set the minimum verbosity level
 *
 * @param Logging object
 * @param Verbosity level
 * @return The previous verbosity level
 */
int lattutil_log_set_verbosity(lattutil_log_t *, int);

/**
 * Set the auxiliary members of the logging context object
 *
 * @param The logging object
 * @param The auxiliary data
 * @param The size of the auxiliary data
 */
void lattutil_log_set_aux(lattutil_log_t *, void *, size_t);

/**
 * Get the auxiliary members of the logging context object
 *
 * @param The logging object
 * @param[out] Optional output variable to store the recorded size of
 *     the auxiliary member.
 * @return The auxiliary member
 */
void *lattutil_log_get_aux(lattutil_log_t *, size_t *);

/**
 * Create new SQLite3 context object
 *
 * @param Path to the database file
 * @param Optional logger (NULL means no logging)
 * @param flags (0)
 * @return lattutil SQLite3 context object
 */
lattutil_sqlite_ctx_t *lattutil_sqlite_ctx_new(const char *, lattutil_log_t *,
    uint64_t);

/**
 * Free The SQLite3 context object
 *
 * @param Double pointer to the context object
 */
void lattutil_sqlite_ctx_free(lattutil_sqlite_ctx_t **);

/**
 * Get the flags of the SQLite3 context object
 *
 * @param The context object
 * @return The flags of the context object
 */
uint64_t lattutil_sqlite_ctx_get_flags(lattutil_sqlite_ctx_t *);

/**
 * Return the flags of the context object associated with a query
 *
 * @param The query object
 * @return The flags of the context object associtated with the query
 */
uint64_t lattutil_sql_query_get_flags(lattutil_sqlite_query_t *);

/**
 * Set the flags of a sqlite3 context object
 *
 * @param The sqlite3 context object
 * @param The flags
 * @return The old flags
 */
uint64_t lattutil_sqlite_ctx_set_flags(lattutil_sqlite_ctx_t *, uint64_t);

/**
 * Set a single flag of a sqlite3 context object
 *
 * @param The sqlite3 context object
 * @param The flag
 * @return The old flags
 */
uint64_t lattutil_sqlite_ctx_set_flag(lattutil_sqlite_ctx_t *, uint64_t);

/**
 * Set the auxiliary members of the sqlite context object
 *
 * @param The sqlite context object
 * @param The auxiliary data
 * @param The size of the auxiliary data
 */
void lattutil_sqlite_ctx_set_aux(lattutil_sqlite_ctx_t *, void *, size_t);

/**
 * Get the auxiliary members of the sqlite context object
 *
 * @param The sqlite context object
 * @param[out] Optional output variable to store the recorded size of
 *     the auxiliary member.
 * @return The auxiliary member
 */
void *lattutil_sqlite_ctx_get_aux(lattutil_sqlite_ctx_t *, size_t *);

/**
 * Get the ABI version number of the sqlite wrapper
 *
 * @param The lattutil sqlite context object
 * @return The ABI version number
 */
uint64_t lattutil_sqlite_get_version(lattutil_sqlite_ctx_t *);

/**
 * Prepare a new query
 *
 * @param The lattutil SQLite3 context object
 * @param The query
 */
lattutil_sqlite_query_t *lattutil_sqlite_prepare(lattutil_sqlite_ctx_t *,
    const char *);

lattutil_sqlite_ctx_t *lattutil_sqlite_query_get_ctx(
    lattutil_sqlite_query_t *);

/**
 * Free a query object
 *
 * @param Double pointer to the query object
 */
void lattutil_sqlite_query_free(lattutil_sqlite_query_t **);

/**
 * Get the embedded result object in the query
 *
 * @param The query object
 * @return The query's result object
 */
lattutil_sql_res_t *lattutil_sqlite_get_result(lattutil_sqlite_query_t *);

/**
 * Bind an integer value to the query
 *
 * @param The query object
 * @param The query param number
 * @param The integer to be bound
 * @return Whether the param bound successfully
 */
bool lattutil_sqlite_bind_int(lattutil_sqlite_query_t *, int, int64_t);

/**
 * Bind a string value to the query
 *
 * @param The query object
 * @param The query param number
 * @param The string to be bound
 * @return Whether the param bound successfully
 */
bool lattutil_sqlite_bind_string(lattutil_sqlite_query_t *, int, const char *);

/**
 * Bind a blob value to the query
 *
 * @param The query object
 * @param The query param number
 * @param The blob to be bound
 * @param The size of the blob
 * @return Whether the param bound successfully
 */
bool lattutil_sqlite_bind_blob(lattutil_sqlite_query_t *, int, void *, size_t);

/**
 * Bind a time_t value to the query
 *
 * Note that this is not 64-bit time safe. That will change soon.
 *
 * @param The query object
 * @param The query param number
 * @param The time_t to be bound
 * @return Whether the param bound successfully
 *
 */
bool lattutil_sqlite_bind_time(lattutil_sqlite_query_t *, int, time_t);

/**
 * Execute the query
 *
 * @param The query to be executed
 * @return Whether the query executed successfully
 */
bool lattutil_sqlite_exec(lattutil_sqlite_query_t *);

/**
 * Look up a row in the query result
 *
 * @param The query object
 * @param The integer row ID
 * @return UCL object of the row
 */
const ucl_object_t *lattutil_sqlite_get_row(lattutil_sqlite_query_t *, size_t);

/**
 * Look up a column in a row object
 *
 * @param The UCL object containing the row
 * @param The integer column ID
 * @return UCL object of the column
 */
const ucl_object_t *lattutil_sqlite_get_column(const ucl_object_t *, size_t);

/**
 * Return a column as a string
 *
 * @param The UCL object containing the row
 * @param The integer column ID
 * @return The string value of the column
 */
const char *lattutil_sqlite_get_column_string(const ucl_object_t *, size_t);

/**
 * Return a column as a 64-bit signed integer
 *
 * @param The UCL object containing the row
 * @param The integer column ID
 * @param The default value if the column cannot be found or converted
 *        to int64_t
 * @return The integer value of the column or the default value
 */
int64_t lattutil_sqlite_get_column_int(const ucl_object_t *, size_t, int64_t);

#ifdef _lattutil_internal
ssize_t lattutil_log_syslog_debug(lattutil_log_t *, int,
    const char *, ...);
ssize_t lattutil_log_syslog_err(lattutil_log_t *, int,
    const char *, ...);
ssize_t lattutil_log_syslog_info(lattutil_log_t *, int,
    const char *, ...);
ssize_t lattutil_log_syslog_warn(lattutil_log_t *, int,
    const char *, ...);
void lattutil_log_syslog_close(lattutil_log_t *);

ssize_t lattutil_log_dummy_debug(lattutil_log_t *, int,
    const char *, ...);
ssize_t lattutil_log_dummy_err(lattutil_log_t *, int,
    const char *, ...);
ssize_t lattutil_log_dummy_info(lattutil_log_t *, int,
    const char *, ...);
ssize_t lattutil_log_dummy_warn(lattutil_log_t *, int,
    const char *, ...);
void lattutil_log_dummy_close(lattutil_log_t *);

ssize_t lattutil_log_stdio_debug(lattutil_log_t *, int,
    const char *, ...);
ssize_t lattutil_log_stdio_err(lattutil_log_t *, int,
    const char *, ...);
ssize_t lattutil_log_stdio_info(lattutil_log_t *, int,
    const char *, ...);
ssize_t lattutil_log_stdio_warn(lattutil_log_t *, int,
    const char *, ...);
void lattutil_log_stdio_close(lattutil_log_t *);
#endif /* _lattutil_internal */

#ifdef __cplusplus
}
#endif

#endif /* !_LIBLATTUTIL_H */
