#include "rules.h"
#include "common.h"
#include "login.h"
#include "v6.h"
#include <stdio.h>

#define RULES_RULEBUFSIZE 4096
char Rules_rulebuf[RULES_RULEBUFSIZE] = {0};
const char RULES_WINCLASS[] = "AUC RULES";

long PASCAL Rules_WP(HWND hwnd, unsigned msg, UINT wparam, LONG lparam) {
        switch(msg) {
                case WM_DESTROY:
                        PostQuitMessage(0);
                break;

                case WM_CREATE: {
                        RECT clrect;
                        GetClientRect(hwnd, &clrect);

                        CreateWindow(
                                "Edit", Rules_rulebuf,
                                WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                                0, 0, clrect.right, clrect.bottom - 32,
                                hwnd, (HMENU) 1, NULL, NULL
                        );

                        CreateWindow(
                                "Button", "I have read and understand these rules.", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                0, clrect.bottom - 32, clrect.right, 32,
                                hwnd, (HMENU) 2, NULL, NULL
                        );
                } break;

                case WM_COMMAND: {
                        switch(HIWORD(wparam)) {
                                case BN_CLICKED:
                                        switch(LOWORD(wparam)) {
                                                case 2:
                                                        DestroyWindow(hwnd);
                                                break;
                                        }
                                break;
                        }
                } break;

                default:
                        return DefWindowProc(hwnd, msg, wparam, lparam);
        }

        return 0;
}


void Rules_init(WNDCLASS *wc, HANDLE hi) {
        wc->hInstance = hi;
        wc->lpszClassName = RULES_WINCLASS;
        wc->lpfnWndProc = Rules_WP;
        wc->hbrBackground = (HBRUSH) COLOR_WINDOW;
        wc->hCursor = LoadCursor(0, IDC_ARROW);
        wc->hIcon = LoadIcon(hi, "APPICON");

        RegisterClass(wc);
}

void Rules_main(HANDLE hInst) {
        HWND hw;
        MSG msg = {0};
        int i,j = 0;

        char rulebufbutnocrlf[RULES_RULEBUFSIZE] = {0};

        V6_rulesrequest(Login_targetip, Login_httpport, Login_tokenbuf, rulebufbutnocrlf, RULES_RULEBUFSIZE);

        for(i=0;rulebufbutnocrlf[i];i++) {
                char c;
                if(j >= RULES_RULEBUFSIZE) break;
                c = rulebufbutnocrlf[i];
                switch(c) {
                        case '\r': break;

                        case '\n':
                                Rules_rulebuf[j] = '\r';
                                j++;
                                Rules_rulebuf[j] = '\n';
                                j++;
                        break;
                    
                        default:
                                Rules_rulebuf[j] = c;
                                j++;
                }
        }

        hw = CreateWindow(
                RULES_WINCLASS, "AuroraChat Rules",
                WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_VISIBLE,
                CW_USEDEFAULT, CW_USEDEFAULT, 560, 400,
                NULL, NULL, hInst, NULL
        );

        while(GetMessage(&msg, NULL, 0, 0) > 0) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }
}

