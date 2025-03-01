#include "arc_dialog.h"

ARCDIALOGINFO s_arcDlgInfo;
ARCDIALOGSTATE s_arcDlgState;

LRESULT CALLBACK ArcDlgSubclassProc(HWND hwndEdit, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	switch(msg) {
		case WM_KEYDOWN: {
			if(wParam == VK_F2) {
				s_arcDlgState.modeIndex = (s_arcDlgState.modeIndex + 1) % (sizeof(g_arcModeInfo) / sizeof(ARCHIVERMODEINFO));
				PostMessage(GetParent(hwndEdit), WM_COMMAND, (WPARAM)g_arcModeInfo[s_arcDlgState.modeIndex].modeId, 0);
				
			} else if(wParam == VK_F3) PostMessage(GetParent(hwndEdit), WM_COMMAND, (WPARAM)ID_ARC_SET_INPUT_PATH, 0);
			break;
		}
		
		case WM_LBUTTONDOWN: 
		case WM_LBUTTONDBLCLK: {
			s_arcDlgState.bLButtonDown = TRUE;
			break;
		}
		
		case WM_LBUTTONUP: 
		case WM_RBUTTONUP: {
			s_arcDlgState.bLButtonDown = FALSE;
			break;
		}
	}
	
	switch(g_arcModeInfo[s_arcDlgState.modeIndex].modeId) {
		case ID_ARC_CREATE_MODE: {
			return ArcDlgCreateModeSubclassProc(hwndEdit, msg, wParam, lParam);
		}
		
		case ID_ARC_EXTRACT_MODE: {
			return ArcDlgExtractModeSubclassProc(hwndEdit, msg, wParam, lParam);
		}
	}
	
	return CallWindowProc(s_arcDlgInfo.origInputPathEditProc, hwndEdit, msg, wParam, lParam);
}



static inline LRESULT ArcDlgCreateModeSubclassProc(HWND hwndEdit, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_LBUTTONDOWN: 
		case WM_LBUTTONDBLCLK: 
		case WM_LBUTTONUP: 
		case WM_RBUTTONUP: {
			SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&s_arcDlgState.selStart, (LPARAM)&s_arcDlgState.selEnd);
			SendMessage(s_arcDlgInfo.hOutputPathEdit, EM_SETSEL, (WPARAM)s_arcDlgState.selStart, (LPARAM)s_arcDlgState.selEnd);
		}
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK: {
			return CallWindowProc(s_arcDlgInfo.origInputPathEditProc, hwndEdit, msg, wParam, lParam);
		}
		
		case WM_KEYDOWN: {
			SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&s_arcDlgState.selStart, (LPARAM)&s_arcDlgState.selEnd);
			if((wParam == VK_DELETE) && (s_arcDlgState.selStart == s_arcDlgState.selEnd)) {
				int inputEditTextLength = GetWindowTextLength(hwndEdit);
				
				if(s_arcDlgState.selEnd == inputEditTextLength) {
					return CallWindowProc(s_arcDlgInfo.origInputPathEditProc, hwndEdit, msg, wParam, lParam);
					
				} else if(GetKeyState(VK_CONTROL) & 0x8000) {
					SendMessage(s_arcDlgInfo.hOutputPathEdit, EM_SETSEL, (WPARAM)s_arcDlgState.selStart, (LPARAM)inputEditTextLength);
				}
				
			} else SendMessage(s_arcDlgInfo.hOutputPathEdit, EM_SETSEL, (WPARAM)s_arcDlgState.selStart, (LPARAM)s_arcDlgState.selEnd);
			
			break;
		}
		
		case WM_CHAR: {
			if((GetKeyState(VK_CONTROL) & 0x8000) > 0) {
				return CallWindowProc(s_arcDlgInfo.origInputPathEditProc, hwndEdit, msg, wParam, lParam);
			}
			break;
		}
		
		case WM_SETTEXT: {
			SendMessage(s_arcDlgInfo.hOutputPathEdit, msg, wParam, (LPARAM)VAStr("%s%s", lParam, ARCHIVE_EXT));
			return CallWindowProc(s_arcDlgInfo.origInputPathEditProc, hwndEdit, msg, wParam, lParam);
		}
		
		case WM_CONTEXTMENU: 
		case WM_SETFOCUS: {
			return CallWindowProc(s_arcDlgInfo.origInputPathEditProc, hwndEdit, msg, wParam, lParam);
		}
	}
	
	if(s_arcDlgState.bLButtonDown == FALSE) SendMessage(s_arcDlgInfo.hOutputPathEdit, msg, wParam, lParam);
	
	return CallWindowProc(s_arcDlgInfo.origInputPathEditProc, hwndEdit, msg, wParam, lParam);
}



