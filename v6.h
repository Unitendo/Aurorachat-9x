#ifndef V6_H
#define V6_H

#include <stddef.h>

extern int V6_init();
extern void V6_end();
extern int V6_connect(const char *host, short port);
extern void V6_onconnecterr();
extern int V6_httppost(
        const char *host, short port,
        const char *path, const char *auth, const char *data,
        char *outbuf, size_t outsize
);
extern int V6_loginrequest(
        const char *host, short port,
        const char *username, const char *password,
        char *outbuf, size_t outsize
);
extern int V6_signuprequest(
        const char *host, short port,
        const char *username, const char *password,
        char *outbuf, size_t outsize
);
extern int V6_roomrequest(
        const char *host, short port,
        char *outbuf, size_t outsize
);
extern int V6_messagerequest(
        const char *host, short port,
        const char *auth, const char *room, const char *msg
);

#endif

