#include "main_gui.h"

APPCONFIG g_appConfig;
APPSETTINGS g_appSettings;
CONSOLEINFO g_console;

HWND g_hwndMain;
HICON g_hIconApp;

MAINMODULESTATE s_mainState;

HWND InitializeMainWindow(HINSTANCE hInstance) {
	
	WNDCLASSEX wc; /* A properties struct of our window */
	
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc; /* This is where we will send messages to */
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = g_hIconApp;
	wc.hIconSm		 = g_hIconApp;
	
	if(!RegisterClassEx(&wc)) {
		EnqueueLogMessage(FALSE, "ERROR", "InitializeMainWindow", "Window registration failed.");
		MessageBox(NULL, "Failed To Register Window!", "Error", MB_ICONEXCLAMATION | MB_OK);
		return NULL;
	}
	
	HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAIN_MENU));
	if(hMenu == NULL) {
		EnqueueLogMessage(FALSE, "ERROR", "InitializeMainWindow", "Menu loading failed.");
		MessageBox(NULL, "Failed To Load Menu!", "Error", MB_ICONEXCLAMATION | MB_OK);
		return NULL;
	}
	
	HWND hwnd = CreateWindowEx(0, wc.lpszClassName, WINDOW_NAME, WS_VISIBLE | WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX, 
		CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, hMenu, hInstance, NULL);
		
	if(hwnd == NULL) {
		EnqueueLogMessage(FALSE, "ERROR", "InitializeMainWindow", "Window creation failed.");
		MessageBox(NULL, "Failed To Create Window!", "Error", MB_ICONEXCLAMATION | MB_OK);
		return NULL;
	}
	
	return hwnd;
}



LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	switch(msg) {
		case WM_CREATE: {
			WndProc_OnCreate(hwnd, wParam, lParam);
			break;
		}
		
		case WM_CTLCOLORSTATIC: {
			if((HWND)lParam == g_console.hwnd) {
				HDC hdcStatic = (HDC)wParam;
				SetBkColor(hdcStatic, RGB(255, 255, 255));
				return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
			}
			break;
		}
		
		case WM_LBUTTONDOWN: {
			SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)s_mainState.hStatic, TRUE);
			break;
		}
		
		case WM_NEXTDLGCTL: {
			if(LOWORD(lParam) == TRUE) {
				UpdateButtonStylesAndFocus((HWND)wParam);
			} else {
				UpdateButtonStylesAndFocus(GetNextDlgTabItem(hwnd, GetFocus(), (WINBOOL)wParam));
			}
			return 0;
		}
		
		case WM_DROPFILES: {
			HDROP hDrop = (HDROP)wParam;
			char szFilePath[MAX_PATH_BYTES];
			POINT pt;
			
			DragQueryPoint(hDrop, &pt);
			HWND hTarget = ChildWindowFromPoint(hwnd, pt);
			
			if(hTarget == GetDlgItem(hwnd, IDC_EDIT_INPUT_PATH) || hTarget == GetDlgItem(hwnd, IDC_EDIT_OUTPUT_PATH)) {
				DragQueryFile(hDrop, 0, szFilePath, sizeof(szFilePath));
				SetWindowTextWithCaretEnd(hTarget, szFilePath);
			}
			
			DragFinish(hDrop);
			break;
		}
		
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				/* Accelerator commands, add ID here when accelerator is added */
				case ID_ACC_SHOW_HELP: 
				case ID_ACC_NEXT_MODE: 
				case ID_ACC_SAVE_SETTINGS: 
				case ID_ACC_RESET_SETTTINGS: 
				case ID_ACC_CLEAR_CONSOLE: 
				case ID_ACC_SHOW_ARCHIVER: 
				case ID_ACC_LOG_RECORD: 
				case ID_ACC_DEBUG_LOGGING: 
				case ID_ACC_OPEN_LOGS: {
					WndProc_OnAcceleratorCommand(hwnd, wParam, lParam);
					break;
				}
				
				/* Mode selection commands */
				case ID_ENCODING_MODE: 
				case ID_DECODING_MODE: 
				case ID_DETAILS_MODE: {
					WndProc_OnSelectModeCommand(hwnd, wParam, lParam);
					break;
				}
				
				case ID_SET_INPUT_PATH: {
					switch(g_encModeInfo[s_mainState.modeIndex].nId) {
						case ID_ENCODING_MODE: {
							if(SelectFilePath(hwnd, "All Files(*.*)\0*.*\0" "0re Files(*.0re)\0*.0re\0", 1, "Select a File", NULL, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST, NULL, IDC_EDIT_INPUT_PATH) == FALSE) {
								if(SelectFolderPath(hwnd, "Select a Folder", BIF_USENEWUI, IDC_EDIT_INPUT_PATH) > 0) {
									MessageBox(hwnd, "Invalid Folder Path!", "Warning", MB_ICONEXCLAMATION | MB_OK);
								}
							}
							break;
						}
						
						case ID_DECODING_MODE: 
						case ID_DETAILS_MODE: {
							SelectFilePath(hwnd, "All Files(*.*)\0*.*\0" "0re Files(*.0re)\0*.0re\0", 2, "Select 0re File", NULL, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST, NULL, IDC_EDIT_INPUT_PATH);
							break;
						}
					}
					SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwnd, ID_SET_INPUT_PATH), TRUE);
					break;
				}
				
				case ID_SET_OUTPUT_PATH: {
					switch(g_encModeInfo[s_mainState.modeIndex].nId) {
						case ID_ENCODING_MODE: {
							if(SelectFilePath(hwnd, "All Files(*.*)\0*.*\0" "0re Files(*.0re)\0*.0re\0", 2, "Save 0re File As", "0re", OFN_OVERWRITEPROMPT, "*.0re", IDC_EDIT_OUTPUT_PATH) == FALSE) {
								if(SelectFolderPath(hwnd, "Select a Folder", BIF_USENEWUI, IDC_EDIT_OUTPUT_PATH) > 0) {
									MessageBox(hwnd, "Invalid Folder Path!", "Warning", MB_ICONEXCLAMATION | MB_OK);
								}
							}
							break;
						}
						
						case ID_DECODING_MODE: {
							if(SelectFolderPath(hwnd, "Select a Folder", BIF_USENEWUI, IDC_EDIT_OUTPUT_PATH) > 0) {
								MessageBox(hwnd, "Invalid Folder Path!", "Warning", MB_ICONEXCLAMATION | MB_OK);
							}
							break;
						}
					}
					SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwnd, ID_SET_OUTPUT_PATH), TRUE);
					break;
				}
				
				case IDOK: {
					if(IsWindowEnableAndVisible(GetDlgItem(hwnd, ID_RUN_BUTTON)) == TRUE) {
						WndProc_OnRunCommand(hwnd, ID_RUN_BUTTON, 0);
						
					} else if(IsWindowEnableAndVisible(GetDlgItem(hwnd, ID_STOP_BUTTON)) == TRUE) {
						WndProc_OnStopCommand(hwnd, ID_STOP_BUTTON, 0);
					}
					break;
				}
				
				case ID_RUN_BUTTON: {
					WndProc_OnRunCommand(hwnd, wParam, lParam);
					SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)s_mainState.hStatic, TRUE);
					break;
				}
				
				case ID_STOP_BUTTON: {
					WndProc_OnStopCommand(hwnd, wParam, lParam);
					SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)s_mainState.hStatic, TRUE);
					break;
				}
				
				case ID_CONFIG_BUTTON: {
					if(s_mainState.isRunActive == TRUE || s_mainState.isStopActive == TRUE) break;
					
					INT_PTR dlgResult = DialogBox(s_mainState.hInstance, MAKEINTRESOURCE(IDD_CONFIG_DIALOG), hwnd, ConfigDialogProc);
					if(LOWORD(dlgResult) == ID_SAVE_BUTTON) {
						if(HIWORD(dlgResult) == CFG_SAVED_APP_ONLY) {
							SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> App config only saved.");
						} else if(HIWORD(dlgResult) == CFG_SAVED_APP_AND_ENC) {
							SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> App & Encoder config saved.");
						}
						
						DWORD dwFileAttr = MakePathAndGetFileAttr(g_appConfig.encoderPath, ENC_APP_NAME);
						if(dwFileAttr == INVALID_FILE_ATTRIBUTES || (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY)) {
							if(IsWindowEnabled(GetDlgItem(hwnd, ID_RUN_BUTTON)) == TRUE) {
								SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\r\n> \'Run\' disabled: \'%s\' not found in encoder path.\r\n" "(Please check your config.)", ENC_APP_NAME));
								EnableWindowWithMenuItem(hwnd, ID_RUN_BUTTON, FALSE, MF_GRAYED);
							}
						} else EnableWindowWithMenuItem(hwnd, ID_RUN_BUTTON, TRUE, MF_ENABLED);
					}
					SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwnd, ID_CONFIG_BUTTON), TRUE);
					break;
				}
				
				case IDCANCEL: {
					SendMessage(hwnd, WM_COMMAND, (WPARAM)ID_CLOSE_BUTTON, 0);
					break;
				}
				
				case ID_CLOSE_BUTTON: {
					SendMessage(hwnd, WM_CLOSE, 0, 0);
					SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)s_mainState.hStatic, TRUE);
					break;
				}
				
				default:
					return DefWindowProc(hwnd, msg, wParam, lParam);
			}
			
			break;
		}
		
		case WM_CLOSE: {
			if(s_mainState.isRunActive == TRUE) {
				SendMessage(hwnd, WM_COMMAND, (WPARAM)ID_STOP_BUTTON, 0);
				if(s_mainState.isStopActive == FALSE) break;
			}
			
			if(MessageBox(hwnd, "Do you want to close the application?", "Confirmation", MB_ICONQUESTION | MB_OKCANCEL) == IDCANCEL) break;
			if(g_appConfig.useSaveLastSettings == TRUE) {
				SendMessage(hwnd, WM_COMMAND, (WPARAM)ID_ACC_SAVE_SETTINGS, 0);
			}
			DestroyWindow(hwnd);
			break;
		}
		
		case WM_DESTROY: {
			DeleteObject((HFONT)SendMessage(g_console.hwnd, WM_GETFONT, 0, 0));
			if(s_mainState.hwndHelp != NULL) DestroyWindow(s_mainState.hwndHelp);
			PostQuitMessage(0);
			EnqueueLogMessage(FALSE, "INFO", "WndProc", "Window destroy message posted.");
			break;
		}
		
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	
	return 0;
}



