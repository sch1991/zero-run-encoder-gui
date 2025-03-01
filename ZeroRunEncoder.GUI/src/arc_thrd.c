#include "arc_thrd.h"

ARCHIVERMODEINFO g_arcModeInfo[MAX_ARC_MODE_COUNT] = {
	{ID_ARC_CREATE_MODE, ARC_OPER_CREATE}, 
	{ID_ARC_EXTRACT_MODE, ARC_OPER_EXTRACT}
};

ARCHIVERTHREAD s_arcThread;
volatile BOOL* s_pbStopRun;

BOOL CreateArchiverThread(int operId, LPCSTR lpPath) {
	
	if(s_arcThread.hThread != NULL) {
		EnqueueLogMessage(FALSE, "WARNING", "CreateArchiverThread", "Duplicate archiver thread creation requested.");
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Archiver thread is already running.");
		return FALSE;
	}
	
	if(PathIsAbsoluteAndValid(PATH_TYPE_EXIST, lpPath) == FALSE) {
		EnqueueLogMessage(FALSE, "WARNING", "CreateArchiverThread", "Non-existent path or relative path entered.");
		SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\r\n> Archiver thread creation failed: Non-existent path.\r\n" "Path: \"%s\"", lpPath));
		return FALSE;
	}
	
	ARCHIVERTHRDPARAM* pArcParam = (ARCHIVERTHRDPARAM*)malloc(sizeof(ARCHIVERTHRDPARAM));
	if(pArcParam == NULL) {
		EnqueueLogMessage(FALSE, "ERROR", "CreateArchiverThread", VAStr("Failed to allocate memory for thread parameters. errno=%d", errno));
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Archiver thread creation failed: Memory allocation error.");
		return FALSE;
	}
	
	pArcParam->operId = operId;
	StringCbCopy(pArcParam->szInputPath, sizeof(pArcParam->szInputPath), lpPath);
	
	s_arcThread.bThrdStop = FALSE;
	s_arcThread.hThread = CreateThread(NULL, 0, ArchiverThreadProc, (LPVOID)pArcParam, 0, NULL);
	if(s_arcThread.hThread == NULL) {
		EnqueueLogMessage(FALSE, "ERROR", "CreateArchiverThread", VAStr("Archiver thread creation failed. error_code=%04u", GetLastError()));
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Archiver thread creation failed: Thread creation failed.");
		free(pArcParam);
		return FALSE;
	}
	
	return TRUE;
}



static DWORD WINAPI ArchiverThreadProc(LPVOID lpParam) {
	
	ARCHIVERTHRDPARAM* pAtp = (ARCHIVERTHRDPARAM*)lpParam;
	
	EnqueueLogMessage(FALSE, "INFO", "ArchiverThreadProc", "Archiver thread executed.");
	
	if(pAtp->operId == ARC_OPER_CREATE) {
		RunArchiving(pAtp->szInputPath, NULL, NULL, 0);
		
	} else if(pAtp->operId == ARC_OPER_EXTRACT) {
		RunArchiveExtraction(pAtp->szInputPath, NULL, FALSE);
		
	} else EnqueueLogMessage(FALSE, "WARNING", "ArchiverThreadProc", VAStr("Invalid operId received. operId=%02X", pAtp->operId));
	
	if(s_arcThread.bThrdStop == FALSE) {
		PostMessage(g_hwndMain, WM_COMMAND, (WPARAM)MAKELONG(ID_STOP_BUTTON, STOP_THREAD_CALL), 0);
	}
	
	free(pAtp);
	ExitThread(0);
}



