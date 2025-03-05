#include "app_procs.h"

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	
	static DWORD selStart, selEnd;
	
	if(nCode == HC_ACTION && !(lParam & 0x80000000)) {
		switch(wParam) {
			case 'A': {
				if((GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000) && !(GetKeyState(VK_MENU) & 0x8000)) {
					HWND hFocus = GetFocus();
					LONG_PTR style = GetWindowLongPtr(hFocus, GWL_STYLE);
					if(style & (ES_AUTOHSCROLL | ES_MULTILINE)) {
						SendMessage(hFocus, EM_SETSEL, 0, (LPARAM)-1);
						return 1;
					}
				}
				break;
			}
			
			case VK_BACK: {
				if(GetKeyState(VK_CONTROL) & 0x8000) {
					HWND hFocus = GetFocus();
					LONG_PTR style = GetWindowLongPtr(hFocus, GWL_STYLE);
					if((style & (ES_AUTOHSCROLL | ES_MULTILINE)) && !(style & ES_READONLY)) { 
						SendMessage(hFocus, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);
						if(selStart != selEnd) {
							SendMessage(hFocus, EM_SETSEL, (WPARAM)selStart, (LPARAM)selEnd);
							SendMessage(hFocus, EM_REPLACESEL, TRUE, (LPARAM)"");
							
						} else SendReplaceLineMessage(hFocus, selEnd, TRUE, "");
						
						return 1;
					}
				}
				break;
			}
			
			case VK_LEFT: 
			case VK_RIGHT: {
				if(!(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000) && !(GetKeyState(VK_MENU) & 0x8000)) {
					HWND hFocus = GetFocus();
					LONG_PTR style = GetWindowLongPtr(hFocus, GWL_STYLE);
					if(style & (ES_AUTOHSCROLL | ES_MULTILINE)) {
						SendMessage(hFocus, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);
						if(selStart != selEnd) {
							if(wParam == VK_LEFT) {
								SendMessage(hFocus, EM_SETSEL, (WPARAM)selStart, (LPARAM)selStart);
							} else if(wParam == VK_RIGHT) SendMessage(hFocus, EM_SETSEL, (WPARAM)selEnd, (LPARAM)selEnd);
							
							return 1;
						}
					}
				}
				break;
			}
			
			case VK_UP: 
			case VK_DOWN: {
				if(!(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000) && !(GetKeyState(VK_MENU) & 0x8000)) {
					HWND hFocus = GetFocus();
					LONG_PTR style = GetWindowLongPtr(hFocus, GWL_STYLE);
					if((style & ES_AUTOHSCROLL) && !(style & ES_MULTILINE)) {
						SendMessage(hFocus, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);
						if(selStart != selEnd) {
							if(wParam == VK_UP) {
								SendMessage(hFocus, EM_SETSEL, (WPARAM)selStart, (LPARAM)selStart);
							} else if(wParam == VK_DOWN) SendMessage(hFocus, EM_SETSEL, (WPARAM)selEnd, (LPARAM)selEnd);
							
						} else {
							if(wParam == VK_UP) {
								SendMessage(hFocus, EM_SETSEL, 0, 0);
							} else if(wParam == VK_DOWN) {
								int testLength = GetWindowTextLength(hFocus);
								SendMessage(hFocus, EM_SETSEL, (WPARAM)testLength, (LPARAM)testLength);
							}
						}
						return 1;
					}
				}
				break;
			}
		}
	}
	
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}



LRESULT CALLBACK ConsoleSubclassProc(HWND hwndEdit, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	switch(msg) {
		case WM_CHAR: {
			switch(wParam) {
				case VK_TAB: {
					SendMessage(g_hwndMain, WM_LBUTTONDOWN, 0, 0);
					SendMessage(g_hwndMain, WM_NEXTDLGCTL, 0, FALSE);
					return 0;
				}
				case VK_RETURN: {
					SendMessage(g_hwndMain, WM_LBUTTONDOWN, 0, 0);
					SendMessage(g_hwndMain, WM_COMMAND, (WPARAM)IDOK, 0);
					return 0;
				}
			}
			break;
		}
		
		case WM_LBUTTONDOWN: {
			SendMessage(g_hwndMain, WM_LBUTTONDOWN, 0, 0);
			LRESULT lResult = CallWindowProc(g_console.origProc, hwndEdit, msg, wParam, lParam);
			HideCaret(hwndEdit);
			return lResult;
		}
		
		case WM_LBUTTONUP: {
			HideCaret(hwndEdit);
			break;
		}
		
		case WM_COPY: {
			PostMessage(g_hwndMain, WM_LBUTTONDOWN, 0, 0);
			break;
		}
	}
	
	return CallWindowProc(g_console.origProc, hwndEdit, msg, wParam, lParam);
}



DWORD WINAPI StopThreadProc(LPVOID lpParam) {
	
	ShutdownArchiverThread();
	ShutdownEncoderThread();
	
	PostMessage(g_hwndMain, WM_COMMAND, (WPARAM)MAKELONG(ID_STOP_BUTTON, STOP_THREAD_DONE), 0);
	
	if((WORD)(UINT_PTR)lpParam == STOP_THREAD_CALL) {
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n>> Run finished <<");
	} else SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n>> Run stopped <<");
	
	ExitThread(0);
}



LRESULT CALLBACK HelpSubclassProc(HWND hwndEdit, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	
	switch(msg) {
		case WM_CHAR: {
			switch(wParam) {
				case VK_TAB: {
					SendMessage(g_hwndMain, WM_LBUTTONDOWN, 0, 0);
					return 0;
				}
				case VK_RETURN: {
					SendMessage((HWND)dwRefData, WM_COMMAND, (WPARAM)IDOK, 0);
					return 0;
				}
			}
			break;
		}
		
		case WM_SETFOCUS: {
			HideCaret(hwndEdit);
			return 0;
		}
	}
	
	return DefSubclassProc(hwndEdit, msg, wParam, lParam);
}



INT_PTR CALLBACK HelpDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	switch(msg) {
		case WM_INITDIALOG: {
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)g_hIconApp);
			SetWindowLongPtr(hwndDlg, GWLP_HWNDPARENT, (LONG_PTR)NULL);
			
			HWND hContent = GetDlgItem(hwndDlg, IDC_EDIT_HELP_CONTENT);
			
			SetWindowSubclass(hContent, HelpSubclassProc, 0, (DWORD_PTR)hwndDlg);
			
			FILE* pReadme;
			errno_t err;
			
			if((err = fopen_s(&pReadme, README_PATH, "r")) != 0) {
				EnqueueLogMessage(FALSE, "WARNING", "HelpDialogProc", VAStr("Failed to open \"" README_PATH "\". errno=%d", err));
				MessageBox(hwndDlg, "Readme File Opening Failed!\n" "(Cannot load help content.)", "Warning", MB_ICONEXCLAMATION | MB_OK);
				return TRUE;
			}
			
			AppendFileSessionTextToWindow(hContent, pReadme, "USER");
			
			fclose(pReadme);
			
			return TRUE;
		}
		
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDOK: 
				case IDCANCEL: {
					PostMessage(hwndDlg, WM_CLOSE, 0, 0);
					return TRUE;
				}
			}
			
			break;
		}
		
		case WM_CLOSE: {
			ShowWindow(hwndDlg, SW_HIDE);
			return TRUE;
		}
	}
	
	return FALSE;
}
