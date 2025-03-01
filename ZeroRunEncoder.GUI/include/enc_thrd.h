#ifndef ENC_THRD_H
#define ENC_THRD_H

#include "common.h"

#include "arc_thrd.h"

#define ENC_APP_NAME	 "ZeroRunEncoder.0re.exe"
#define ZERORE_EXT		 ".0re"

#define ENC_MAX_COMMAND_LINE	 1024
#define DEC_OUTPUT_PATH_LABEL	 "output_file_path: "

#define ENC_OPER_ENCODING		 "encoding"
#define ENC_OPER_DECODING		 "decoding"
#define ENC_OPER_DETAILS		 "details"
#define MAX_ENC_MODE_COUNT		 3

#define ENC_MODE_NEED_INPUT		 0b00000001
#define ENC_MODE_NEED_OUTPUT	 0b00000010

typedef struct tagENCODERMODEINFO {
	const int nId;
	LPCSTR lpOper;
	const DWORD reqFlag;
} ENCODERMODEINFO;

extern ENCODERMODEINFO g_encModeInfo[MAX_ENC_MODE_COUNT];

typedef struct _tagENCODERTHREAD {
	HANDLE hThread;
	volatile BOOL bThrdStop;
} ENCODERTHREAD;

typedef struct _tagENCODERTHRDPARAM {
	const ENCODERMODEINFO* pModeInfo;
	char szInputPath[MAX_PATH_BYTES];
	char szOutputPath[MAX_PATH_BYTES];
} ENCODERTHRDPARAM;

/* Encoder thread creation function: Validates lpInputPath and lpOutputPath based on pModeInfo (ensuring absolute paths, valid filenames, and existence if required), 
 * then initializes an ENCODERTHRDPARAM instance and passes it to the thread; returns TRUE on success, FALSE on failure */
BOOL CreateEncoderThread(const ENCODERMODEINFO* pModeInfo, LPCSTR lpInputPath, LPCSTR lpOutputPath);

/* Encoder thread procedure: lpParam is an ENCODERTHRDPARAM instance from CreateEncoderThread(); based on its pModeInfo, 
 * calls mode-specific processing functions with necessary paths and, upon completion, posts STOP_THREAD_CALL with ID_STOP_BUTTON to the main window */
static DWORD WINAPI EncoderThreadProc(LPVOID lpParam);

/* Checks if lpInputPath and lpOutputPath are files or directories; if lpInputPath is a directory, creates a temporary archive via RunArchiving, 
 * if lpOutputPath is a directory, constructs the output path as "lpOutputPath\<lpInputPath's file or directory name>.0re", 
 * then builds the encoder command line for ExecuteEncoderProcess(), and deletes the temporary archive upon completion */
static inline void RunEncodingMode(LPCSTR lpModeOper, LPCSTR lpInputPath, LPCSTR lpOutputPath);

/* Checks if lpInputPath and lpOutputPath are files or directories; builds the encoder command line, calls ExecuteEncoderProcess(), 
 * and if the decoding result file is an archive ('*.arc') after completion, calls RunArchiveExtraction(). */
static inline void RunDecodingMode(LPCSTR lpModeOper, LPCSTR lpInFilePath, LPCSTR lpOutDirPath);

/* Checks if lpInFilePath is a file or directory; builds the encoder command line for ExecuteEncoderProcess() and then calls it */
static inline void RunDetailMode(LPCSTR lpModeOper, LPCSTR lpInFilePath);

/* Executes the encoder process from g_appConfig.encoderPath using lpCmdLine, redirects its console output to the app's console via a pipe, 
 * and if lpDecOutBuf is not NULL, copies the decoding mode output file path; returns TRUE on success, FALSE otherwise */
static inline BOOL ExecuteEncoderProcess(LPSTR lpCmdLine, LPSTR lpDecOutBuf, size_t cbDecOutBuf);

/* Sets the encoder thread shutdown flag to TRUE to halt any running archiver or process and clean up resources */
void ShutdownEncoderThread();

#endif
