#ifndef ARC_DIALOG_H
#define ARC_DIALOG_H

#include "common.h"

#include "arc_thrd.h"

#define ARC_DIALOG_RUN	 0x10

typedef struct _tagARCDIALOGINFO {
	HWND hInputPathEdit;
	HWND hOutputPathEdit;
	WNDPROC origInputPathEditProc;
} ARCDIALOGINFO;

typedef struct _tagARCDIALOGSTATE {
	UINT modeIndex;
	DWORD selStart;
	DWORD selEnd;
	BOOL bLButtonDown;
	char szPathBuf[MAX_PATH_BYTES];
	size_t pathBufLen;
	BOOL bCtrlKeyDown;
} ARCDIALOGSTATE;

/* Subclass procedure for the archiver dialog's edit boxes; automatically displays the completed output path when the input path is entered */
LRESULT CALLBACK ArcDlgSubclassProc(HWND hwndEdit, UINT msg, WPARAM wParam, LPARAM lParam);

/* Subclass procedure for archive create mode; mirrors changes from the input path edit box to the output path edit box at the same position, 
 * and on WM_SET_TEXT, sets output path to "<input path>.arc" */
static inline LRESULT ArcDlgCreateModeSubclassProc(HWND hwndEdit, UINT msg, WPARAM wParam, LPARAM lParam);

/* Subclass procedure for archive extract mode; extracts the directory path from the input path edit box's file path and sets it in the output path edit box */
static inline LRESULT ArcDlgExtractModeSubclassProc(HWND hwndEdit, UINT msg, WPARAM wParam, LPARAM lParam);

/* Inline function for ArcDlgExtractModeSubclassProc(); gets text from the input path edit box, normalizes path separators to '\' and extracts the directory path, 
 * stores its length in s_arcDlgState.pathBufLen, and sets the directory path in the output path edit box */
static inline LRESULT UpdateExtractOutputPathEdit(HWND hwndEdit, UINT msg, WPARAM wParam, LPARAM lParam);

/* Aarchiver dialog procedure: when the run button is pressed, sends ID_ARC_RUN_BUTTON to the main window (which then runs 'Archiving'/'Archive extraction') */
INT_PTR CALLBACK ArchiverDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