static inline void WndProc_OnCreate(HWND hwnd, WPARAM wParam, LPARAM lParam) {
	
	int x, y;
	HWND hControl;
	
	s_mainState.hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
	
	CreateWindow("BUTTON", "Mode", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 8, y=8, 296, 59, hwnd, (HMENU)IDC_STATIC, s_mainState.hInstance, NULL);
	CreateWindow("BUTTON", "Encoding", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_RADIOBUTTON | WS_GROUP, x=23, y+=23, 85, 25, hwnd, (HMENU)ID_ENCODING_MODE, s_mainState.hInstance, NULL);
	CreateWindow("BUTTON", "Decoding", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_RADIOBUTTON, x+=91, y, 85, 25, hwnd, (HMENU)ID_DECODING_MODE, s_mainState.hInstance, NULL);
	CreateWindow("BUTTON", "Details", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_RADIOBUTTON, x+=91, y, 70, 25, hwnd, (HMENU)ID_DETAILS_MODE, s_mainState.hInstance, NULL);
	
	CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 10, y+=45, 280, 23, hwnd, (HMENU)IDC_LABEL_INPUT_PATH, s_mainState.hInstance, NULL);
	hControl = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, x=8, y+=23, 265, 25, hwnd, (HMENU)IDC_EDIT_INPUT_PATH, s_mainState.hInstance, NULL);
	SetWindowTextWithCaretEnd(hControl, g_appSettings.inputPath);
	SendMessage(hControl, EM_LIMITTEXT, (WPARAM)(MAX_PATH-1), 0);
	
	CreateWindow("BUTTON", "...", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | WS_BORDER, x+268, y, 28, 23, hwnd, (HMENU)ID_SET_INPUT_PATH, s_mainState.hInstance, NULL);
	
	CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 10, y+=38, 280, 23, hwnd, (HMENU)IDC_LABEL_OUTPUT_PATH, s_mainState.hInstance, NULL);
	hControl = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, x=8, y+=23, 265, 25, hwnd, (HMENU)IDC_EDIT_OUTPUT_PATH, s_mainState.hInstance, NULL);
	SetWindowTextWithCaretEnd(hControl, g_appSettings.outputPath);
	SendMessage(hControl, EM_LIMITTEXT, (WPARAM)(MAX_PATH-1), 0);
	
	CreateWindow("BUTTON", "...", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | WS_BORDER, x+268, y, 28, 23, hwnd, (HMENU)ID_SET_OUTPUT_PATH, s_mainState.hInstance, NULL);
	
	SendMessage(hwnd, WM_COMMAND, (WPARAM)g_encModeInfo[g_appSettings.modeIndex].nId, 0);
	
	CreateWindow("BUTTON", "Run", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON | WS_BORDER, x=52, y+=43, 200, 25, hwnd, (HMENU)ID_RUN_BUTTON, s_mainState.hInstance, NULL);
	CreateWindow("BUTTON", "Stop", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON | WS_BORDER, x, y, 200, 25, hwnd, (HMENU)ID_STOP_BUTTON, s_mainState.hInstance, NULL);
	CreateWindow("BUTTON", "Config", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | WS_BORDER, x, y+=29, 98, 25, hwnd, (HMENU)ID_CONFIG_BUTTON, s_mainState.hInstance, NULL);
	CreateWindow("BUTTON", "Close", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | WS_BORDER, x+=102, y, 98, 25, hwnd, (HMENU)ID_CLOSE_BUTTON, s_mainState.hInstance, NULL);
	
	g_console.hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_READONLY | ES_MULTILINE | WS_VSCROLL | WS_BORDER, 
		WINDOW_WIDTH-385, 3, 375, WINDOW_HEIGHT-56, hwnd, (HMENU)IDC_EDIT_CONSOLE_OUTPUT, s_mainState.hInstance, NULL);
	
	g_console.origProc = (WNDPROC)SetWindowLongPtr(g_console.hwnd, GWLP_WNDPROC, (LONG_PTR)ConsoleSubclassProc);
	
	HFONT hFont = CreateFont(0, 6, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH, TEXT("Consolas"));
	SendMessage(g_console.hwnd, WM_SETFONT, (WPARAM)hFont, TRUE);
	
	DragAcceptFiles(hwnd, TRUE);
	
	s_mainState.hStatic = GetDlgItem(hwnd, IDC_STATIC);
	PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)s_mainState.hStatic, TRUE);
	
	SendInsertTextMessage(g_console.hwnd, TRUE, "<< GUI App initialization completed >>");
	
	DWORD dwFileAttr = MakePathAndGetFileAttr(g_appConfig.encoderPath, ENC_APP_NAME);
	if(dwFileAttr == INVALID_FILE_ATTRIBUTES || (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY)) {
		SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\r\n> \'Run\' disabled: \'%s\' not found in encoder path.\r\n" "(Please check your config.)", ENC_APP_NAME));
		EnableWindowWithMenuItem(hwnd, ID_RUN_BUTTON, FALSE, MF_GRAYED);
	}
	
	if(g_appConfig.useLogRecord == TRUE) CheckMenuItem(GetMenu(hwnd), ID_ACC_LOG_RECORD, MF_BYCOMMAND | MF_CHECKED);
	if(g_appConfig.useDebugLogging == TRUE) CheckMenuItem(GetMenu(hwnd), ID_ACC_LOG_RECORD, MF_BYCOMMAND | MF_CHECKED);
	
	EnqueueLogMessage(FALSE, "INFO", "WndProc_OnCreate", "Window creation completed.");
	
	return;
}



