#ifndef APP_PROCS_H
#define APP_PROCS_H

#include "common.h"

#define STOP_THREAD_CALL	 0x10
#define STOP_THREAD_DONE	 0x11

#define README_PATH		 "." "\\" "Readme.txt"

/* Keyboard hook callback: processes keyboard events like Ctrl+A, Ctrl+Backspace, and caret manipulation via arrow keys */
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);

/* Subclass procedure for the main window's console(edit box) that restricts it to read and copy operations only */
LRESULT CALLBACK ConsoleSubclassProc(HWND hwndEdit, UINT msg, WPARAM wParam, LPARAM lParam);

/* Stop thread procedure created by WndProc_OnStopCommand(): cleans up encoder/archiver resources and sends STOP_THREAD_DONE with ID_STOP_BUTTON to the main window, 
 * and prints "run finished" to the console if lpParam equals STOP_THREAD_CALL; otherwise, prints "run stopped" */
DWORD WINAPI StopThreadProc(LPVOID lpParam);

/* Subclass procedure for the help dialog's content edit box (set via SetWindowSubclass from commctrl.h); manages caret and focus */
LRESULT CALLBACK HelpSubclassProc(HWND hwndEdit, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

/* Modeless help dialog procedure, initialized once, reused, and destroyed along with the main window */
INT_PTR CALLBACK HelpDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