BOOL RunArchiving(LPCSTR lpInDirPath, volatile BOOL* pStopRunFlag, LPSTR lpOutPathBuf, size_t cbOutPathBuf) {
	
	if(pStopRunFlag == NULL) {
		s_pbStopRun = &s_arcThread.bThrdStop;
	} else s_pbStopRun = pStopRunFlag;
	
	DWORD dwFileAttr = GetFileAttributes(lpInDirPath);
	if(!(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY)) {
		EnqueueLogMessage(FALSE, "WARNING", "RunArchiving", "Archiving requested with a non-directory path.");
		SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\r\n> Archiving failed: Input path is not a folder.\r\n" "Input path: \"%s\"", lpInDirPath));
		return FALSE;
	}
	
	SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\r\n> Archiving started.\r\n" "Input folder name: \'%s\'", PathFindFileName(lpInDirPath)));
	
	DWORD dwFileCnt = 0, dwDirCnt = 0;
	
	if(CountDirContents(&dwFileCnt, &dwDirCnt, lpInDirPath) == FALSE) {
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Failed to count folder contents.");
	}
	SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\r\nFiles to archive: %u\r\n" "Folders to archive: %u", dwFileCnt, dwDirCnt));

	char szOutFilePath[MAX_PATH_BYTES];
	StringCbPrintf(szOutFilePath, sizeof(szOutFilePath), "%s%s", lpInDirPath, ARCHIVE_EXT);
	
	HANDLE hArchive = CreateFile(szOutFilePath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hArchive == INVALID_HANDLE_VALUE) {
		EnqueueLogMessage(FALSE, "ERROR", "RunArchiving", VAStr("Failed to open archive file. error_code=%04u", GetLastError()));
		SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\r\n> Archiving failed: File creation error.\r\n" "Output file name: \'%s\'", PathFindFileName(szOutFilePath)));
		return FALSE;
		
	} else {
		DWORD dwArchiveHead = ARCHIVE_HEAD;
		WriteFile(hArchive, &dwArchiveHead, sizeof(DWORD), NULL, NULL);
	}
	
	DrawProgress(SET_PROG_DEFAULT, dwFileCnt + dwDirCnt);
	
	char szBuffer[g_appConfig.archiverBufferSize << ARC_BUF_OFFSET];
	
	if(WriteArchiveFile(lpInDirPath, hArchive, szBuffer, sizeof(szBuffer)) == FALSE) {
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Archiving failed.");
		CloseHandle(hArchive);
		DeleteFile(szOutFilePath);
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Incomplete file deleted.");
		return FALSE;
	}
	
	CloseHandle(hArchive);
	
	SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\r\n> Archiving completed.\r\n" "Output file name: \'%s\'", PathFindFileName(szOutFilePath)));
	
	/* If lpOutPathBuf is not NULL, copy the new archive file path into lpOutPathBuf */
	if(lpOutPathBuf != NULL) {
		StringCbCopy(lpOutPathBuf, cbOutPathBuf, szOutFilePath);
	}
	
	return TRUE;
}



BOOL RunArchiveExtraction(LPCSTR lpInFilePath, volatile BOOL* pStopRunFlag, BOOL bDelInputFile) {
	
	if(pStopRunFlag == NULL) {
		s_pbStopRun = &s_arcThread.bThrdStop;
	} else s_pbStopRun = pStopRunFlag;
	
	LPCSTR lpInFileExt = strrchr(PathFindFileName(lpInFilePath), '.');
	if(lpInFileExt == NULL || strcmp(lpInFileExt, ARCHIVE_EXT) != 0) {
		EnqueueLogMessage(FALSE, "WARNING", "RunArchiveExtraction", "Extraction requested for a file with a non-\'" ARCHIVE_EXT "\' extension.");
		SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\r\n> Archive extraction failed: Input file extension is not \'%s\'.\r\n" "Input file name: \'%s\'", ARCHIVE_EXT, PathFindFileName(lpInFilePath)));
		return FALSE;
	}
	
	SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\r\n> Archive extraction started.\r\n" "Input file name: \'%s\'", PathFindFileName(lpInFilePath)));
	
	HANDLE hArchive = CreateFile(lpInFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hArchive == INVALID_HANDLE_VALUE) {
		EnqueueLogMessage(FALSE, "ERROR", "RunArchiveExtraction", VAStr("Failed to open archive file. error_code=%04u", GetLastError()));
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Archive extraction failed: File open error.");
		return FALSE;
		
	} else {
		DWORD dwArchiveHead;
		ReadFile(hArchive, &dwArchiveHead, sizeof(DWORD), NULL, NULL);
		if(dwArchiveHead != ARCHIVE_HEAD) {
			EnqueueLogMessage(FALSE, "WARNING", "RunArchiveExtraction", "Archive file header mismatch.");
			SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Archive extraction failed: File is not an archive.");
			return FALSE;
		}
	}

	char szOutDirPath[MAX_PATH_BYTES];
	StringCbCopy(szOutDirPath, (strlen(lpInFilePath) - sizeof(ARCHIVE_EXT) + 2), lpInFilePath);
	
	char szTempDirPath[MAX_PATH_BYTES];
	StringCbCopy(szTempDirPath, sizeof(szTempDirPath), szOutDirPath);
	
	int dupIdx = 1;
	while(PathFileExists(szTempDirPath) == TRUE && dupIdx<MAX_DUP_INDEX) {
		StringCbPrintf(szTempDirPath, sizeof(szTempDirPath), "%s (%d)", szOutDirPath, ++dupIdx);
	}
	
	LARGE_INTEGER archiveSize;
	if(GetFileSizeEx(hArchive, &archiveSize) == 0) {
		EnqueueLogMessage(FALSE, "ERROR", "RunArchiveExtraction", VAStr("Failed to get archive file size. error_code=%04u", GetLastError()));
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Failed to get archive file size.");
		archiveSize.QuadPart = 0;
	}
	
	DrawProgress(SET_PROG_PERCENT, archiveSize.QuadPart - sizeof(DWORD));
	
	char szBuffer[g_appConfig.archiverBufferSize << ARC_BUF_OFFSET];
	
	CreateDirectory(szTempDirPath, NULL);
	if(ExtractArchiveFile(hArchive, szTempDirPath, szBuffer, sizeof(szBuffer)) == FALSE) {
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> Archive extraction failed.");
		CloseHandle(hArchive);
		if(DeleteDir(szTempDirPath) == FALSE) {
			SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\r\n> Extracting folder deletion incomplete.\r\n" "Temp folder name: \'%s\'", PathFindFileName(szTempDirPath)));
		} else SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\r\n> Extracting folder deleted.\r\n" "Temp folder name: \'%s\'", PathFindFileName(szTempDirPath)));
		return FALSE;
	}
	
	if(dupIdx > 1 && MergeDir(szTempDirPath, szOutDirPath) == FALSE) {
		SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\r\n> Folder merge incomplete.\r\n" "Temp folder name: \'%s\'", PathFindFileName(szTempDirPath)));
	}
	
	CloseHandle(hArchive);
	
	SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\r\n> Archive extraction completed.\r\n" "Output folder name: \'%s\'", PathFindFileName(szOutDirPath)));
	
	if(bDelInputFile == TRUE) {
		DeleteFile(lpInFilePath);
	}
	
	return TRUE;
}



