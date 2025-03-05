#include "app_utils.h"

LPCSTR VAStr(LPCSTR lpFormat, ...) {
	
	static char szString[MAX_STRING];
	
	char szBuffer[sizeof(szString)];
	
	va_list args;
    va_start(args, lpFormat);
    StringCbVPrintf(szBuffer, sizeof(szBuffer), lpFormat, args);
    va_end(args);
	
	StringCbCopy(szString, sizeof(szString), szBuffer);
	
	return szString;
}



LRESULT SendInsertTextMessage(HWND hWnd, BOOL cls, LPCSTR lpText) {
	
	static BOOL isEmpty = TRUE;
	
	time_t now = time(NULL);
	
	if(cls) {
		SetWindowText(hWnd, "");
		isEmpty = TRUE;
	}
	
	LPCSTR lpCurText = lpText;
	char szLine[TEXT_MAX_MESSAGE];
	size_t lineLength;
	
	while(lpCurText[0] != '\0') {
		szLine[0] = '\0';
		
		if(lpCurText[0] == '\r') {
			lpCurText++;
			
			if(lpCurText[0] == '\n') {
				if(isEmpty == FALSE) strncat(szLine, "\r\n", 2);
				lpCurText++;
				
			} else if(SendReplaceLineMessage(hWnd, -1, FALSE, "") < 0) return -1;
		}
		
		if(lpCurText[0] == '<' || lpCurText[0] == '>') {
			char szTimestamp[sizeof(TEXT_MESSAGE_TIME_FORMAT)];
			size_t tsLength = strftime(szTimestamp, sizeof(szTimestamp), TEXT_MESSAGE_TIME_FORMAT, localtime(&now));
			strncat(szLine, szTimestamp, tsLength);
		}
		
		lineLength = strcspn(lpCurText, "\r\n");
		strncat(szLine, lpCurText, lineLength);
		lpCurText += lineLength;
		
		if(lineLength != 0) {
			SendMessage(hWnd, EM_SETSEL, GetWindowTextLength(hWnd), (LPARAM)-1);
			if(SendMessage(hWnd, EM_REPLACESEL, FALSE, (LPARAM)szLine) < 0) return -1;
			isEmpty = FALSE;
		}
	}
	
	return 0;
}


inline LRESULT SendReplaceLineMessage(HWND hWnd, int caretPos, BOOL canUndo, LPCSTR lpText) {

	int lineNum, lineIdx;
	
	if(caretPos == -1) {
		lineNum = (int)SendMessage(hWnd, EM_GETLINECOUNT, 0, 0) - 1;
		
	} else lineNum = (int)SendMessage(hWnd, EM_LINEFROMCHAR, caretPos, 0);
	
	lineIdx = (int)SendMessage(hWnd, EM_LINEINDEX, (WPARAM)lineNum, 0);
	SendMessage(hWnd, EM_SETSEL, (WPARAM)lineIdx, (LPARAM)caretPos);
	
	return SendMessage(hWnd, EM_REPLACESEL, canUndo, (LPARAM)lpText);
}



inline BOOL SetWindowTextWithCaretEnd(HWND hWnd, LPCSTR lpText) {

	if(SetWindowText(hWnd, lpText) == FALSE) return FALSE;
		
	int textLength = GetWindowTextLength(hWnd);
	SendMessage(hWnd, EM_SETSEL, (WPARAM)textLength, (LPARAM)textLength);
	
	return TRUE;
}



inline BOOL AppendFileSessionTextToWindow(HWND hWnd, FILE* pFile, LPCSTR lpSession) {
	
	char szBuffer[MAX_STRING];
	char szText[sizeof(szBuffer)];
	
	BOOL isEmpty = FALSE;
	
	while(fgets(szBuffer, sizeof(szBuffer), pFile) != NULL) {
		if(szBuffer[0] == '[') {
			sscanf_s(&szBuffer[1], "%[^]]", szText, sizeof(szText));
			if(strcmp(szText, lpSession) == 0) break;
		}
	}
	
	if(feof(pFile) != 0) return FALSE; /* Target session not found or empty */
	
	if(GetWindowTextLength(hWnd) == 0) isEmpty = TRUE;
	
	while(fgets(szBuffer, sizeof(szBuffer), pFile) != NULL) {
		if(szBuffer[0] != '[') {
			if(sscanf_s(szBuffer, "%[^\n]", szText, sizeof(szText)) == 0) szText[0] = '\0';
			
			if(isEmpty == TRUE) {
				SendMessage(hWnd, EM_REPLACESEL, FALSE, (LPARAM)szText);
				isEmpty = FALSE;
				
			} else SendMessage(hWnd, EM_REPLACESEL, FALSE, (LPARAM)VAStr("%s%s", "\r\n", szText));
			
		} else break;
	}
	
	return TRUE;
}



inline DWORD MakePathAndGetFileAttr(LPCSTR lpDirPath, LPCSTR lpFileName) {
	
	char szFilePath[MAX_PATH_BYTES];
	StringCbPrintf(szFilePath, sizeof(szFilePath), "%s\\%s", lpDirPath, lpFileName);
	
	return GetFileAttributes(szFilePath);
}



