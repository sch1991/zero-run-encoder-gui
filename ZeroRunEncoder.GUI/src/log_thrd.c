#include "log_thrd.h"

LOGGERTHREAD s_logThread;
LOGMESSAGEQUEUE s_logMsgQueue;

BOOL InitializeLoggerThread() {
	
	if(PathFileExists(LOGS_DIR_PATH) == FALSE) {
		if(CreateDirectory(LOGS_DIR_PATH, NULL) == 0) {
			MessageBox(NULL, VAStr("Failed To Create Logs Folder!\n\n" "error_code: %u", GetLastError()), "Error", MB_ICONEXCLAMATION | MB_OK);
			return FALSE;
		}
	}
	
	time_t curDate = time(NULL);
	char szLogPath[sizeof(LOG_PATH_TIME_FORMAT) + 2];
	
	strftime(szLogPath, sizeof(szLogPath), LOG_PATH_TIME_FORMAT, localtime(&curDate));
	
	FILE* pLogFile;
	errno_t err;
	
	if((err = fopen_s(&pLogFile, szLogPath, "a")) != 0) {
		MessageBox(NULL, VAStr("Failed To Open Log File!\n\n" "errno: %d", err), "Error", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	
	s_logThread.hQueueFullEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(s_logThread.hQueueFullEvent == NULL) {
		MessageBox(NULL, "Failed To Create Logger Event!", "Error", MB_ICONEXCLAMATION | MB_OK);
		fclose(pLogFile);
		DestroyLoggerThread();
		return FALSE;
	}
	
	s_logThread.hQueueMutex = CreateMutex(NULL, FALSE, NULL);
	if(s_logThread.hQueueMutex == NULL) {
		MessageBox(NULL, "Failed To Create Logger Mutex!", "Error", MB_ICONEXCLAMATION | MB_OK);
		fclose(pLogFile);
		DestroyLoggerThread();
		return FALSE;
	}
	
	s_logThread.hThread = CreateThread(NULL, 0, LoggerThreadProc, (LPVOID)pLogFile, 0, NULL);
	if(s_logThread.hThread == NULL) {
		MessageBox(NULL, "Failed To Create Logger Thread!", "Error", MB_ICONEXCLAMATION | MB_OK);
		fclose(pLogFile);
		DestroyLoggerThread();
		return FALSE;
	}
	
	return TRUE;
}



static DWORD WINAPI LoggerThreadProc(LPVOID lpParam) {
	
	FILE* pLogFile = (FILE*)lpParam;
	
	EnqueueLogMessage(FALSE, "INFO", "LoggerThreadProc", "Logger thread started.");
	
	int queIdx, queCnt;
	
	do {
		WaitForSingleObject(s_logThread.hQueueFullEvent, EVENT_WAIT_TIMEOUT);
		WaitForSingleObject(s_logThread.hQueueMutex, INFINITE);
		
		queCnt = s_logMsgQueue.count;
		for(queIdx=0 ; queIdx<queCnt ; queIdx++) {
			fputs(s_logMsgQueue.msgQueue[queIdx], pLogFile);
		}
		
		s_logMsgQueue.count = 0;
		ReleaseMutex(s_logThread.hQueueMutex);
		
	} while(s_logThread.bThrdExit == FALSE);
	
	EnqueueLogMessage(FALSE, "INFO", "LoggerThreadProc", "Logger thread stopped.");
	fputs(s_logMsgQueue.msgQueue[0], pLogFile);
	
	fflush(pLogFile);
	fclose(pLogFile);
	
	ExitThread(0);
}



void EnqueueLogMessage(BOOL isDebug, LPCSTR lpLevel, LPCSTR lpSource, LPCSTR lpLogMsg) {
	
	if(g_appConfig.useLogRecord == FALSE) return;
	if(isDebug == TRUE && g_appConfig.useDebugLogging == FALSE) return;
	
	time_t now = time(NULL);
	char szTime[sizeof(LOG_MESSAGE_TIME_FORMAT) + 2];
	
	strftime(szTime, sizeof(szTime), LOG_MESSAGE_TIME_FORMAT, localtime(&now));
	
	while(TRUE) {
		WaitForSingleObject(s_logThread.hQueueMutex, INFINITE);
		if(!(s_logMsgQueue.count < LOG_QUEUE_SIZE)) {
			ReleaseMutex(s_logThread.hQueueMutex);
			Sleep(0);
		} else break;
	}
	
	int queIdx = s_logMsgQueue.count;
	
	size_t msgLen = strlen(lpLogMsg);
	if(msgLen == 0 || lpLogMsg[msgLen - 1] != '\n') {
		StringCbPrintf(s_logMsgQueue.msgQueue[queIdx], sizeof(*s_logMsgQueue.msgQueue), "%s [%s] [%s] %s\n", szTime, lpLevel, lpSource, lpLogMsg);
	} else {
		StringCbPrintf(s_logMsgQueue.msgQueue[queIdx], sizeof(*s_logMsgQueue.msgQueue), "%s [%s] [%s] %s", szTime, lpLevel, lpSource, lpLogMsg);
	}
	
	if(++s_logMsgQueue.count == LOG_QUEUE_SIZE) {
		SetEvent(s_logThread.hQueueFullEvent);
	}
	
	ReleaseMutex(s_logThread.hQueueMutex);
	
	return;
}



void DestroyLoggerThread() {
	
	if(s_logThread.hThread != NULL) {
		s_logThread.bThrdExit = TRUE;
		SetEvent(s_logThread.hQueueFullEvent);
		
		WaitForSingleObject(s_logThread.hThread, INFINITE);
		CloseHandle(s_logThread.hThread);
	}
	
	if(s_logThread.hQueueFullEvent != NULL) CloseHandle(s_logThread.hQueueFullEvent);
	if(s_logThread.hQueueMutex != NULL) CloseHandle(s_logThread.hQueueMutex);
	
	return;
}