static inline LRESULT ArcDlgExtractModeSubclassProc(HWND hwndEdit, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_KEYDOWN: {
			if(wParam == VK_DELETE) {
				SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&s_arcDlgState.selStart, (LPARAM)&s_arcDlgState.selEnd);
				if(s_arcDlgState.pathBufLen >= s_arcDlgState.selStart || s_arcDlgState.pathBufLen >= s_arcDlgState.selEnd) {
					return UpdateExtractOutputPathEdit(hwndEdit, msg, wParam, lParam);
				}
			} else if(wParam == VK_CONTROL) {
				s_arcDlgState.bCtrlKeyDown = TRUE;
			}
			break;
		}
		
		case WM_KEYUP: {
			if(wParam == VK_CONTROL) {
				s_arcDlgState.bCtrlKeyDown = FALSE;
			}
			break;
		}
		
		case WM_CHAR: {
			if(s_arcDlgState.bCtrlKeyDown == TRUE) break;
			
			SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&s_arcDlgState.selStart, (LPARAM)&s_arcDlgState.selEnd);
			
			if(wParam == '\\' || wParam == '/') {
				return UpdateExtractOutputPathEdit(hwndEdit, msg, wParam, lParam);
			} else {
				if(s_arcDlgState.selStart == 0 && s_arcDlgState.selEnd == 0) break;
				if(wParam == VK_BACK) {
					if(s_arcDlgState.pathBufLen + 1 >= s_arcDlgState.selStart || s_arcDlgState.pathBufLen + 1 >= s_arcDlgState.selEnd) {
						return UpdateExtractOutputPathEdit(hwndEdit, msg, wParam, lParam);
					}
				} else if(s_arcDlgState.pathBufLen >= s_arcDlgState.selStart || s_arcDlgState.pathBufLen >= s_arcDlgState.selEnd) {
					return UpdateExtractOutputPathEdit(hwndEdit, msg, wParam, lParam);
				}
			}
			break;
		}
		
		case EM_UNDO: 
		case WM_UNDO: 
		case WM_PASTE: 
		case WM_CUT: 
		case WM_CLEAR: 
		case WM_SETTEXT: {
			return UpdateExtractOutputPathEdit(hwndEdit, msg, wParam, lParam);
		}
	}
	
	return CallWindowProc(s_arcDlgInfo.origInputPathEditProc, hwndEdit, msg, wParam, lParam);
}



static inline LRESULT UpdateExtractOutputPathEdit(HWND hwndEdit, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	LRESULT lResult = CallWindowProc(s_arcDlgInfo.origInputPathEditProc, hwndEdit, msg, wParam, lParam);
	
	GetWindowText(hwndEdit, s_arcDlgState.szPathBuf, sizeof(s_arcDlgState.szPathBuf));
	
	StrReplaceAllChar(s_arcDlgState.szPathBuf, '/', '\\');
	PathRemoveFileSpec(s_arcDlgState.szPathBuf);
	s_arcDlgState.pathBufLen = strlen(s_arcDlgState.szPathBuf);
	
	SetWindowTextWithCaretEnd(s_arcDlgInfo.hOutputPathEdit, s_arcDlgState.szPathBuf);
	
	return lResult;
}



