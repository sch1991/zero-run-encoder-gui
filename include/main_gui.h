#ifndef MAIN_GUI_H
#define MAIN_GUI_H

#include "common.h"

#include "cfg_dialog.h"
#include "arc_dialog.h"
#include "log_thrd.h"
#include "enc_thrd.h"
#include "arc_thrd.h"

#define WINDOW_NAME		 "ZeroRunEncoder.GUI_ver.1.01"
#define WINDOW_WIDTH	 695
#define WINDOW_HEIGHT	 330

typedef struct _tagMAINMODULESTATE {
	HINSTANCE hInstance;
	HWND hStatic;
	UINT modeIndex;
	BOOL isRunActive;
	BOOL isStopActive;
	HANDLE hStopThread;
	HWND hwndHelp;
} MAINMODULESTATE;

/* Registers the window class, loads the main menu, creates the main window, and returns the main window's handle (HWND) */
HWND InitializeMainWindow(HINSTANCE hInstance);

/* Window callback function (WndProc) that processes all messages sent to the main window */ 
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/* Handles the WM_CREATE message for the main window */
static inline void WndProc_OnCreate(HWND hwnd, WPARAM wParam, LPARAM lParam);

/* Handles accelerator command messages for all registered accelerators */
static inline void WndProc_OnAcceleratorCommand(HWND hwnd, WPARAM wParam, LPARAM lParam);

/* Handles command messages from mode selection radio buttons */
static inline void WndProc_OnSelectModeCommand(HWND hwnd, WPARAM wParam, LPARAM lParam);

/* Handles the run button command: executes the encoder/archiver and enables the stop button  */
static inline void WndProc_OnRunCommand(HWND hwnd, WPARAM wParam, LPARAM lParam);

/* Handles the stop button command: when invoked by the stop button or after encoder/archiver completes, creates a stop thread for cleanup; 
 * upon receiving the stop thread's completion signal, cleans up its resources and re-enables the run button */
static inline void WndProc_OnStopCommand(HWND hwnd, WPARAM wParam, LPARAM lParam);

/* Updates button styles based on hControl and sets focus to it; if hControl is hStatic, resets all button styles to default */
static inline void UpdateButtonStylesAndFocus(HWND hControl);

/* Handles file association launches ("open with") by initializing the window for direct decoding/extracting of '*.0re'/'*.arc' files */
static inline BOOL WinMain_OnFileAssociationLaunch(LPSTR lpPath);

#endif
