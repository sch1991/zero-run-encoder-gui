#ifndef CFG_DIALOG_H
#define CFG_DIALOG_H

#include "common.h"

#define CONFIG_PATH			 "." "\\" "Config.ini"
#define CFG_MAX_VALUE		 MAX_PATH_BYTES

#define ENC_MAX_CONFIG		 1024
#define ENC_CONFIG_NAME		 "config.ini"
#define ENC_PRIMARY_KEY		 "ENCODING_COUNT"
#define ENC_MAX_VALUE(key)	 (sizeof(key) - strlen(key) - 1)

#define CFG_SAVED_APP_ONLY		 0x10
#define CFG_SAVED_APP_AND_ENC	 0x11

#define CFG_USE_LOG_RECORD		 0x01
#define CFG_USE_DEBUG_LOGGING	 0x02


typedef struct _tagENCODERCONFIG {
	char entry[ENC_MAX_CONFIG];
	LPSTR value;
} ENCODERCONFIG;

/* Config dialog procedure: handles config dialog messages; when the save button is pressed, 
 * returns ID_SAVE_BUTTON with CFG_SAVED_APP_AND_ENC (if encoder config is loaded) or CFG_SAVED_APP_ONLY */
INT_PTR CALLBACK ConfigDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/* Loads 'Config.ini' and initializes g_appConfig and g_appSettings (mode index, input path, output path) */
void LoadAppConfigFile();

/* Saves the current g_appConfig or g_appSettings to 'Config.ini', determined by bSaveConfig (only one is saved) */
void SaveAppConfigFile(BOOL bSaveConfig);

/* Updates a config value in 'Config.ini' that is not shown in the config dialog (CFG_USE_LOG_RECORD, CFG_USE_DEBUG_LOGGING) */
void ChangeAppConfigFileValue(int cfgId);

/* Loads encoder config values and displays them in the config dialog if successful; hides encoder config items if loading fails */
static inline void InitializeEncoderConfig(HWND hwndDlg);

/* Loads the encoder's 'config.ini' from lpConfigPath; if successful, initializes s_encConfig; returns TRUE on success, FALSE on failure */
static inline BOOL LoadEncoderConfig(HWND hwndDlg, LPCSTR lpConfigPath);

/* Saves the values from the config dialog to the 'config.ini' at g_appConfig.encoderPath; returns TRUE on success, FALSE on failure */
static inline BOOL SaveEncoderConfig(HWND hwndDlg);

#endif
