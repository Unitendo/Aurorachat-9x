#include <windows.h>
#include "v6.h"
#include "login.h"

int PASCAL WinMain(HANDLE hInst, HANDLE hPrevInst, LPSTR cmdLine, int cmdShow) {   
        if(V6_init()) {
                return 1;
        }
        return Login_main(hInst, hPrevInst, cmdLine, cmdShow);
}
