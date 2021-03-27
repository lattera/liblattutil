SHLIB=		lattutil
SHLIB_MAJOR=	0
SRCS=		config.c
INCS=		liblattutil.h

.PATH: ${.CURDIR}/src
.PATH: ${.CURDIR}/include

CFLAGS+=	-I${.CURDIR}/include
CFLAGS+=	-I/usr/local/include
LDFLAGS+=	-L/usr/local/lib

LDADD+=		-lucl

.if defined(PREFIX)
INCLUDEDIR=	${PREFIX}/include
LIBDIR=		${PREFIX}/lib
.endif

.include <bsd.lib.mk>