static inline void WndProc_OnAcceleratorCommand(HWND hwnd, WPARAM wParam, LPARAM lParam) {
	
	switch(LOWORD(wParam)) {
		case ID_ACC_SHOW_HELP: {
			if(s_mainState.hwndHelp == NULL) {
				s_mainState.hwndHelp = CreateDialog(s_mainState.hInstance, MAKEINTRESOURCE(IDD_HELP_DIALOG), hwnd, HelpDialogProc);
				if(s_mainState.hwndHelp == NULL) {
					MessageBox(g_hwndMain, VAStr("Help Dialog Creation Failed!\n\n" "error_code: %u", GetLastError()), "Error", MB_ICONEXCLAMATION | MB_OK);
					break;
				}
			}
			ShowWindow(s_mainState.hwndHelp, (IsWindowVisible(s_mainState.hwndHelp) ? SW_HIDE : SW_SHOW));
			break;
		}
		
		case ID_ACC_NEXT_MODE: {
			s_mainState.modeIndex = (s_mainState.modeIndex + 1) % (sizeof(g_encModeInfo) / sizeof(ENCODERMODEINFO));
			SendMessage(hwnd, WM_COMMAND, (WPARAM)g_encModeInfo[s_mainState.modeIndex].nId, 0);
			break;
		}
		
		case ID_ACC_SAVE_SETTINGS: {
			if(s_mainState.isRunActive == TRUE || s_mainState.isStopActive == TRUE) break;
			
			g_appSettings.modeIndex = s_mainState.modeIndex;
			GetWindowText(GetDlgItem(hwnd, IDC_EDIT_INPUT_PATH), g_appSettings.inputPath, sizeof(g_appSettings.inputPath));
			GetWindowText(GetDlgItem(hwnd, IDC_EDIT_OUTPUT_PATH), g_appSettings.outputPath, sizeof(g_appSettings.outputPath));
			
			SaveAppConfigFile(FALSE);
			SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Settings saved.");
			break;
		}
		
		case ID_ACC_RESET_SETTTINGS: {
			SendMessage(hwnd, WM_COMMAND, (WPARAM)g_encModeInfo[0].nId, 0);
			SetWindowText(GetDlgItem(hwnd, IDC_EDIT_INPUT_PATH), "");
			SetWindowText(GetDlgItem(hwnd, IDC_EDIT_OUTPUT_PATH), "");
			break;
		}
		
		case ID_ACC_CLEAR_CONSOLE: {
			SendInsertTextMessage(g_console.hwnd, TRUE, "");
			break;
		}
		
		case ID_ACC_SHOW_ARCHIVER: {
			if(s_mainState.isRunActive == TRUE || s_mainState.isStopActive == TRUE) break;
			
			INT_PTR dlgResult = DialogBoxParam(s_mainState.hInstance, MAKEINTRESOURCE(IDD_ARCHIVER_DIALOG), hwnd, ArchiverDialogProc, lParam); /* (lParam != 0): file association launch  */
			if(LOWORD(dlgResult) == ID_ARC_RUN_BUTTON) {
				PostMessage(hwnd, WM_COMMAND, (WPARAM)MAKELONG(ID_RUN_BUTTON, ARC_DIALOG_RUN), 0);
			}
			break;
		}
				
		case ID_ACC_LOG_RECORD: {
			ChangeAppConfigFileValue(CFG_USE_LOG_RECORD);
			if(g_appConfig.useLogRecord == TRUE) {
				CheckMenuItem(GetMenu(hwnd), ID_ACC_LOG_RECORD, MF_BYCOMMAND | MF_CHECKED);
				SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Log recording started.\r\n" "(App config file updated.)");
			} else {
				CheckMenuItem(GetMenu(hwnd), ID_ACC_LOG_RECORD, MF_BYCOMMAND | MF_UNCHECKED);
				SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Log recording stopped.\r\n" "(App config file updated.)");
			}
			break;
		}
		
		case ID_ACC_DEBUG_LOGGING: {
			ChangeAppConfigFileValue(CFG_USE_DEBUG_LOGGING);
			if(g_appConfig.useDebugLogging == TRUE) {
				CheckMenuItem(GetMenu(hwnd), ID_ACC_DEBUG_LOGGING, MF_BYCOMMAND | MF_CHECKED);
				SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Debug logging started.\r\n" "(App config file updated.)");
			} else {
				CheckMenuItem(GetMenu(hwnd), ID_ACC_DEBUG_LOGGING, MF_BYCOMMAND | MF_UNCHECKED);
				SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Debug logging stopped.\r\n" "(App config file updated.)");
			}
			break;
		}
		
		case ID_ACC_OPEN_LOGS: {
			INT_PTR shlResult = (INT_PTR)ShellExecute(hwnd, "open", LOGS_DIR_PATH, NULL, NULL, SW_SHOW);
			if(shlResult <= 32) {
				MessageBox(hwnd, VAStr("Logs Folder Opening Failed!\n\n" "error_code: %d", shlResult), "Error", MB_ICONEXCLAMATION | MB_OK);
			}
			break;
		}
		
		/* Add case here when accelerator is added */
	}
	
	return;
}



