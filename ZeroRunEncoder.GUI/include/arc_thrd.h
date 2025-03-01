#ifndef ARC_THRD_H
#define ARC_THRD_H

#include "common.h"

#define ARCHIVE_EXT		 ".arc"
#define ARCHIVE_HEAD	 (DWORD)0x40657230 /* '0re@' (ASCII & Little Endian) */

#define MAX_DUP_INDEX	 999
#define ARC_BUF_OFFSET	 20 /* MiB */

#define ARC_OPER_CREATE		 0x01
#define ARC_OPER_EXTRACT	 0x02
#define MAX_ARC_MODE_COUNT	 2

#define PROGRESS_INTERVAL	 500 /* millisecond */
#define ADD_PROG_COUNT		 0x00
#define SET_PROG_DEFAULT	 0x01
#define SET_PROG_PERCENT	 0x02

typedef struct tagARCHIVERMODEINFO {
	const int modeId;
	const int operId;
} ARCHIVERMODEINFO;

extern ARCHIVERMODEINFO g_arcModeInfo[MAX_ARC_MODE_COUNT];

typedef struct _tagARCHIVERTHREAD {
	HANDLE hThread;
	volatile BOOL bThrdStop;
} ARCHIVERTHREAD;

typedef struct _tagARCHIVERTHRDPARAM {
	int operId;
	char szInputPath[MAX_PATH_BYTES];
} ARCHIVERTHRDPARAM;

typedef struct _tagPROGRESSDRAWINFO {
	int type;
	int width;
	DWORD64 count;
	DWORD64 total;
	ULONGLONG lastTick;
} PROGRESSDRAWINFO;

/* Archiver thread creation function: validates lpPath for existence and validity, creates an ARCHIVERTHRDPARAM instance for the thread, 
 * and returns TRUE on success, FALSE on failure */
BOOL CreateArchiverThread(int operId, LPCSTR lpPath);

/* Archiver thread procedure: lpParam is an ARCHIVERTHRDPARAM instance; if operId is ARC_OPER_CREATE, calls RunArchiving(), if ARC_OPER_EXTRACT, calls RunArchiveExtraction(); 
 * if the operation completes without being stopped, posts STOP_THREAD_CALL with ID_STOP_BUTTON to the main window */
static DWORD WINAPI ArchiverThreadProc(LPVOID lpParam);

/* Runs directory archiving: if pStopRunFlag is not NULL, uses its value as the stop flag; verifies lpInDirPath is a directory and constructs an output path ("<directory name>.arc"), 
 * runs archiving, and if lpOutPathBuf is not NULL, copies the output path into it, and returns TRUE on success or deletes the incomplete file and returns FALSE on failure */
BOOL RunArchiving(LPCSTR lpInDirPath, volatile BOOL* pStopRunFlag, LPSTR lpOutPathBuf, size_t cbOutPathBuf);

/* Runs extraction of an '*.arc' file: if pStopRunFlag is not NULL, uses its value as the stop flag; verifies lpInFilePath by checking its extension and file signature; 
 * extracts contents into "lpInFilePath\<file name>(if duplicates occur, adds a suffix and calls MergeDir() after extraction)\"; if bDelInputFile is TRUE, deletes lpInFilePath after extraction; 
 * returns TRUE on success or deletes the incomplete directory and returns FALSE on failure */
BOOL RunArchiveExtraction(LPCSTR lpInFilePath, volatile BOOL* pStopRunFlag, BOOL bDelInputFile);

/* Recursively writes an archive file from lpInDirPath: for files, writes name, attributes, and data; for directories, writes name and attributes then recurses; 
 * after processing all contents of a directory, appends '\0' to mark the end for hierarchical reconstruction; returns TRUE on success, FALSE on failure or if stopped */
static BOOL WriteArchiveFile(LPCSTR lpInDirPath, HANDLE hArchive, LPSTR lpBuffer, size_t cbBuffer);

/* Recursively extracts files and directories from an archive file to lpOutDirPath; if the extracted content is a directory, calls itself recursively; 
 * extraction is considered complete when an empty file name ('\0') is encountered; returns TRUE on success, FALSE if stopped or on failure */
static BOOL ExtractArchiveFile(HANDLE hArchive, LPCSTR lpOutDirPath, LPSTR lpBuffer, size_t cbBuffer);

/* Recursively counts files and directories in lpInDirPath, updating *pFileCnt and *pDirCnt; if an item is a directory, calls itself recursively; 
 * returns TRUE on success, FALSE if stopped or on failure */
static BOOL CountDirContents(DWORD* pFileCnt, DWORD* pDirCnt, LPCSTR lpInDirPath);

/* Merges lpSrcDir into lpDestDir: moves all contents, deletes lpSrcDir on success and returns TRUE; returns FALSE on error or if merge is canceled */
static BOOL MergeDir(LPCSTR lpSrcDir, LPCSTR lpDestDir);

/* Force deletes lpDirPath without showing a confirmation or progress window; returns TRUE on success, FALSE on error */
static BOOL DeleteDir(LPCSTR lpDirPath);

/* Displays archiver progress on the console: if setId is SET_PROG_DEFAULT, initializes display as "count/total..."; 
 * if setId is SET_PROG_PERCENT, initializes display as "n%..." with qwCnt as total; if setId is ADD_PROG_COUNT, increments the count by qwCnt */
static void DrawProgress(int setId, DWORD64 qwCnt);

/* Sets the archiver thread shutdown flag to TRUE, terminating the thread and cleaning up resources */
void ShutdownArchiverThread();

#endif
