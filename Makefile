SHLIB=		lattutil
SHLIB_MAJOR=	0
INCS=		liblattutil.h

SRCS+=		config.c
SRCS+=		log-main.c
SRCS+=		log-syslog.c

.PATH: ${.CURDIR}/src
.PATH: ${.CURDIR}/include

CFLAGS+=	-D_lattutil_internal
CFLAGS+=	-I${.CURDIR}/include
CFLAGS+=	-I/usr/local/include
LDFLAGS+=	-L/usr/local/lib

LDADD+=		-lucl

.if defined(PREFIX)
INCLUDEDIR=	${PREFIX}/include
LIBDIR=		${PREFIX}/lib
.endif

.include <bsd.lib.mk>
