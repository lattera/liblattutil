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

#include <ucl.h>

struct _lllog;

#define	EXPORTED_SYM	__attribute__((visibility("default")))

#define LATTUTIL_LOG_VERBOSITY_DEFAULT	1000
#define LATTUTIL_LOG_DEFAULT_NAME	"lattutil"

typedef ssize_t (*log_cb)(struct _lllog *, int, const char *, ...);
typedef void (*log_close)(struct _lllog *);

/*
 * Though we export the underlying data structure, using the API is
 * preferred over directly accessing the ABI.
 */
typedef struct _llconfig {
	struct stat		 l_sb;
	char			*l_path;
	int			 l_fd;
	struct ucl_parser	*l_parser;
	const ucl_object_t	*l_rootobj;
} lattutil_config_path_t;

typedef struct _lllog {
	int		 ll_verbosity;
	char		*ll_path;

	log_cb		 ll_log_err;
	log_cb		 ll_log_info;
	log_cb		 ll_log_warn;

	log_close	 ll_log_close;
} lattutil_log_t;

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
 * Initialize syslog-based logging
 *
 * @param Logging object
 * @param syslog(3) logopt
 * @param syslog(3) facility
 * @return True on success, False otherwise
 */
bool lattutil_log_syslog_init(lattutil_log_t *, int, int);

#ifdef _lattutil_internal
ssize_t lattutil_log_syslog_err(lattutil_log_t *, int,
    const char *, ...);
ssize_t lattutil_log_syslog_info(lattutil_log_t *, int,
    const char *, ...);
ssize_t lattutil_log_syslog_warn(lattutil_log_t *, int,
    const char *, ...);
void lattutil_log_syslog_close(lattutil_log_t *);
#endif /* _lattutil_internal */

#endif /* !_LIBLATTUTIL_H */
