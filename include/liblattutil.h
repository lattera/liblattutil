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

#include <ucl.h>

#define	EXPORTED_SYM	__attribute__((visibility("default")))

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

#endif /* !_LIBLATTUTIL_H */
