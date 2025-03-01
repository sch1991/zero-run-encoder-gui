#include "enc_thrd.h"

ENCODERMODEINFO g_encModeInfo[MAX_ENC_MODE_COUNT] = {
	{ID_ENCODING_MODE, ENC_OPER_ENCODING, ENC_MODE_NEED_INPUT | ENC_MODE_NEED_OUTPUT}, 
	{ID_DECODING_MODE, ENC_OPER_DECODING, ENC_MODE_NEED_INPUT | ENC_MODE_NEED_OUTPUT}, 
	{ID_DETAILS_MODE, ENC_OPER_DETAILS, ENC_MODE_NEED_INPUT}
};

ENCODERTHREAD s_encThread;
PROCESS_INFORMATION s_procInfo;

BOOL CreateEncoderThread(const ENCODERMODEINFO* pModeInfo, LPCSTR lpInputPath, LPCSTR lpOutputPath) {
	
	if(s_encThread.hThread != NULL) {
		EnqueueLogMessage(FALSE, "WARNING", "CreateEncoderThread", "Duplicate encoder thread creation requested.");
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Encoder thread is already running.");
		return FALSE;
	}
	
	/* Check input path validity for all modes */
	if(PathIsAbsoluteAndValid(PATH_TYPE_EXIST, lpInputPath) == FALSE) {
		EnqueueLogMessage(FALSE, "WARNING", "CreateEncoderThread", "Non-existent or relative input path entered.");
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Encoder thread creation failed: Non-existent input path.");
		return FALSE;
	}
	
	/* Check output path validity for each mode */
	if(pModeInfo->nId == ID_ENCODING_MODE) {
		char szOutDirPath[sizeof(lpOutputPath)];
		StringCbCopy(szOutDirPath, sizeof(szOutDirPath), lpOutputPath);
		
		if(PathFileExists(szOutDirPath) == FALSE) {
			if(FileNameIsValid(PathFindFileName(szOutDirPath)) == FALSE) {
				EnqueueLogMessage(FALSE, "WARNING", "CreateEncoderThread", "Invalid output file name entered.");
				SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Encoder thread creation failed: Invalid output file name.");
				return FALSE;
			}
		
			PathRemoveFileSpec(szOutDirPath); /* Remove output file name from output path */
		}
		
		if(PathIsAbsoluteAndValid(PATH_TYPE_EXIST, szOutDirPath) == FALSE) {
			EnqueueLogMessage(FALSE, "WARNING", "CreateEncoderThread", "Invalid or relative output path entered.");
			SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Encoder thread creation failed: Invalid output path.");
			return FALSE;
		}
		
	} else if(pModeInfo->nId == ID_DECODING_MODE) {
		if(PathIsAbsoluteAndValid(PATH_TYPE_EXIST, lpOutputPath) == FALSE) {
			EnqueueLogMessage(FALSE, "WARNING", "CreateEncoderThread", "Non-existent or relative output path entered.");
			SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Encoder thread creation failed: Non-existent output path.");
			return FALSE;
		}
	}
	
	ENCODERTHRDPARAM* pEncParam = (ENCODERTHRDPARAM*)malloc(sizeof(ENCODERTHRDPARAM));
	if(pEncParam == NULL) {
		EnqueueLogMessage(FALSE, "ERROR", "CreateEncoderThread", VAStr("Failed to allocate memory for thread parameters. errno=%d", errno));
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Encoder creation failed: Memory allocation error.");
		return FALSE;
	}
	
	pEncParam->pModeInfo = pModeInfo;
	StringCbCopy(pEncParam->szInputPath, sizeof(pEncParam->szInputPath), lpInputPath);
	StringCbCopy(pEncParam->szOutputPath, sizeof(pEncParam->szOutputPath), lpOutputPath);
	
	ZeroMemory(&s_procInfo, sizeof(PROCESS_INFORMATION));
	
	s_encThread.bThrdStop = FALSE;
	s_encThread.hThread = CreateThread(NULL, 0, EncoderThreadProc, (LPVOID)pEncParam, 0, NULL);
	if(s_encThread.hThread == NULL) {
		EnqueueLogMessage(FALSE, "ERROR", "CreateEncoderThread", VAStr("Encoder thread creation failed. error_code=%04u", GetLastError()));
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Encoder creation failed: Thread creation failed.");
		free(pEncParam);
		return FALSE;
	}
	
	return TRUE;
}



