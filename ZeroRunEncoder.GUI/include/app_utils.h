#ifndef APP_UTILS_H
#define APP_UTILS_H

#include "common.h"

#define TEXT_MESSAGE_TIME_FORMAT	 "[%H:%M:%S] "
#define TEXT_MAX_MESSAGE	 (sizeof(TEXT_MESSAGE_TIME_FORMAT) + MAX_STRING)

#define PATH_TYPE_EXIST		 0x00
#define PATH_TYPE_FILE		 0x01
#define PATH_TYPE_DIR		 0x02

/* Creates a string using the provided format and variable argument list in an internal static string array and returns it */
LPCSTR VAStr(LPCSTR lpFormat, ...);

/* Custom function modified from SendMessage(EM_REPLACESEL): sends a message to hWnd to insert text from lpText; 
 * it processes '\r' and '\n' as in a console, splits lpText into lines using '\r', '\n', or '\0' as delimiters; if cls is TRUE, it clears all text in hWnd; 
 * if a line's length (excluding '\r' and '\n') is 0 then it is not output, and if the first non-newline character is '<' or '>' a timestamp is prepended. */
LRESULT SendInsertTextMessage(HWND hWnd, BOOL cls, LPCSTR lpText);

/* Custom function modified from SendMessage(EM_REPLACESEL): replaces the text from the start of the line containing caretPos up to caretPos with lpText; 
 * if canUndo is FALSE, the replacement cannot be undo */
inline LRESULT SendReplaceLineMessage(HWND hWnd, int caretPos, BOOL canUndo, LPCSTR lpText);

/* Custom function modified from SetWindowText(): after setting the window text, moves the caret to the end of the text */
inline BOOL SetWindowTextWithCaretEnd(HWND hWnd, LPCSTR lpString);

/* Appends text to hWnd from pFile by extracting the session defined by lpSession; the session's range is the text following a line in the form "[<lpSession>]" until the next "[" or '\0' */
inline BOOL AppendFileSessionTextToWindow(HWND hWnd, FILE* pFile, LPCSTR lpSession);

/* Makes a path in the form "<lpDirPath>\<lpFileName>" using the given parameters and returns the result of GetFileAttributes() */
inline DWORD MakePathAndGetFileAttr(LPCSTR lpDirPath, LPCSTR lpFileName);

/* Makes a path in the form "<lpPathBuf>\<lpFileName>", copies it to lpPathBuf, and returns the result of GetFileAttributes() */
inline DWORD MakePathBufAndGetFileAttr(LPSTR lpPathBuf, LPCSTR lpFileName, size_t cbPathBuf);

/* Opens a file selection dialog provided by Windows and sets the selected path to GetDlgItem(hwnd, controlId) using SetWindowTextWithCaretEnd() */
inline BOOL SelectFilePath(HWND hwnd, LPCSTR filter, int filterIndex, LPCSTR title, LPCSTR defExt, DWORD flags, LPCSTR defFileName, int controlId);

/* Opens a folder selection dialog provided by Windows and sets the selected path to GetDlgItem(hwnd, controlId) using SetWindowTextWithCaretEnd(); 
 * returns 0 on success, greater than 0 if an invalid path is selected, and less than 0 if the selection is canceled */
inline int SelectFolderPath(HWND hwnd, LPCSTR title, DWORD flags, int controlId);

/* Replaces all occurrences of chOld with chNew in lpStr */
inline BOOL StrReplaceAllChar(LPSTR lpStr, char chOld, char chNew);

/* Checks the validity of lpFileName; returns TRUE if valid, FALSE otherwise */
inline BOOL FileNameIsValid(LPCSTR lpFileName);

/* Checks if lpPath is an absolute path and validates it based on pathType (PATH_TYPE_EXIST, PATH_TYPE_FILE, PATH_TYPE_DIR); 
 * returns FALSE if lpPath is a relative path or does not match pathType (or does not exist) */
inline BOOL PathIsAbsoluteAndValid(int pathType, LPCSTR lpPath);

/* Copies pszPath without the file extension to lpPathBuf and returns lpPathBuf */
inline LPSTR GetFileNameWithoutExt(LPCSTR pszPath, LPSTR lpPathBuf, size_t cbPathBuf);

/* Returns TRUE if hWnd is both enabled and visible, otherwise returns FALSE */
inline BOOL IsWindowEnableAndVisible(HWND hWnd);

/* Custom function modified from ShowWindow(): Sets the control of commandId to nCmdShow and its associated menu to uMenuEnable, then returns the previous control state */
inline BOOL ShowWindowWithMenuItem(HWND hwnd, int commandId, int nCmdShow, UINT uMenuEnable);

/* Custom function modified from EnableWindow(): Sets the control of commandId to bBtnEnable and its associated menu to uMenuEnable, then returns the previous control state */
inline BOOL EnableWindowWithMenuItem(HWND hwnd, int commandId, BOOL bBtnEnable, UINT uMenuEnable);

#endif
