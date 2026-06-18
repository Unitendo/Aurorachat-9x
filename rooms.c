#include "rooms.h"
#include "common.h"
#include "v6.h"
#include "login.h"
#include "rules.h"
#include <stdio.h>
#include <string.h>
#include <winsock.h>

#define DEFAULTROOMNAME "general"
#define MSGBUFSIZE 16384
#define TIMERINTERVAL 500

char Rooms_roombuf[ROOMS_ROOMBUFSIZE] = {0};
char Rooms_roomname[ROOMS_ROOMBUFSIZE] = DEFAULTROOMNAME;
const char ROOMS_WINCLASS[] = "AUC ROOMS";
int Rooms_v6socket = -1;
char Rooms_messagebuffer[MSGBUFSIZE + 1] = {0};
size_t Rooms_lastmsgbuflen = 0;

WNDPROC Rooms_OldMsgEditBoxProc;

long PASCAL Rooms_MsgEditBoxProc(HWND hwnd, unsigned msg, UINT wparam, LONG lparam) {
        switch(msg) {
                case WM_KEYDOWN:
                        switch(wparam) {
                                char message[4096];
                            
                                case VK_RETURN:
                                        GetWindowText(hwnd, message, 4095);
                                        SetWindowText(hwnd, "");
                                        V6_messagerequest(Login_targetip, Login_httpport, Login_tokenbuf, Rooms_roomname, message);
                                break;
                        }
            
                default:
                        return CallWindowProc(Rooms_OldMsgEditBoxProc, hwnd, msg, wparam, lparam);
        }

        return 0;
}

