#ifndef LOG_THRD_H
#define LOG_THRD_H

#include "common.h"

#define EVENT_WAIT_TIMEOUT	 900000 /* millisecond */

#define LOGS_DIR_PATH			 "." "\\" "Logs"
#define LOG_PATH_TIME_FORMAT	 LOGS_DIR_PATH "\\" "ZeroRunEncoder.GUI-%Y-%m-%d.log"
#define LOG_MESSAGE_TIME_FORMAT	 "%Y-%m-%d %H:%M:%S "
#define LOG_MAX_MESSAGE			 (sizeof(LOG_MESSAGE_TIME_FORMAT) + 2 + MAX_STRING)
#define LOG_QUEUE_SIZE			 32

typedef struct _tagLOGGERTHREAD {
	HANDLE hThread;
	HANDLE hQueueFullEvent;
	HANDLE hQueueMutex;
	volatile BOOL bThrdExit;
} LOGGERTHREAD;

typedef struct _tagLOGMESSAGEQUEUE {
	char msgQueue[LOG_QUEUE_SIZE][LOG_MAX_MESSAGE];
	volatile int count;
} LOGMESSAGEQUEUE;

/* Creates today's log file and initializes the logger thread; returns TRUE on success, FALSE on failure */
BOOL InitializeLoggerThread();

/* Logger thread procedure: dequeues log messages and writes them to the log file (lpParam is the file pointer from InitializeLoggerThread()) */
static DWORD WINAPI LoggerThreadProc(LPVOID lpParam);

/* Formats a log message from lpLevel, lpSource, and lpLogMsg, and enqueues it; if isDebug is TRUE, enqueues only when debug logging is enabled */
void EnqueueLogMessage(BOOL isDebug, LPCSTR lpLevel, LPCSTR lpSource, LPCSTR lpLogMsg);

/* Terminates the logger thread and cleans up its resources */
void DestroyLoggerThread();

#endif