static DWORD WINAPI EncoderThreadProc(LPVOID lpParam) {
	
	ENCODERTHRDPARAM* pEtp = (ENCODERTHRDPARAM*)lpParam;
	
	EnqueueLogMessage(FALSE, "INFO", "EncoderThreadProc", "Encoder thread executed.");
	
	if(pEtp->pModeInfo->nId == ID_ENCODING_MODE) {
		RunEncodingMode(pEtp->pModeInfo->lpOper, pEtp->szInputPath, pEtp->szOutputPath);
		
	} else if(pEtp->pModeInfo->nId == ID_DECODING_MODE) {
		RunDecodingMode(pEtp->pModeInfo->lpOper, pEtp->szInputPath, pEtp->szOutputPath);
		
	} else if(pEtp->pModeInfo->nId == ID_DETAILS_MODE) {
		RunDetailMode(pEtp->pModeInfo->lpOper, pEtp->szInputPath);
		
	} else EnqueueLogMessage(FALSE, "WARNING", "EncoderThreadProc", VAStr("Unimplemented mode id. nId=%04d", pEtp->pModeInfo->nId));
	
	if(s_encThread.bThrdStop == FALSE) {
		PostMessage(g_hwndMain, WM_COMMAND, (WPARAM)MAKELONG(ID_STOP_BUTTON, STOP_THREAD_CALL), 0);
	}
	
	free(pEtp);
	ExitThread(0);
}



static inline void RunEncodingMode(LPCSTR lpModeOper, LPCSTR lpInputPath, LPCSTR lpOutputPath) {
	
	DWORD dwFileAttr;
	char szInFilePath[MAX_PATH_BYTES];
	char szOutFilePath[MAX_PATH_BYTES];
	
	BOOL isTempArchive = FALSE;
	
	dwFileAttr = GetFileAttributes(lpInputPath);
	if(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) {
		if(RunArchiving(lpInputPath, &s_encThread.bThrdStop, szInFilePath, sizeof(szInFilePath)) == FALSE) {
			SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Encoding faild: Archiving failed.");
			return;
		}
		
		isTempArchive = TRUE;
		
	} else StringCbCopy(szInFilePath, sizeof(szInFilePath), lpInputPath);
	
	dwFileAttr = GetFileAttributes(lpOutputPath);
	if(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) {
		char szOutFileName[MAX_PATH_BYTES];
		GetFileNameWithoutExt(lpInputPath, szOutFileName, sizeof(szOutFileName));
		StringCbPrintf(szOutFilePath, sizeof(szOutFilePath), "%s\\%s%s", lpOutputPath, szOutFileName, ZERORE_EXT);
		
	} else StringCbCopy(szOutFilePath, sizeof(szOutFilePath), lpOutputPath);
	
	char szCmdLine[ENC_MAX_COMMAND_LINE];
	StringCbPrintf(szCmdLine, sizeof(szCmdLine), "\"%s\\%s\" %s \"%s\" \"%s\"", g_appConfig.encoderPath, ENC_APP_NAME, lpModeOper, szInFilePath, szOutFilePath);
	
	SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Encoder program executed.");
	
	if(ExecuteEncoderProcess(szCmdLine, NULL, 0) == TRUE) {
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Encoding completed.");
	} else SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Encoding failed.");
	
	if(isTempArchive == TRUE) DeleteFile(szInFilePath);
	
	return;
}



static inline void RunDecodingMode(LPCSTR lpModeOper, LPCSTR lpInFilePath, LPCSTR lpOutDirPath) {
	
	DWORD dwFileAttr;
	
	dwFileAttr = GetFileAttributes(lpInFilePath);
	if(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) {
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Decoding faild: Input path is a folder.");
		return;
	}
	
	dwFileAttr = GetFileAttributes(lpOutDirPath);
	if(!(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY)) {
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Decoding faild: Output path is not a folder.");
		return;
	}
	
	char szCmdLine[ENC_MAX_COMMAND_LINE];
	StringCbPrintf(szCmdLine, sizeof(szCmdLine), "\"%s\\%s\" %s \"%s\" \"%s\"", g_appConfig.encoderPath, ENC_APP_NAME, lpModeOper, lpInFilePath, lpOutDirPath);
	
	SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Encoder program executed.");
	
	char szOutFilePath[MAX_PATH_BYTES];
	if(ExecuteEncoderProcess(szCmdLine, szOutFilePath, sizeof(szOutFilePath)) == FALSE) {
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Decoding failed.");
		return;
	}
	
	SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Decoding completed.");
	
	LPCSTR lpOutFileExt = strrchr(PathFindFileName(szOutFilePath), '.');
	if(lpOutFileExt != NULL && strcmp(lpOutFileExt, ARCHIVE_EXT) == 0) {
		RunArchiveExtraction(szOutFilePath, &s_encThread.bThrdStop, TRUE);
	}
	
	return;
}