static BOOL WriteArchiveFile(LPCSTR lpInDirPath, HANDLE hArchive, LPSTR lpBuffer, size_t cbBuffer) {
	
	WIN32_FIND_DATA findData;
	char szTargetPath[MAX_PATH_BYTES];
	
	HANDLE hFind;

	StringCbPrintf(szTargetPath, sizeof(szTargetPath), "%s\\*", lpInDirPath);
	hFind = FindFirstFile(szTargetPath, &findData);
	if(hFind == INVALID_HANDLE_VALUE) {
		EnqueueLogMessage(FALSE, "ERROR", "WriteArchiveFile", VAStr("Failed to create file find handle. error_code=%04u", GetLastError()));
		return FALSE;
	}
	
	char szFilePath[MAX_PATH_BYTES];
	HANDLE hFile;
	
	DWORD dwBytesRead, dwBytesWritten;
	DWORD64 qwFileSize;
	
	do {
		if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if(strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) continue;
			
			WriteFile(hArchive, findData.cFileName, strlen(findData.cFileName) + 1, NULL, NULL);
			WriteFile(hArchive, &findData.dwFileAttributes, sizeof(DWORD), NULL, NULL);
			
			StringCbPrintf(szTargetPath, sizeof(szTargetPath), "%s\\%s", lpInDirPath, findData.cFileName);
			if(WriteArchiveFile(szTargetPath, hArchive, lpBuffer, cbBuffer) == FALSE) {
				FindClose(hFind);
				return FALSE;
			}
			
		} else {
			StringCbPrintf(szFilePath, sizeof(szFilePath), "%s\\%s", lpInDirPath, findData.cFileName);
			
			hFile = CreateFile(szFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if(hFile == INVALID_HANDLE_VALUE) {
				SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\rFile excluded: \'%s\'" "\r\n> ", findData.cFileName));
				DrawProgress(ADD_PROG_COUNT, 0);
				continue;
			}
			
			WriteFile(hArchive, findData.cFileName, strlen(findData.cFileName) + 1, NULL, NULL);
			WriteFile(hArchive, &findData.dwFileAttributes, sizeof(DWORD), NULL, NULL);
			
			qwFileSize = ((DWORD64)findData.nFileSizeHigh << 32) | (DWORD64)findData.nFileSizeLow;
			WriteFile(hArchive, &qwFileSize, sizeof(DWORD64), NULL, NULL);
	
			while(ReadFile(hFile, lpBuffer, cbBuffer, &dwBytesRead, NULL) && dwBytesRead > 0) {
				WriteFile(hArchive, lpBuffer, dwBytesRead, &dwBytesWritten, NULL);
				if(dwBytesRead != dwBytesWritten) {
					EnqueueLogMessage(FALSE, "ERROR", "WriteArchiveFile", VAStr("dwBytesRead and dwBytesWritten mismatch. (dwBytesRead=%u) != (dwBytesWritten=%u)", dwBytesRead, dwBytesWritten));
					return FALSE;
				}
				
				if(*s_pbStopRun == TRUE) break;
			}
	
			CloseHandle(hFile);
		}
		
		DrawProgress(ADD_PROG_COUNT, 1);
		
		if(*s_pbStopRun == TRUE) {
			FindClose(hFind);
			return FALSE;
		}
		
	} while(FindNextFile(hFind, &findData));
	
	FindClose(hFind);
	
	WriteFile(hArchive, "", 1, NULL, NULL);
	
	return TRUE;
}



