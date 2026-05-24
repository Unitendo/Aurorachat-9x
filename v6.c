#include "v6.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <winsock.h>

#define HTTP_BUFFER_SIZE 4096

int V6_init() {
        WSADATA wsadata;

        if(WSAStartup(MAKEWORD(1,1), &wsadata)) {
                MessageBox(NULL, "WSAStartup failure.", "V6 Network Error", MB_ICONERROR);
                return 1;
        }

        return 0;
}

void V6_end() {
        WSACleanup();
}

int V6_connect(const char *host, short port) {
        struct sockaddr_in sa;
        int sock;

        sa.sin_family = AF_INET;
        sa.sin_port = htons(port);
        sa.sin_addr.s_addr = inet_addr(host);

        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(sock == INVALID_SOCKET) return -1;

        if(connect(sock, (struct sockaddr *) &sa, sizeof(sa))) return -1;

        return sock;
}

void V6_stripline(char *s) {
        size_t p = strlen(s) - 1;
        if(s[p] == '\r') s[p] = 0;
}

void V6_onconnecterr() {
        int err;
        char buffer[1024];

        err = WSAGetLastError();
        _snprintf(buffer, 1023, "V6 connect failure: %d", err);
        MessageBox(NULL, buffer, "V6 Network Error", MB_ICONERROR);
}

int V6_httppost(
        const char *host, short port,
        const char *path, const char *auth, const char *data,
        char *outbuf, size_t outsize
) {
        int sock;
        char buf[HTTP_BUFFER_SIZE + 1];
        char buf2[HTTP_BUFFER_SIZE + 1];
        size_t offset = 0;
        size_t recved = 0;
        char *token;
        
        sock = V6_connect(host, port);
        if(sock == -1) {
                V6_onconnecterr();
                return 1;
        }

        buf[0] = 0;
        _snprintf(buf2, HTTP_BUFFER_SIZE, "POST %s HTTP/1.0\r\n", path);
        strncat(buf, buf2, HTTP_BUFFER_SIZE);
        if(auth) {
                _snprintf(buf2, HTTP_BUFFER_SIZE, "Auth: %s\r\n", auth);
                strncat(buf, buf2, HTTP_BUFFER_SIZE);
        }
        if(data) {
                _snprintf(buf2, HTTP_BUFFER_SIZE, "Content-Type: text/plain\r\n");
                strncat(buf, buf2, HTTP_BUFFER_SIZE);
                _snprintf(buf2, HTTP_BUFFER_SIZE, "Content-Length: %d\r\n", strlen(data));
                strncat(buf, buf2, HTTP_BUFFER_SIZE);
        }
        strncat(buf, "\r\n", HTTP_BUFFER_SIZE);

        send(sock, buf, strlen(buf), 0);
        if(data) send(sock, data, strlen(data), 0);

        while( (recved = recv(sock, buf + offset, HTTP_BUFFER_SIZE - offset, 0)) ) {
                offset += recved;
                if(offset >= HTTP_BUFFER_SIZE) break;
        }

        buf[offset] = 0;
        token = buf;

        while(1) {
                if(*token == '\n') {
                        token++;
                        if(*token == '\r') token++;
                        if(*token == '\n') break;
                }
                token++;
        }
        token++;

        if(outbuf && outsize) {
                strncpy(outbuf, token, outsize);
                outbuf[outsize - 1] = 0;
        }

        return 0;
}

int V6_loginrequest(
        const char *host, short port,
        const char *username, const char *password,
        char *outbuf, size_t outsize
) {
        char buffer[1024];
        _snprintf(buffer, 1023, "%s|%s", username, password);
        return V6_httppost(host, port, "/api/login", NULL, buffer, outbuf, outsize);
}

int V6_signuprequest(
        const char *host, short port,
        const char *username, const char *password,
        char *outbuf, size_t outsize
) {
        char buffer[1024];
        _snprintf(buffer, 1023, "%s|%s", username, password);
        return V6_httppost(host, port, "/api/signup", NULL, buffer, outbuf, outsize);
}

int V6_roomrequest(
        const char *host, short port,
        char *outbuf, size_t outsize
) {
        return V6_httppost(host, port, "/api/rooms", NULL, NULL, outbuf, outsize);
}

int V6_messagerequest(
        const char *host, short port,
        const char *auth, const char *room, const char *msg
) {
        char buffer[8192];
        _snprintf(buffer, 8191, "%s|%s", msg, room);
    
        return V6_httppost(host, port, "/api/chat", auth, buffer, NULL, 0);
}