static inline void WndProc_OnSelectModeCommand(HWND hwnd, WPARAM wParam, LPARAM lParam) {

	switch(LOWORD(wParam)) {
		case ID_ENCODING_MODE: {
			CheckRadioButton(hwnd, ID_ENCODING_MODE, ID_DETAILS_MODE, ID_ENCODING_MODE);
			s_mainState.modeIndex = 0;
			
			SetWindowText(GetDlgItem(hwnd, IDC_LABEL_INPUT_PATH), "Input File or Folder Path : ");
			SetWindowText(GetDlgItem(hwnd, IDC_LABEL_OUTPUT_PATH), "Output File or Folder Path : ");
			
			ShowWindow(GetDlgItem(hwnd, IDC_EDIT_OUTPUT_PATH), SW_SHOW);
			ShowWindowWithMenuItem(hwnd, ID_SET_OUTPUT_PATH, SW_SHOW, MF_ENABLED);
			break;
		}
		
		case ID_DECODING_MODE: {
			CheckRadioButton(hwnd, ID_ENCODING_MODE, ID_DETAILS_MODE, ID_DECODING_MODE);
			s_mainState.modeIndex = 1;
			
			SetWindowText(GetDlgItem(hwnd, IDC_LABEL_INPUT_PATH), "0re File Path : ");
			SetWindowText(GetDlgItem(hwnd, IDC_LABEL_OUTPUT_PATH), "Output Folder Path : ");
			
			ShowWindow(GetDlgItem(hwnd, IDC_EDIT_OUTPUT_PATH), SW_SHOW);
			ShowWindowWithMenuItem(hwnd, ID_SET_OUTPUT_PATH, SW_SHOW, MF_ENABLED);
			break;
		}
		
		case ID_DETAILS_MODE: {
			CheckRadioButton(hwnd, ID_ENCODING_MODE, ID_DETAILS_MODE, ID_DETAILS_MODE);
			s_mainState.modeIndex = 2;
			
			SetWindowText(GetDlgItem(hwnd, IDC_LABEL_INPUT_PATH), "0re File Path : ");
			SetWindowText(GetDlgItem(hwnd, IDC_LABEL_OUTPUT_PATH), "");
			
			ShowWindow(GetDlgItem(hwnd, IDC_EDIT_OUTPUT_PATH), SW_HIDE);
			ShowWindowWithMenuItem(hwnd, ID_SET_OUTPUT_PATH, SW_HIDE, MF_GRAYED);
			break;
		}
	}
	
	return;
}



