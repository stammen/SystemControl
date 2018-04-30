// SystemControlDesktopExtension.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SystemControlDesktopExtension.h"
#include "SystemControl.h"

// Global Variables:
HINSTANCE hInst; // current instance

[Platform::MTAThread]
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
	hInst = hInstance; // Store instance handle in our global variable

	SystemControl sc;
	int result = sc.Run();
	return result;
}




