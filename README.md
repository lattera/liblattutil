# liblattutil

* License: 2-Clause BSD
* Author: Shawn Webb <shawn.webb@hardenedbsd.org>

This is a collection of code I've written over the years, rewritten to
be available as a C API. This project isn't fit for production use,
yet, and both the ABI and API may change arbitrarily.

## Logging

The logging API supports the following backends:

* Dummy
* Syslog
* stdio

The dummy backend simply discards any messages passed to it. The
syslog backend sends messages to syslog, if the message meets or
exceeds the minimum verbosity configuration parameter. If `-1` is
passed as the verbosity level, a default minimum verbosity is set. The
current default is 1000.

Do not terminate the log message with a newline.

Sample syslog logging code:

```C
#include <liblattutil.h>

lattutil_log_t *logp;
logp = lattutil_log_init("myApp", 5);
if (logp == NULL) {
	Fatal();
}

/*
 * if lattutil_log_syslog_init isn't called, the logger will use the
 * dummy logger.
 */

lattutil_log_syslog_init(logp, LOG_PID, LOG_USER);
logp->ll_log_info(logp, 2, "This debugging message will get ignored"
    " because 2 is less than 5");
logp->ll_log_info(logp, 5, "This will get logged");
lattutil_log_free(&logp);
```

## SQLite3 Queries

liblattutil has a SQLite3 abstraction layer. Right now, it only
supports those data types that I myself care about. I'm open to
accepting patches for supporting unimplemented data types.

The sqlite3 APIs also support the logging APIs described above. When
creating a sqlite3 context, you have the option of specifying a
logging context object. Error, info, and debugging messages will be
reported through the logging object.

Sample sqlite3 code:

```C
#include <liblattutil.h>

lattutil_sqlite_query_t *query;
lattutil_sqlite_ctx_t *ctx;
const ucl_object_t *row;

ctx = lattutil_sqlite_ctx_new("/tmp/db.sqlite3", NULL, 0);
if (ctx == NULL) {
	Fatal();
}

query = lattutil_sqlite_prepare(ctx, "SELECT id FROM table WHERE id = ?");
if (query == NULL) {
	Fatal();
}

if (!lattutil_sqlite_bind_int(query, 1, 1234)) {
	Fatal();
}

if (!lattutil_sqlite_exec(query)) {
	Fatal();
}

row = lattutil_sqlite_get_row(query, 0);

printf("ID: %ld\n", lattutil_sqlite_get_column_int(row, 0, 0));
```
