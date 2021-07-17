PROG=	lattutil
MAN=

SRCS+=	lattutil.c

CFLAGS+=	-I${.CURDIR}
CFLAGS+=	-I${.CURDIR}/../include
CFLAGS+=	-I/usr/local/include

LDFLAGS+=	-L${.CURDIR}/../obj
LDFLAGS+=	-L/usr/local/lib

LDADD+=		-llattutil -lucl

.include <bsd.prog.mk>