static inline void WndProc_OnRunCommand(HWND hwnd, WPARAM wParam, LPARAM lParam) {

	if(s_mainState.isRunActive == TRUE || s_mainState.isStopActive == TRUE) return;

	EnqueueLogMessage(FALSE, "INFO", "WndProc_OnRunCommand", "Run started.");
	BOOL bPrevRunBtnState = !EnableWindowWithMenuItem(hwnd, ID_RUN_BUTTON, FALSE, MF_GRAYED);
	
	WORD nRunCode = HIWORD(wParam);
	
	s_mainState.isRunActive = TRUE;
	EnableWindow(g_console.hwnd, FALSE);
	
	if(nRunCode == 0) {
		SendInsertTextMessage(g_console.hwnd, TRUE, "<< Run activated: Encoder >>");
		
		g_appSettings.modeIndex = s_mainState.modeIndex;
		GetWindowText(GetDlgItem(hwnd, IDC_EDIT_INPUT_PATH), g_appSettings.inputPath, sizeof(g_appSettings.inputPath));
		GetWindowText(GetDlgItem(hwnd, IDC_EDIT_OUTPUT_PATH), g_appSettings.outputPath, sizeof(g_appSettings.outputPath));
		
		if(g_encModeInfo[g_appSettings.modeIndex].reqFlag & ENC_MODE_NEED_INPUT) {
			StrReplaceAllChar(g_appSettings.inputPath, '/', '\\');
		}
		if(g_encModeInfo[g_appSettings.modeIndex].reqFlag & ENC_MODE_NEED_OUTPUT) {
			StrReplaceAllChar(g_appSettings.outputPath, '/', '\\');
		}
		
		if(CreateEncoderThread(&g_encModeInfo[g_appSettings.modeIndex], g_appSettings.inputPath, g_appSettings.outputPath) == FALSE) {
			s_mainState.isRunActive = FALSE;
			EnableWindow(g_console.hwnd, TRUE);
			EnableWindowWithMenuItem(hwnd, ID_RUN_BUTTON, bPrevRunBtnState, (bPrevRunBtnState ? MF_ENABLED : MF_GRAYED));
			return;
		}
		
	} else if(nRunCode == ARC_DIALOG_RUN) {
		SendInsertTextMessage(g_console.hwnd, TRUE, "<< Run activated: Archiver >>");
		
		if(CreateArchiverThread(g_arcModeInfo[g_appSettings.modeIndex].operId, g_appSettings.inputPath) == FALSE) {
			s_mainState.isRunActive = FALSE;
			EnableWindow(g_console.hwnd, TRUE);
			EnableWindowWithMenuItem(hwnd, ID_RUN_BUTTON, bPrevRunBtnState, (bPrevRunBtnState ? MF_ENABLED : MF_GRAYED));
			return;
		}
	}
	
	ShowWindowWithMenuItem(hwnd, ID_RUN_BUTTON, SW_HIDE, MF_GRAYED);
	EnableWindow(GetDlgItem(hwnd, ID_RUN_BUTTON), bPrevRunBtnState);
	
	ShowWindowWithMenuItem(hwnd, ID_STOP_BUTTON, SW_SHOW, MF_ENABLED);
	EnableWindowWithMenuItem(hwnd, ID_CONFIG_BUTTON, FALSE, MF_GRAYED);
	
	EnableMenuItem(GetMenu(hwnd), ID_ACC_SAVE_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(GetMenu(hwnd), ID_ACC_SHOW_ARCHIVER, MF_BYCOMMAND | MF_GRAYED);
	
	return;
}



static inline void WndProc_OnStopCommand(HWND hwnd, WPARAM wParam, LPARAM lParam) {
	
	WORD nStopCode = HIWORD(wParam);
	
	if(nStopCode == 0) {
		if(MessageBox(hwnd, "Do you want to stop the run?", "Confirmation", MB_ICONQUESTION | MB_OKCANCEL) == IDCANCEL) return;
	}
	if(s_mainState.isRunActive == FALSE) return;
	
	if(s_mainState.hStopThread == NULL && s_mainState.isStopActive == FALSE) {
		EnqueueLogMessage(FALSE, "INFO", "WndProc_OnStopCommand", "Stop executed.");
		EnableWindowWithMenuItem(hwnd, ID_STOP_BUTTON, FALSE, MF_GRAYED);
		
		s_mainState.hStopThread = CreateThread(NULL, 0, StopThreadProc, (LPVOID)(UINT_PTR)nStopCode, 0, NULL);
		if(s_mainState.hStopThread == NULL) {
			EnqueueLogMessage(FALSE, "ERROR", "WndProc_OnStopCommand", VAStr("Stop thread creation failed. error_code=%04u", GetLastError()));
			SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Stop failed: Thread creation failed.");
			
		} else s_mainState.isStopActive = TRUE;
		
	} else if(nStopCode == STOP_THREAD_DONE && s_mainState.isStopActive == TRUE) {
		if(s_mainState.hStopThread == NULL) return;
		
		CloseHandle(s_mainState.hStopThread);
		s_mainState.hStopThread = NULL;
		
		s_mainState.isStopActive = FALSE;
		
		ShowWindowWithMenuItem(hwnd, ID_STOP_BUTTON, SW_HIDE, MF_GRAYED);
		EnableWindow(GetDlgItem(hwnd, ID_STOP_BUTTON), TRUE);
		
		s_mainState.isRunActive = FALSE;
		EnableWindow(g_console.hwnd, TRUE);
		
		ShowWindowWithMenuItem(hwnd, ID_RUN_BUTTON, SW_SHOW, (IsWindowEnabled(GetDlgItem(hwnd, ID_RUN_BUTTON)) ? MF_ENABLED : MF_GRAYED));
		EnableWindowWithMenuItem(hwnd, ID_CONFIG_BUTTON, TRUE, MF_ENABLED);
		
		EnableMenuItem(GetMenu(hwnd), ID_ACC_SAVE_SETTINGS, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(GetMenu(hwnd), ID_ACC_SHOW_ARCHIVER, MF_BYCOMMAND | MF_ENABLED);
	}
	
	return;
}



static inline void UpdateButtonStylesAndFocus(HWND hControl) {
	
	SendMessage(GetDlgItem(g_hwndMain, ID_SET_INPUT_PATH), BM_SETSTYLE, BS_PUSHBUTTON, FALSE);
	SendMessage(GetDlgItem(g_hwndMain, ID_SET_OUTPUT_PATH), BM_SETSTYLE, BS_PUSHBUTTON, FALSE);
	SendMessage(GetDlgItem(g_hwndMain, ID_RUN_BUTTON), BM_SETSTYLE, BS_PUSHBUTTON, FALSE);
	SendMessage(GetDlgItem(g_hwndMain, ID_STOP_BUTTON), BM_SETSTYLE, BS_PUSHBUTTON, FALSE);
	SendMessage(GetDlgItem(g_hwndMain, ID_CONFIG_BUTTON), BM_SETSTYLE, BS_PUSHBUTTON, FALSE);
	SendMessage(GetDlgItem(g_hwndMain, ID_CLOSE_BUTTON), BM_SETSTYLE, BS_PUSHBUTTON, FALSE);
	/* Add code here when a button that requires reset is added */
	
	if(hControl != s_mainState.hStatic) {
		LONG_PTR style = GetWindowLongPtr(hControl, GWL_STYLE);
		if((style & BS_TYPEMASK) == BS_PUSHBUTTON) {
			SendMessage(hControl, BM_SETSTYLE, BS_DEFPUSHBUTTON, FALSE);
		} else SendMessage(GetDlgItem(g_hwndMain, ID_RUN_BUTTON), BM_SETSTYLE, BS_DEFPUSHBUTTON, FALSE);
		
	} else SendMessage(GetDlgItem(g_hwndMain, ID_RUN_BUTTON), BM_SETSTYLE, BS_DEFPUSHBUTTON, FALSE);
	
	InvalidateRect(g_hwndMain, NULL, FALSE);
	
	SetFocus(hControl);
	
	return;
}



static inline BOOL WinMain_OnFileAssociationLaunch(LPSTR lpPath) {
	
	HANDLE hFile = CreateFile(lpPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE) {
		EnqueueLogMessage(FALSE, "ERROR", "WinMain_OnFileAssociationLaunch", VAStr("Failed to open associated file. error_code=%04u", GetLastError()));
		MessageBox(g_hwndMain, "Associated File Opening Failed!\n", "Error", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	
	DWORD dwFileHead;
	ReadFile(hFile, &dwFileHead, sizeof(DWORD), NULL, NULL);
	
	if(dwFileHead == ARCHIVE_HEAD) {
		SendMessage(g_hwndMain, WM_COMMAND, (WPARAM)ID_ACC_SHOW_ARCHIVER, (LPARAM)lpPath);
		
	} else {
		SendMessage(g_hwndMain, WM_COMMAND, (WPARAM)ID_DECODING_MODE, 0);
		SetWindowTextWithCaretEnd(GetDlgItem(g_hwndMain, IDC_EDIT_INPUT_PATH), lpPath);
		PathRemoveFileSpec(lpPath);
		SetWindowTextWithCaretEnd(GetDlgItem(g_hwndMain, IDC_EDIT_OUTPUT_PATH), lpPath);
	}
	
	CloseHandle(hFile);
	
	return TRUE;
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	char szPath[MAX_PATH_BYTES];
	
	if(GetModuleFileName(NULL, szPath, sizeof(szPath)) != 0) {
		PathRemoveFileSpec(szPath);
		SetCurrentDirectory(szPath);
	} else {
		MessageBox(NULL, VAStr("Failed To Retrieve Application Path!\n\n" "error_code: %u", GetLastError()), "Error", MB_ICONERROR | MB_OK);
		return 1;
	}
	
	if(InitializeLoggerThread() == FALSE) {
		MessageBox(NULL, "Logger Thread Initialization Failed!", "Error", MB_ICONERROR | MB_OK);
		return 1;
		
	} else LoadAppConfigFile();
	
	g_hIconApp = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
	if(g_hIconApp == NULL) {
		EnqueueLogMessage(FALSE, "WARNING", "WinMain", "Failed to load application icon.");
		MessageBox(NULL, "Application Icon Loading Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
	}
	
	g_hwndMain = InitializeMainWindow(hInstance);
	if(g_hwndMain == NULL) {
		EnqueueLogMessage(FALSE, "CRITICAL", "WinMain", "Window initialization failed, program terminating.");
		MessageBox(NULL, "Window Initialization Failed!", "Error", MB_ICONERROR | MB_OK);
		DestroyLoggerThread();
		return 1;
	}
	
	HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardHookProc, NULL, GetWindowThreadProcessId(g_hwndMain, NULL));
	if(hHook == NULL) {
		EnqueueLogMessage(FALSE, "WARNING", "WinMain", "Failed to install keyboard hook.");
		MessageBox(g_hwndMain, "Keyboard Hook Installation Failed!\n" "(Keyboard hook functions are unavailable.)", "Error", MB_ICONEXCLAMATION | MB_OK);
	}
	
	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
	if(hAccel == NULL) {
		EnqueueLogMessage(FALSE, "WARNING", "WinMain", "Failed to load accelerators table.");
		MessageBox(g_hwndMain, "Accelerators Loading Failed!\n" "(Hotkeys are unavailable.)", "Error", MB_ICONEXCLAMATION | MB_OK);
	}
	
	/* If launched via file association */
	if(lpCmdLine[0] != '\0') {
		if(lpCmdLine[0] == '\"') {
			sscanf_s(lpCmdLine, "\"%[^\"]", szPath, sizeof(szPath));
		} else StringCbCopy(szPath, sizeof(szPath), lpCmdLine);
		
		WinMain_OnFileAssociationLaunch(szPath);
	}
	
	MSG msg; /* A temporary location for all messages */
	
	/* Processed and sent all input to WndProc. */
	while(GetMessage(&msg, NULL, 0, 0) > 0) {
		if(!TranslateAccelerator(g_hwndMain, hAccel, &msg) && !IsDialogMessage(g_hwndMain, &msg)) {
			if(s_mainState.hwndHelp && IsDialogMessage(s_mainState.hwndHelp, &msg)) continue;
			
			TranslateMessage(&msg); /* Translate key codes to chars if present */
			DispatchMessage(&msg); /* Send it to WndProc */
		}
	}
	
	if(s_mainState.hStopThread != NULL) {
		WaitForSingleObject(s_mainState.hStopThread, INFINITE);
		CloseHandle(s_mainState.hStopThread);
	}
	
	if(hHook != NULL) UnhookWindowsHookEx(hHook);
	
	DestroyLoggerThread();
	
	return msg.wParam;
}
