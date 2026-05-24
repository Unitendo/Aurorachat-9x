#ifndef LOGIN_H
#define LOGIN_H

#include <windows.h>

#define LOGIN_TOKENBUFSIZE 4096

extern char Login_tokenbuf[LOGIN_TOKENBUFSIZE];
extern int Login_main(HANDLE hInst, HANDLE hPrevInst, LPSTR cmdLine, int cmdshow);

extern char Login_targetip[64];
extern short Login_httpport;
extern short Login_tcpport;

#endif