static BOOL ExtractArchiveFile(HANDLE hArchive, LPCSTR lpOutDirPath, LPSTR lpBuffer, size_t cbBuffer) {

	char szFileName[MAX_PATH_BYTES];
	DWORD64 qwBytesCount;
	
	char szFilePath[MAX_PATH_BYTES];
	HANDLE hFile;
	
	DWORD dwBytesRead, dwBytesToRead;
	DWORD dwFileAttr;
	DWORD64 qwFileSize;

	while(TRUE) {
		qwBytesCount = 0;
		do {
			ReadFile(hArchive, &szFileName[qwBytesCount], 1, &dwBytesRead, NULL);
			if(dwBytesRead == 0) {
				EnqueueLogMessage(FALSE, "ERROR", "ExtractArchiveFile", "Archive file EOF is not \'\0\'.");
				return FALSE; /* End of File */
			}
		} while(szFileName[qwBytesCount++] != '\0');
		
		DrawProgress(ADD_PROG_COUNT, qwBytesCount);
		
		if(szFileName[0] == '\0') break; /* End of Directory */

		
		ReadFile(hArchive, &dwFileAttr, sizeof(DWORD), NULL, NULL);
		qwBytesCount = sizeof(DWORD);
		
		if(!(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY)) {
			ReadFile(hArchive, &qwFileSize, sizeof(DWORD64), NULL, NULL);
			qwBytesCount += sizeof(DWORD64);
		}
		
		DrawProgress(ADD_PROG_COUNT, qwBytesCount);

		StringCbPrintf(szFilePath, sizeof(szFilePath), "%s\\%s", lpOutDirPath, szFileName);
		
		if(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) {
			CreateDirectory(szFilePath, NULL);
			if(ExtractArchiveFile(hArchive, szFilePath, lpBuffer, cbBuffer) == FALSE) return FALSE;
			
		} else {
			hFile = CreateFile(szFilePath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if(hFile == INVALID_HANDLE_VALUE) {
				SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\rFile excluded: \'%s\'" "\r\n> ", szFileName));
				SetFilePointer(hArchive, qwFileSize, NULL, FILE_CURRENT);
				DrawProgress(ADD_PROG_COUNT, qwFileSize);
				continue;
			}
			
			qwBytesCount = qwFileSize;
			
			while(qwBytesCount > 0) {
				dwBytesToRead = min(cbBuffer, qwBytesCount);
				ReadFile(hArchive, lpBuffer, dwBytesToRead, &dwBytesRead, NULL);
				if(dwBytesToRead != dwBytesRead) {
					EnqueueLogMessage(FALSE, "ERROR", "ExtractArchiveFile", VAStr("dwBytesToRead and dwBytesRead mismatch. (dwBytesToRead=%u) != (dwBytesRead=%u)", dwBytesToRead, dwBytesRead));
					SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\rFile extraction failed: \'%s\'" "\r\n> ", szFileName));
					SetFilePointer(hArchive, qwBytesCount-dwBytesRead, NULL, FILE_CURRENT);
					break;
				}
				WriteFile(hFile, lpBuffer, dwBytesRead, &dwBytesRead, NULL);
				qwBytesCount -= dwBytesRead;
				
				if(*s_pbStopRun == TRUE) break;
			}
			
			DrawProgress(ADD_PROG_COUNT, qwFileSize);
	
			CloseHandle(hFile);
		}
		
		SetFileAttributes(szFilePath, dwFileAttr);
		
		if(*s_pbStopRun == TRUE) return FALSE;
	}
	
	return TRUE;
}



static BOOL CountDirContents(DWORD* pFileCnt, DWORD* pDirCnt, LPCSTR lpInDirPath) {
	
	WIN32_FIND_DATA findData;
	char szTargetPath[MAX_PATH_BYTES];
	
	HANDLE hFind;
	
	StringCbPrintf(szTargetPath, sizeof(szTargetPath), "%s\\*", lpInDirPath);
	hFind = FindFirstFile(szTargetPath, &findData);
	if(hFind == INVALID_HANDLE_VALUE) {
		EnqueueLogMessage(FALSE, "ERROR", "CountDirContents", VAStr("Failed to create file find handle. error_code=%04u", GetLastError()));
		return FALSE;
	}
	
	do {
		if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if(strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) continue;
			(*pDirCnt)++;
			StringCbPrintf(szTargetPath, sizeof(szTargetPath), "%s\\%s", lpInDirPath, findData.cFileName);
			if(CountDirContents(pFileCnt, pDirCnt, szTargetPath) == FALSE) {
				FindClose(hFind);
				return FALSE;
			}
			
		} else (*pFileCnt)++;
		
		if(*s_pbStopRun == TRUE) {
			FindClose(hFind);
			return FALSE;
		}
		
	} while(FindNextFile(hFind, &findData));
	
	FindClose(hFind);
	
	return TRUE;
}