inline DWORD MakePathBufAndGetFileAttr(LPSTR lpPathBuf, LPCSTR lpFileName, size_t cbPathBuf) {
	
	char szDirPath[MAX_PATH_BYTES];
	
	StringCbCopy(szDirPath, sizeof(szDirPath), lpPathBuf);
	StringCbPrintf(lpPathBuf, cbPathBuf, "%s\\%s", szDirPath, lpFileName);
	
	return GetFileAttributes(lpPathBuf);
}



inline BOOL SelectFilePath(HWND hwnd, LPCSTR filter, int filterIndex, LPCSTR title, LPCSTR defExt, DWORD flags, LPCSTR defFileName, int controlId) {
	
	OPENFILENAME ofn;
	char szFile[MAX_PATH_BYTES] = {'\0'};
	
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	if(defFileName != NULL) StringCbCopy(szFile, sizeof(szFile), defFileName);
	
	ofn.lStructSize	 = sizeof(OPENFILENAME);
	ofn.hwndOwner	 = hwnd;
	ofn.lpstrFile	 = szFile;
	ofn.nMaxFile	 = sizeof(szFile);
	
	ofn.lpstrFilter	 = filter;
	ofn.nFilterIndex = filterIndex;
	ofn.lpstrTitle	 = title;
	ofn.lpstrDefExt	 = defExt;
	ofn.Flags = flags;
	
	char szOrigCurDir[MAX_PATH_BYTES];
	GetCurrentDirectory(sizeof(szOrigCurDir), szOrigCurDir);
	
	if(GetOpenFileName(&ofn) == TRUE) {
		SetWindowTextWithCaretEnd(GetDlgItem(hwnd, controlId), ofn.lpstrFile);
		
	} else return FALSE;
	
	SetCurrentDirectory(szOrigCurDir);
	
	return TRUE;
}



inline int SelectFolderPath(HWND hwnd, LPCSTR title, DWORD flags, int controlId) {
	
	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	
	bi.lpszTitle = title;
	bi.ulFlags	 = flags;
	
	PIDLIST_ABSOLUTE pidl = SHBrowseForFolder(&bi);
	if(pidl != NULL) {
		char szPath[MAX_PATH];
		
		BOOL bResult = SHGetPathFromIDList(pidl, szPath);
		CoTaskMemFree(pidl);
		
		if(bResult == TRUE) {
			SetWindowTextWithCaretEnd(GetDlgItem(hwnd, controlId), szPath);
			
		} else return 1;
		
	} else return -1;
	
	return 0;
}



inline BOOL StrReplaceAllChar(LPSTR lpStr, char chOld, char chNew) {
	
	if(lpStr == NULL) return FALSE;
	
	char* strPtr;
	
	for(strPtr=lpStr ; *strPtr!='\0' ; strPtr++) {
		if(*strPtr == chOld) *strPtr = chNew;
	}
	
	return TRUE;
}



inline BOOL FileNameIsValid(LPCSTR lpFileName) {
	
	if(*lpFileName == '\0') return FALSE;
	if(lpFileName[strcspn(lpFileName, "\\/:*?\"<>|")] != '\0') return FALSE;
	
	return TRUE;
}



inline BOOL PathIsAbsoluteAndValid(int pathType, LPCSTR lpPath) {
	
	if(PathIsRelative(lpPath) == TRUE) return FALSE;
		
	switch(pathType) {
		case PATH_TYPE_FILE: {
			DWORD dwFileAttr = GetFileAttributes(lpPath);
			if(dwFileAttr == INVALID_FILE_ATTRIBUTES || (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY)) return FALSE;
			break;
		}
		
		case PATH_TYPE_DIR: {
			DWORD dwFileAttr = GetFileAttributes(lpPath);
			if(!(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY)) return FALSE;
			break;
		}
		
		default: {
			if(PathFileExists(lpPath) == FALSE) return FALSE;
		}
	}
	
	return TRUE;
}



inline LPSTR GetFileNameWithoutExt(LPCSTR pszPath, LPSTR lpPathBuf, size_t cbPathBuf) {
	
	StringCbCopy(lpPathBuf, cbPathBuf, PathFindFileName(pszPath));
	
	char* pFileExt;
	if((pFileExt = strrchr(lpPathBuf, '.')) != NULL) *pFileExt = '\0';
	
	return lpPathBuf;
}



inline BOOL IsWindowEnableAndVisible(HWND hWnd) {
	
	return (IsWindowEnabled(hWnd) && IsWindowVisible(hWnd));
}



inline BOOL ShowWindowWithMenuItem(HWND hwnd, int commandId, int nCmdShow, UINT uMenuEnable) {
	
	BOOL bPrevWindowState = ShowWindow(GetDlgItem(hwnd, commandId), nCmdShow);
	EnableMenuItem(GetMenu(hwnd), commandId, MF_BYCOMMAND | uMenuEnable);
	
	return bPrevWindowState;
}



inline BOOL EnableWindowWithMenuItem(HWND hwnd, int commandId, BOOL bBtnEnable, UINT uMenuEnable) {
	
	BOOL bPrevWindowState = EnableWindow(GetDlgItem(hwnd, commandId), bBtnEnable);
	EnableMenuItem(GetMenu(hwnd), commandId, MF_BYCOMMAND | uMenuEnable);
	
	return bPrevWindowState;
}
