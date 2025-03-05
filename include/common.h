#ifndef COMMON_H
#define COMMON_H

#include <windows.h>
#include <shlobj.h>
#include <combaseapi.h>
#include <shlwapi.h>
#include <commctrl.h> /* WARNING: Unicode-only library, caution when using its functions (e.g., SetWindowSubclass()) */

#include <stdio.h>
#include <strsafe.h>
#include <time.h>

#include "../resource/resource.h"
#include "app_procs.h"
#include "app_utils.h"

#define MAX_PATH_BYTES	 ((MAX_PATH << 1) - 1)
#define MAX_STRING		 1024

typedef struct tagAPPCONFIG {
	char encoderPath[MAX_PATH_BYTES];
	UINT archiverBufferSize;
	BOOL useSaveLastSettings;
	BOOL useLogRecord;
	BOOL useDebugLogging;
} APPCONFIG;

extern APPCONFIG g_appConfig;

typedef struct tagAPPSETTINGS {
	UINT modeIndex;
	char inputPath[MAX_PATH_BYTES];
	char outputPath[MAX_PATH_BYTES];
} APPSETTINGS;

extern APPSETTINGS g_appSettings;

typedef struct tagCONSOLEINFO {
	HWND hwnd;
	WNDPROC origProc;
} CONSOLEINFO;

extern CONSOLEINFO g_console;

extern HWND g_hwndMain;
extern HICON g_hIconApp;

#endif