static inline void RunDetailMode(LPCSTR lpModeOper, LPCSTR lpInFilePath) {
	
	DWORD dwFileAttr = GetFileAttributes(lpInFilePath);
	if(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) {
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Show details faild: Input path is a folder.");
		return;
	}
	
	char szCmdLine[ENC_MAX_COMMAND_LINE];
	StringCbPrintf(szCmdLine, sizeof(szCmdLine), "\"%s\\%s\" %s \"%s\"", g_appConfig.encoderPath, ENC_APP_NAME, lpModeOper, lpInFilePath);
	
	SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Encoder program executed.");
	
	if(ExecuteEncoderProcess(szCmdLine, NULL, 0) == TRUE) {
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Show details completed.");
	} else SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Show details failed.");
	
	return;
}



static inline BOOL ExecuteEncoderProcess(LPSTR lpCmdLine, LPSTR lpDecOutBuf, size_t cbDecOutBuf) {
	
	SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
	HANDLE hRead, hWrite;
	if (CreatePipe(&hRead, &hWrite, &sa, 0) == 0) {
		EnqueueLogMessage(FALSE, "ERROR", "ExecuteEncoderProcess", VAStr("Failed to create pipe. error_code=%04u", GetLastError()));
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Encoder execute failed: Pipe creation error.");
		return FALSE;
	}
	
	STARTUPINFO si = {sizeof(STARTUPINFO)};
	si.dwFlags		 = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow	 = SW_HIDE;
	si.hStdOutput	 = hWrite;
	si.hStdError	 = hWrite;
	
	if(CreateProcess(NULL, lpCmdLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, g_appConfig.encoderPath, &si, &s_procInfo) != 0) {
		CloseHandle(hWrite);
		
		char szBuffer[MAX_STRING];
		DWORD bufIdx, dwBytesRead;
		
		char szText[MAX_STRING] = "\r\n";
		DWORD textIdx = 2;
		BOOL bPrevCR = FALSE; /* Tracks if previous text started with Carriage Return */
		
		while(ReadFile(hRead, szBuffer, sizeof(szBuffer), &dwBytesRead, NULL) && dwBytesRead > 0) {
			for(bufIdx=0 ; bufIdx<dwBytesRead ; bufIdx++) {
				if(s_encThread.bThrdStop == TRUE) break;
				
				if(szBuffer[bufIdx] == '\r' || textIdx == sizeof(szText)-1) {
					szText[textIdx] = '\0';
					if(szText[0] == '\r' && szText[1] != '\n') {
						if(bPrevCR == FALSE) {
							SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> ");
							bPrevCR = TRUE;
						}
					} else bPrevCR = FALSE;
					
					SendInsertTextMessage(g_console.hwnd, FALSE, szText);
					
					if(lpDecOutBuf != NULL && strncmp(szText, "\r\n" DEC_OUTPUT_PATH_LABEL, sizeof(DEC_OUTPUT_PATH_LABEL)+1) == 0) {
						sscanf_s(szText, "\r\n" DEC_OUTPUT_PATH_LABEL "\"%[^\"]s", lpDecOutBuf, cbDecOutBuf);
					}
					
					textIdx = 0;
				}
				
				szText[textIdx++] = szBuffer[bufIdx];
			}
		}
		
		if(s_encThread.bThrdStop == TRUE) {
			CloseHandle(hRead);
			return FALSE;
		}
		
	} else {
		EnqueueLogMessage(FALSE, "ERROR", "ExecuteEncoderProcess", VAStr("Failed to create encoder process. error_code=%04u", GetLastError()));
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Encoder execute failed: Process creation failed.");
		CloseHandle(hRead);
		return FALSE;
	}
	
	WaitForSingleObject(s_procInfo.hProcess, INFINITE);
	CloseHandle(hRead);
	
	return TRUE;
}



void ShutdownEncoderThread() {
	
	if(s_encThread.hThread == NULL) return;
	
	s_encThread.bThrdStop = TRUE;
	
	if(s_procInfo.hProcess != NULL) {
		SetConsoleCtrlHandler(NULL, TRUE); /* Protect the current process */
		AttachConsole(s_procInfo.dwProcessId); /* Attach to the process to interrupt */
		GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0); /* Generate an interrupt signal */
		
		WaitForSingleObject(s_procInfo.hProcess, INFINITE);
		CloseHandle(s_procInfo.hProcess);
		
		SetConsoleCtrlHandler(NULL, FALSE);
		FreeConsole();
	}
	
	WaitForSingleObject(s_encThread.hThread, INFINITE);
	CloseHandle(s_encThread.hThread);
	
	ZeroMemory(&s_encThread, sizeof(ENCODERTHREAD));
	
	return;
}