long PASCAL Rooms_WP(HWND hwnd, unsigned msg, UINT wparam, LONG lparam) {
        switch(msg) {
                case WM_DESTROY:
                        PostQuitMessage(0);
                break;

                case WM_CREATE: {
                        HWND hmsginput;
                        RECT clrect;
                        GetClientRect(hwnd, &clrect);

                        CreateWindow(
                                "Edit", NULL,
                                WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                                0, 32, clrect.right, clrect.bottom - 64,
                                hwnd, (HMENU) 1, NULL, NULL
                        );

                        CreateWindow(
                                "Static", "#", WS_CHILD | WS_VISIBLE | SS_RIGHT,
                                0, 4, 8, 16,
                                hwnd, 0, NULL, NULL
                        );

                        CreateWindow(
                                "Edit", DEFAULTROOMNAME, WS_CHILD | WS_VISIBLE | WS_BORDER,
                                8, 0, clrect.right - (256 + 64 + 8), 24,
                                hwnd, (HMENU) 2, NULL, NULL
                        );

                        CreateWindow(
                                "Button", "Join Room", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                clrect.right - (256 + 64), 0, 128, 24,
                                hwnd, (HMENU) 3, NULL, NULL
                        );

                        CreateWindow(
                                "Button", "List Rooms", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                clrect.right - (128 + 64), 0, 128, 24,
                                hwnd, (HMENU) 4, NULL, NULL
                        );

                        CreateWindow(
                                "Button", "Clear", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                clrect.right - 64, 0, 64, 24,
                                hwnd, (HMENU) 7, NULL, NULL
                        );

                        hmsginput = CreateWindow(
                                "Edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
                                0, clrect.bottom - 24, clrect.right - 64, 24,
                                hwnd, (HMENU) 5, NULL, NULL
                        );

                        CreateWindow(
                                "Button", "Send", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                clrect.right - 64, clrect.bottom - 24, 64, 24,
                                hwnd, (HMENU) 6, NULL, NULL
                        );

                        Rooms_OldMsgEditBoxProc = (WNDPROC) SetWindowLong(hmsginput, GWL_WNDPROC, (LONG) Rooms_MsgEditBoxProc);

                        SetTimer(hwnd, 1337, TIMERINTERVAL, NULL);
                } break;

                case WM_COMMAND: {
                        switch(HIWORD(wparam)) {
                                case BN_CLICKED:
                                        switch(LOWORD(wparam)) {
                                                case 3: {
                                                        char message[128] = {0};
                                                        GetDlgItemText(hwnd, 2, Rooms_roomname, ROOMS_ROOMBUFSIZE - 1);
                                                        strncpy(Rooms_messagebuffer, "Auc9x: Joined #", MSGBUFSIZE);
                                                        strncat(Rooms_messagebuffer, Rooms_roomname, MSGBUFSIZE);
                                                        strncat(Rooms_messagebuffer, ".\r\n", MSGBUFSIZE);
                                                        _snprintf(message, 127, "history|2048|%s", Rooms_roomname);
                                                        send(Rooms_v6socket, message, strlen(message), 0);
                                                } break;

                                                case 4: {
                                                        char rbbackup[ROOMS_ROOMBUFSIZE];
                                                        char buffer[ROOMS_ROOMBUFSIZE];
                                                        char *token;
                                                        int count = 0;
                                                        int i;
                                                        
                                                        strncpy(rbbackup, Rooms_roombuf, ROOMS_ROOMBUFSIZE);
                                                        token = strtok(rbbackup, "|");
                                                        sscanf(token, "%d", &count);

                                                        buffer[0] = 0;
                                                        for(i=0;i<count;i++) {
                                                                token = strtok(NULL, "|");
                                                                if(token == NULL) break;
                                                                strncat(buffer, "#", ROOMS_ROOMBUFSIZE);
                                                                strncat(buffer, token, ROOMS_ROOMBUFSIZE);
                                                                if(i < count - 1)
                                                                        strncat(buffer, "\n", ROOMS_ROOMBUFSIZE);
                                                        }
                                                        
                                                        MessageBox(hwnd, buffer, "Room List", MB_ICONINFORMATION);
                                                } break;

                                                case 6: {
                                                        char message[4096];
                                                        GetDlgItemText(hwnd, 5, message, 4095);
                                                        SetDlgItemText(hwnd, 5, "");
                                                        V6_messagerequest(Login_targetip, Login_httpport, Login_tokenbuf, Rooms_roomname, message);
                                                } break;

                                                case 7: {
                                                        Rooms_messagebuffer[0] = 0;
                                                } break;
                                        }
                                break;
                        }
                } break;

                case WM_TIMER: {
                        switch(wparam) {
                                case 1337: {
                                        char buffer[4096] = {0};
                                        char buffer2[4096] = {0};
                                        char escapebuffer[4096] = {0};
                                        char *rawmsg;
                                        char *user, *message, *channel;
                                        u_long bufsize;
                                        HWND edit;
                                        DWORD startsel, endsel;

                                        ioctlsocket(Rooms_v6socket, FIONREAD, &bufsize);
                                        while(bufsize) {                    
                                                recv(Rooms_v6socket, buffer, 4095, 0);
                                                rawmsg = buffer;
                                                while(*rawmsg) {
                                                        int i;
                                                        int j = 0;
                                                        int len;
                                                    
                                                        user = strtok(rawmsg, "|");
                                                        if(user == NULL) goto next;
                                                        message = strtok(NULL, "|");
                                                        if(message == NULL) goto next;
                                                        channel = strtok(NULL, "|");
                                                        if(message == NULL) goto next;
                                                        len = strlen(message);

                                                        if(strncmp(channel, Rooms_roomname, 4096))
                                                                goto next;

                                                        for(i=0;i<len;i++) {
                                                                char c = message[i];
                                                                if(j >= 4095) break;
                                                                if(c == '\\') {
                                                                        char c;
                                                                        i++;
                                                                        c = message[i];
                                                                        switch(c) {
                                                                                case 'n':
                                                                                case 'N': {
                                                                                        escapebuffer[j] = '\r';
                                                                                        j++;
                                                                                        if(j >= 4095) break;
                                                                                        escapebuffer[j] = '\n';
                                                                                        j++;
                                                                                        if(j >= 4095) break;
                                                                                        escapebuffer[j] = ' ';
                                                                                        j++;
                                                                                        if(j >= 4095) break;
                                                                                        escapebuffer[j] = ' ';
                                                                                } break;
                                                                            
                                                                                default: escapebuffer[j] = c;
                                                                        }
                                                                } else escapebuffer[j] = c;
                                                                
                                                                j++;
                                                        }
                                                        escapebuffer[j] = 0;
                                                        
                                                        _snprintf(buffer2, 4095, "<%s> %s\r\n", user, escapebuffer); 
                                                        strncat(Rooms_messagebuffer, buffer2, MSGBUFSIZE);

                                                        next:

                                                        rawmsg = channel;
                                                        while(*rawmsg) rawmsg++;
                                                        rawmsg++;

                                                        for(;*rawmsg;rawmsg++) {
                                                                if((*rawmsg) == '\n') {
                                                                        rawmsg++;
                                                                        break;
                                                                }
                                                        }
                                                }

                                                ioctlsocket(Rooms_v6socket, FIONREAD, &bufsize);
                                        }

                                        if(strlen(Rooms_messagebuffer) >= ((MSGBUFSIZE * 3) / 4)) {
                                                int i;
                                                int m = (MSGBUFSIZE / 4);
                                                int n = strlen(Rooms_messagebuffer) - m;
                                                for(i=0;i<m;i++) {
                                                        Rooms_messagebuffer[i] = Rooms_messagebuffer[i + n];
                                                }
                                                Rooms_messagebuffer[m] = 0;
                                        }

                                        edit = GetDlgItem(hwnd, 1);
                                        if(strlen(Rooms_messagebuffer) != Rooms_lastmsgbuflen) {
                                                SendMessage(edit, EM_GETSEL, (WPARAM) &startsel, (LPARAM) &endsel);
                                                SetDlgItemText(hwnd, 1, Rooms_messagebuffer);
                                                SendMessage(edit, EM_SETSEL, startsel, endsel);
                                                SendMessage(edit, EM_SCROLLCARET, 0, 0);
                                                Rooms_lastmsgbuflen = strlen(Rooms_messagebuffer);
                                        }
                                } break;
                        }
                } break;

                default:
                        return DefWindowProc(hwnd, msg, wparam, lparam);
        }

        return 0;
}

void Rooms_init(WNDCLASS *wc, HANDLE hi) {
        wc->hInstance = hi;
        wc->lpszClassName = ROOMS_WINCLASS;
        wc->lpfnWndProc = Rooms_WP;
        wc->hbrBackground = (HBRUSH) COLOR_WINDOW;
        wc->hCursor = LoadCursor(0, IDC_ARROW);
        wc->hIcon = LoadIcon(hi, "APPICON");
        
        RegisterClass(wc);
}

int Rooms_main(HANDLE hInst) {
        HWND hw;
        MSG msg = {0};
        const char *message = "history|2048|general";

        Rules_main(hInst);

        V6_roomrequest(Login_targetip, Login_httpport, Rooms_roombuf, ROOMS_ROOMBUFSIZE);
        Rooms_v6socket = V6_connect(Login_targetip, Login_tcpport);
        if(Rooms_v6socket == -1) {
                V6_onconnecterr();
                return 1;
        }

        send(Rooms_v6socket, message, strlen(message), 0);

        hw = CreateWindow(
                ROOMS_WINCLASS, "AuroraChat 9x",
                WS_AUC,
                CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
                NULL, NULL, hInst, NULL
        );

        if(hw == NULL) return 1;

        while(GetMessage(&msg, NULL, 0, 0) > 0) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }

        return 0;
}

