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

#include <fcntl.h>
#include <sys/stat.h>

#include "liblattutil.h"

EXPORTED_SYM
lattutil_config_path_t *
lattutil_find_config(const char **paths, size_t npaths,
    const char *filename, int flags)
{
	lattutil_config_path_t *res;
	size_t i, sz;

	res = calloc(1, sizeof(*res));
	if (res == NULL) {
		return (NULL);
	}

	res->l_version = LATTUTIL_VERSION;

	res->l_fd = -1;
	for (i = 0; i < npaths; i++) {
		sz = strlen(paths[i]) + strlen(filename) + 3;
		res->l_path = calloc(1, sz);
		if (res->l_path == NULL) {
			return (res);
		}

		snprintf(res->l_path, sz-1, "%s/%s", paths[i], filename);
		res->l_fd = open(res->l_path, flags);
		if (res->l_fd < 0) {
			free(res->l_path);
			res->l_path = NULL;
			continue;
		}

		if (fstat(res->l_fd, &(res->l_sb))) {
			close(res->l_fd);
			free(res->l_path);
			res->l_path = NULL;
			continue;
		}

		res->l_parser = ucl_parser_new(UCL_PARSER_KEY_LOWERCASE);
		if (res->l_parser == NULL) {
			free(res->l_path);
			close(res->l_fd);
			free(res);
			return (NULL);
		}

		if (!ucl_parser_add_fd(res->l_parser, res->l_fd)) {
			close(res->l_fd);
			free(res->l_path);
			res->l_path = NULL;
			ucl_parser_free(res->l_parser);
			res->l_parser = NULL;
			continue;
		}

		res->l_rootobj = ucl_parser_get_object(res->l_parser);

		break;
	}

	if (i == npaths) {
		lattutil_free_config_path(&res);
	}

	return (res);
}

EXPORTED_SYM
void
lattutil_free_config_path(lattutil_config_path_t **obj)
{

	if (obj == NULL || *obj == NULL) {
		return;
	}
	free((*obj)->l_path);
	(*obj)->l_path = NULL;
	if ((*obj)->l_parser != NULL) {
		ucl_parser_free((*obj)->l_parser);
		(*obj)->l_parser = NULL;
	}
	if ((*obj)->l_fd >= 0) {
		close((*obj)->l_fd);
		(*obj)->l_fd = -1;
	}

	memset(*obj, 0, sizeof(**obj)); /* Prevent UAF */
	free(*obj);
	*obj = NULL; /* Prevent UAF */
}

EXPORTED_SYM
char *
lattutil_config_path_path(lattutil_config_path_t *path)
{

	if (path == NULL) {
		return (NULL);
	}

	return (path->l_path);
}

EXPORTED_SYM
int
lattutil_config_path_fd(lattutil_config_path_t *path)
{

	if (path == NULL) {
		return (-1);
	}

	return (path->l_fd);
}

EXPORTED_SYM
char *
lattutil_find_config_string(const ucl_object_t *root, const char *path,
    const char *def)
{
	const ucl_object_t *obj;

	obj = ucl_object_lookup_path(root, path);
	if (obj == NULL) {
		if (def != NULL) {
			return (strdup(def));
		}

		return (strdup(""));
	}

	return (strdup(""));
}

EXPORTED_SYM
int64_t
lattutil_find_config_int(const ucl_object_t *root, const char *path, int64_t def)
{
	const ucl_object_t *obj;
	int64_t res;

	res = def;
	obj = ucl_object_lookup_path(root, path);
	if (obj != NULL) {
		res = ucl_object_toint(obj);
	}

	return (res);
}