static BOOL MergeDir(LPCSTR lpSrcDir, LPCSTR lpDestDir) {
	
	char szSrcFiles[MAX_PATH_BYTES] = {'\0'}; /* Double null-terminated */
	StringCbPrintf(szSrcFiles, sizeof(szSrcFiles), "%s\\*", lpSrcDir);
	
	char szDestDir[MAX_PATH_BYTES] = {'\0'}; /* Double null-terminated */
	StringCbCopy(szDestDir, sizeof(szDestDir), lpDestDir);
	
	SHFILEOPSTRUCT shfo;
	
	ZeroMemory(&shfo, sizeof(SHFILEOPSTRUCT));
	shfo.wFunc	 = FO_MOVE;
	shfo.pFrom	 = szSrcFiles;
	shfo.pTo	 = szDestDir;
	shfo.fFlags	 = FOF_NOCONFIRMMKDIR;

	if(SHFileOperation(&shfo) == 0) {
		if(RemoveDirectory(lpSrcDir) == FALSE) return FALSE;
		
	} else return FALSE;
	
	return TRUE;
}



static BOOL DeleteDir(LPCSTR lpDirPath) {
	
	char szDirPath[MAX_PATH_BYTES] = {'\0'}; /* Double null-terminated */
	StringCbCopy(szDirPath, sizeof(szDirPath), lpDirPath);
	
	SHFILEOPSTRUCTA shfo;
	
	ZeroMemory(&shfo, sizeof(SHFILEOPSTRUCT));
	shfo.wFunc	 = FO_DELETE;
	shfo.pFrom	 = szDirPath;
	shfo.fFlags	 = FOF_NOCONFIRMATION | FOF_SILENT;

	if(SHFileOperationA(&shfo) != 0) return FALSE;
	
	return TRUE;
}



static void DrawProgress(int setId, DWORD64 qwCnt) {
	
	static PROGRESSDRAWINFO pdi;
	
	ULONGLONG curTick = GetTickCount64();
	
	if(setId != ADD_PROG_COUNT) {
		pdi.type = setId;
		pdi.count = 0;
		pdi.total = qwCnt;
		pdi.lastTick = curTick;
		SendInsertTextMessage(g_console.hwnd, FALSE, "\r\n> ...");
		if(setId == SET_PROG_DEFAULT) pdi.width = snprintf(NULL, 0, "%d", qwCnt);
		
	} else pdi.count += qwCnt;
	
	if(pdi.count > pdi.total || (pdi.count < pdi.total && curTick - pdi.lastTick < PROGRESS_INTERVAL)) return;
	
	pdi.lastTick = curTick;
	
	if(pdi.type == SET_PROG_PERCENT) {
		SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\r> %3llu%%...", (pdi.total > 0 ? (pdi.count * 100) / pdi.total : 0)));
		
	} else SendInsertTextMessage(g_console.hwnd, FALSE, VAStr("\r> %*llu/%llu...", pdi.width, pdi.count, pdi.total));
	
	return;
}



void ShutdownArchiverThread() {
	
	if(s_arcThread.hThread == NULL) return;
	
	s_arcThread.bThrdStop = TRUE;
	
	WaitForSingleObject(s_arcThread.hThread, INFINITE);
	CloseHandle(s_arcThread.hThread);
	
	ZeroMemory(&s_arcThread, sizeof(ARCHIVERTHREAD));
	
	return;
}