INT_PTR CALLBACK ArchiverDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	switch(msg) {
		case WM_INITDIALOG: {
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)g_hIconApp);
			
			ZeroMemory(&s_arcDlgState, sizeof(ARCDIALOGSTATE));
		
			s_arcDlgInfo.hInputPathEdit = GetDlgItem(hwndDlg, IDC_EDIT_ARC_INPUT_PATH);
			s_arcDlgInfo.hOutputPathEdit = GetDlgItem(hwndDlg, IDC_EDIT_ARC_OUTPUT_PATH);
			
			s_arcDlgInfo.origInputPathEditProc = (WNDPROC)SetWindowLongPtr(s_arcDlgInfo.hInputPathEdit, GWLP_WNDPROC, (LONG_PTR)ArcDlgSubclassProc);
			SendMessage(s_arcDlgInfo.hOutputPathEdit, EM_LIMITTEXT, (WPARAM)(MAX_PATH-1), 0);
			
			if(lParam != 0) {
				SendMessage(hwndDlg, WM_COMMAND, (WPARAM)ID_ARC_EXTRACT_MODE, 0);
				SetWindowTextWithCaretEnd(s_arcDlgInfo.hInputPathEdit, (LPSTR)lParam);
			} else SendMessage(hwndDlg, WM_COMMAND, (WPARAM)ID_ARC_CREATE_MODE, 0);
			
			DragAcceptFiles(hwndDlg, TRUE);
			
			PostMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)s_arcDlgInfo.hInputPathEdit, TRUE);
			
			return TRUE;
		}
		
		case WM_LBUTTONDOWN: {
			PostMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)s_arcDlgInfo.hInputPathEdit, TRUE);
			return TRUE;
		}
		
		case WM_DROPFILES: {
			char szFilePath[MAX_PATH_BYTES];
			
			HDROP hDrop = (HDROP)wParam;
			HWND hTarget = s_arcDlgInfo.hInputPathEdit;
			
			DragQueryFile(hDrop, 0, szFilePath, sizeof(szFilePath));
			SetWindowTextWithCaretEnd(hTarget, szFilePath);
			
			DragFinish(hDrop);
			return TRUE;
		}
		
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case ID_ARC_CREATE_MODE: {
					CheckRadioButton(hwndDlg, ID_ARC_CREATE_MODE, ID_ARC_EXTRACT_MODE, ID_ARC_CREATE_MODE);
					s_arcDlgState.modeIndex = 0;
							
					SetWindowText(GetDlgItem(hwndDlg, IDC_LABEL_ARC_INPUT_PATH), "Input Folder Path : ");
					SendMessage(s_arcDlgInfo.hInputPathEdit, EM_LIMITTEXT, (WPARAM)(MAX_PATH-sizeof(ARCHIVE_EXT)), 0);
					
					GetWindowText(s_arcDlgInfo.hInputPathEdit, s_arcDlgState.szPathBuf, min(sizeof(s_arcDlgState.szPathBuf), (MAX_PATH_BYTES-sizeof(ARCHIVE_EXT)+1)));
					SetWindowTextWithCaretEnd(s_arcDlgInfo.hInputPathEdit, s_arcDlgState.szPathBuf);
					
					SetWindowText(GetDlgItem(hwndDlg, IDC_LABEL_ARC_OUTPUT_PATH), "Output File Path : ");
					return TRUE;
				}
				
				case ID_ARC_EXTRACT_MODE: {
					CheckRadioButton(hwndDlg, ID_ARC_CREATE_MODE, ID_ARC_EXTRACT_MODE, ID_ARC_EXTRACT_MODE);
					s_arcDlgState.modeIndex = 1;
							
					SetWindowText(GetDlgItem(hwndDlg, IDC_LABEL_ARC_INPUT_PATH), "Archive File Path : ");
					SendMessage(s_arcDlgInfo.hInputPathEdit, EM_LIMITTEXT, (WPARAM)(MAX_PATH-1), 0);
					
					GetWindowText(s_arcDlgInfo.hInputPathEdit, s_arcDlgState.szPathBuf, min(sizeof(s_arcDlgState.szPathBuf), MAX_PATH_BYTES));
					SetWindowTextWithCaretEnd(s_arcDlgInfo.hInputPathEdit, s_arcDlgState.szPathBuf);
					
					SetWindowText(GetDlgItem(hwndDlg, IDC_LABEL_ARC_OUTPUT_PATH), "Output Folder Path : ");
					return TRUE;
				}
				
				case ID_ARC_SET_INPUT_PATH: {
					switch(g_arcModeInfo[s_arcDlgState.modeIndex].modeId) {
						case ID_ARC_CREATE_MODE: {
							if(SelectFolderPath(hwndDlg, "Select a Folder", BIF_USENEWUI, IDC_EDIT_ARC_INPUT_PATH) > 0) {
								MessageBox(hwndDlg, "Invalid Folder Path!", "Warning", MB_ICONEXCLAMATION | MB_OK);
							}
							break;
						}
						
						case ID_ARC_EXTRACT_MODE: {
							SelectFilePath(hwndDlg, "All Files(*.*)\0*.*\0" "Archive Files(*" ARCHIVE_EXT ")\0*" ARCHIVE_EXT "\0", 2, "Select Archive File", NULL, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST, NULL, IDC_EDIT_ARC_INPUT_PATH);
							break;
						}
					}
					PostMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)s_arcDlgInfo.hInputPathEdit, TRUE);
					return TRUE;
				}
				
				case IDOK: 
				case ID_ARC_RUN_BUTTON: {
					g_appSettings.modeIndex = s_arcDlgState.modeIndex;
					GetWindowText(s_arcDlgInfo.hInputPathEdit, g_appSettings.inputPath, sizeof(g_appSettings.inputPath));
					
					StrReplaceAllChar(g_appSettings.inputPath, '/', '\\');
					SetWindowTextWithCaretEnd(s_arcDlgInfo.hInputPathEdit, g_appSettings.inputPath);
					
					PostMessage(hwndDlg, WM_CLOSE, (WPARAM)ID_ARC_RUN_BUTTON, 0);
					return TRUE;
				}
				
				case IDCANCEL: 
				case ID_ARC_CLOSE_BUTTON: {
					PostMessage(hwndDlg, WM_CLOSE, 0, 0);
					return TRUE;
				}
			}
			
			break;
		}
		
		case WM_CLOSE: {
			EndDialog(hwndDlg, wParam);
			return TRUE;
		}
	}
	
	return FALSE;
}
