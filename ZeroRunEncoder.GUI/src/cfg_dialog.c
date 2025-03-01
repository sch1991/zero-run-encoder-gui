#include "cfg_dialog.h"

ENCODERCONFIG s_encConfig[MAX_ENC_CONFIG_CNT];
BOOL s_isEncConfigVisible;

INT_PTR CALLBACK ConfigDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	switch(msg) {
		case WM_INITDIALOG: {
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)g_hIconApp);
			
			HWND hControl;
			int cmbIdx;
			char cmbItem[3];
			
			ZeroMemory(s_encConfig, sizeof(s_encConfig));
			s_isEncConfigVisible = TRUE;
			
			hControl = GetDlgItem(hwndDlg, IDC_EDIT_ARC_BUF_SIZE);
			SendMessage(hControl, EM_LIMITTEXT, 2, 0);
			
			hControl = GetDlgItem(hwndDlg, IDC_CMB_ENC_CNT);
			for(cmbIdx=0 ; cmbIdx<=3 ; cmbIdx++) SendMessage(hControl, CB_ADDSTRING, 0, (LPARAM)_itoa(cmbIdx, cmbItem, 10));
			
			hControl = GetDlgItem(hwndDlg, IDC_EDIT_ENC_BUF_SIZE);
			SendMessage(hControl, EM_LIMITTEXT, 5, 0);
			
			hControl = GetDlgItem(hwndDlg, IDC_CMB_CHUNK_SIZE);
			for(cmbIdx=1 ; cmbIdx<=32 ; cmbIdx++) SendMessage(hControl, CB_ADDSTRING, 0, (LPARAM)_itoa(cmbIdx, cmbItem, 10));
			
			hControl = GetDlgItem(hwndDlg, IDC_CMB_CHUNK_OFFSET);
			for(cmbIdx=0 ; cmbIdx<=7 ; cmbIdx++) SendMessage(hControl, CB_ADDSTRING, 0, (LPARAM)_itoa(cmbIdx, cmbItem, 10));
			
			/* Add codes here when controls requiring init are added */
			
			SetWindowTextWithCaretEnd(GetDlgItem(hwndDlg, IDC_EDIT_ENCODER_PATH), g_appConfig.encoderPath);
			SetDlgItemInt(hwndDlg, IDC_EDIT_ARC_BUF_SIZE, g_appConfig.archiverBufferSize, FALSE);
			if(g_appConfig.useSaveLastSettings == TRUE)	CheckDlgButton(hwndDlg, IDC_CHK_USE_SAVE_LAST, BST_CHECKED);
			/* Add code here when app config is added */
			
			InitializeEncoderConfig(hwndDlg);
			
			DragAcceptFiles(hwndDlg, TRUE);
			
			PostMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, IDC_STATIC), TRUE);
			
			return TRUE;
		}
		
		case WM_CTLCOLORSTATIC: {
			if((HWND)lParam == GetDlgItem(hwndDlg, IDC_EDIT_ENCODER_PATH)) {
				HDC hdcStatic = (HDC)wParam;
				SetBkColor(hdcStatic, RGB(255, 255, 255));
				return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
			}
			break;
		}
			
		case WM_LBUTTONDOWN: {
			PostMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, IDC_STATIC), TRUE);
			return TRUE;
		}
		
		case WM_DROPFILES: {
			char szFilePath[MAX_PATH_BYTES];
			
			HDROP hDrop = (HDROP)wParam;
			HWND hTarget = GetDlgItem(hwndDlg, IDC_EDIT_ENCODER_PATH);
			
			DragQueryFile(hDrop, 0, szFilePath, sizeof(szFilePath));
			DWORD dwFileAttr = GetFileAttributes(szFilePath);
			
			if(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) {
				SetWindowTextWithCaretEnd(hTarget, szFilePath);
				InitializeEncoderConfig(hwndDlg);
			}
			
			DragFinish(hDrop);
			return TRUE;
		}
		
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case ID_SET_ENCODER_PATH: {
					int selectResult = SelectFolderPath(hwndDlg, "Select Encoder Folder", BIF_USENEWUI, IDC_EDIT_ENCODER_PATH);
					if(selectResult == 0) {
						InitializeEncoderConfig(hwndDlg);
					} else if(selectResult > 0) MessageBox(hwndDlg, "Invalid Folder Path!", "Warning", MB_ICONEXCLAMATION | MB_OK);
					
					PostMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, IDC_STATIC), TRUE);
					return TRUE;
				}
				
				case IDOK: 
				case ID_SAVE_BUTTON: {
					GetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_ENCODER_PATH), g_appConfig.encoderPath, sizeof(g_appConfig.encoderPath));
					
					g_appConfig.archiverBufferSize = GetDlgItemInt(hwndDlg, IDC_EDIT_ARC_BUF_SIZE, NULL, FALSE);
					if(g_appConfig.archiverBufferSize < 1) { g_appConfig.archiverBufferSize = 1;
					} else if(g_appConfig.archiverBufferSize > 24) g_appConfig.archiverBufferSize = 24;
					
					g_appConfig.useSaveLastSettings = (IsDlgButtonChecked(hwndDlg, IDC_CHK_USE_SAVE_LAST) == BST_CHECKED);
					
					/* Add code here when app config is added (Validation check needed) */
					
					SaveAppConfigFile(TRUE);
					if(s_isEncConfigVisible == TRUE) {
						if(SaveEncoderConfig(hwndDlg) == TRUE) {
							PostMessage(hwndDlg, WM_CLOSE, (WPARAM)MAKELONG(ID_SAVE_BUTTON, CFG_SAVED_APP_AND_ENC), 0);
							return TRUE;
							
						} else MessageBox(hwndDlg, "Encoder Config File Saving Failed!", "Warning", MB_ICONEXCLAMATION | MB_OK);
					}
					PostMessage(hwndDlg, WM_CLOSE, (WPARAM)MAKELONG(ID_SAVE_BUTTON, CFG_SAVED_APP_ONLY), 0);
					return TRUE;
				}
				
				case IDCANCEL: 
				case ID_CANCEL_BUTTON: {
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



void LoadAppConfigFile() {
	
	char szValue[CFG_MAX_VALUE];
	
	ZeroMemory(&g_appConfig, sizeof(APPCONFIG));
	
	GetPrivateProfileString("Config", "ENCODER_PATH", "", szValue, sizeof(szValue), CONFIG_PATH);
	DWORD dwFileAttr = GetFileAttributes(szValue);
	if(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) {
		strncpy(g_appConfig.encoderPath, szValue, sizeof(g_appConfig.encoderPath)-1);
	}
	
	g_appConfig.archiverBufferSize = GetPrivateProfileInt("Config", "ARCHIVER_BUFFER_SIZE", 1, CONFIG_PATH);
	if(g_appConfig.archiverBufferSize < 1) { g_appConfig.archiverBufferSize = 1;
	} else if(g_appConfig.archiverBufferSize > 24) g_appConfig.archiverBufferSize = 24;
	
	GetPrivateProfileString("Config", "USE_SAVE_LAST_SETTINGS", "1", szValue, 2, CONFIG_PATH);
	g_appConfig.useSaveLastSettings = (szValue[0]=='1' ? TRUE : FALSE);
	
	GetPrivateProfileString("Config", "USE_LOG_RECORD", "1", szValue, 2, CONFIG_PATH);
	g_appConfig.useLogRecord = (szValue[0]=='1' ? TRUE : FALSE);
	
	GetPrivateProfileString("Config", "USE_DEBUG_LOGGING", "0", szValue, 2, CONFIG_PATH);
	g_appConfig.useDebugLogging = (szValue[0]=='1' ? TRUE : FALSE);
	
	/* Add code here when app config is added (Validation check needed) */
	
	EnqueueLogMessage(FALSE, "INFO", "LoadAppConfigFile", "App config loaded successfully.");
	
	ZeroMemory(&g_appSettings, sizeof(APPSETTINGS));
	
	g_appSettings.modeIndex = GetPrivateProfileInt("Settings", "MODE_INDEX", 0, CONFIG_PATH);
	if(g_appSettings.modeIndex > 2) g_appSettings.modeIndex = 0;
	
	GetPrivateProfileString("Settings", "INPUT_PATH", "", g_appSettings.inputPath, sizeof(szValue), CONFIG_PATH);
	GetPrivateProfileString("Settings", "OUTPUT_PATH", "", g_appSettings.outputPath, sizeof(szValue), CONFIG_PATH);
	
	/* Add code here when app setting is added (Validation check needed) */
	
	if(PathFileExists(CONFIG_PATH) == FALSE) {
		SaveAppConfigFile(TRUE);
		EnqueueLogMessage(FALSE, "INFO", "LoadAppConfigFile", "App config file created. CONFIG_PATH=" CONFIG_PATH);
	}
	
	return;
}



void SaveAppConfigFile(BOOL bSaveConfig) {
	
	char szValue[3];
	
	if(bSaveConfig == TRUE) {
		WritePrivateProfileString("Config", "ENCODER_PATH", g_appConfig.encoderPath, CONFIG_PATH);
		WritePrivateProfileString("Config", "ARCHIVER_BUFFER_SIZE", _itoa(g_appConfig.archiverBufferSize, szValue, 10), CONFIG_PATH);
		WritePrivateProfileString("Config", "USE_SAVE_LAST_SETTINGS", (g_appConfig.useSaveLastSettings==TRUE ? "1" : "0"), CONFIG_PATH);
		WritePrivateProfileString("Config", "USE_LOG_RECORD", (g_appConfig.useLogRecord==TRUE ? "1" : "0"), CONFIG_PATH);
		WritePrivateProfileString("Config", "USE_DEBUG_LOGGING", (g_appConfig.useDebugLogging==TRUE ? "1" : "0"), CONFIG_PATH);
		/* Add code here when app config is added */
		
		EnqueueLogMessage(FALSE, "INFO", "SaveAppConfigFile", "App config saved successfully.");
		
	} else {
		WritePrivateProfileString("Settings", "MODE_INDEX", _itoa(g_appSettings.modeIndex, szValue, 10), CONFIG_PATH);
		WritePrivateProfileString("Settings", "INPUT_PATH", g_appSettings.inputPath, CONFIG_PATH);
		WritePrivateProfileString("Settings", "OUTPUT_PATH", g_appSettings.outputPath, CONFIG_PATH);
		/* Add code here when app setting is added */
	}
	
	return;
}



void ChangeAppConfigFileValue(int cfgId) {
	
	switch(cfgId) {
		case CFG_USE_LOG_RECORD: {
			if(g_appConfig.useLogRecord == TRUE) {
				EnqueueLogMessage(FALSE, "INFO", "ChangeAppConfigFileValue", VAStr("App config \'USE_LOG_RECORD\' change requested. USE_LOG_RECORD=%d->%d", g_appConfig.useLogRecord, !g_appConfig.useLogRecord));
				g_appConfig.useLogRecord = FALSE;
				
			} else {
				g_appConfig.useLogRecord = TRUE;
				EnqueueLogMessage(FALSE, "INFO", "ChangeAppConfigFileValue", VAStr("App config \'USE_LOG_RECORD\' changed. USE_LOG_RECORD=%d->%d", !g_appConfig.useLogRecord, g_appConfig.useLogRecord));
			}
			
			WritePrivateProfileString("Config", "USE_LOG_RECORD", (g_appConfig.useLogRecord==TRUE ? "1" : "0"), CONFIG_PATH);
			break;
		}

		case CFG_USE_DEBUG_LOGGING: {
			g_appConfig.useDebugLogging = !g_appConfig.useDebugLogging;
			WritePrivateProfileString("Config", "USE_DEBUG_LOGGING", (g_appConfig.useDebugLogging==TRUE ? "1" : "0"), CONFIG_PATH);
			EnqueueLogMessage(FALSE, "INFO", "ChangeAppConfigFileValue", VAStr("App config \'USE_DEBUG_LOGGING\' changed. USE_DEBUG_LOGGING=%d->%d", !g_appConfig.useDebugLogging, g_appConfig.useDebugLogging));
			break;
		}
	}
	
	return;
}



static inline void InitializeEncoderConfig(HWND hwndDlg) {
	
	char szConfigPath[MAX_PATH_BYTES];
	DWORD configIdc;
	
	int encPathLen = GetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_ENCODER_PATH), szConfigPath, sizeof(szConfigPath));
	strncat(szConfigPath, "\\" ENC_CONFIG_NAME, sizeof(szConfigPath) - encPathLen - 1);
	
	if(LoadEncoderConfig(hwndDlg, szConfigPath) != FALSE) {
		
		int entryIdx;
		
		entryIdx = 0;
		SetWindowText(GetDlgItem(hwndDlg, IDC_LABEL_ENC_CNT), s_encConfig[entryIdx].entry);
		SendMessage(GetDlgItem(hwndDlg, IDC_CMB_ENC_CNT), CB_SELECTSTRING, (WPARAM)-1, (LPARAM)s_encConfig[entryIdx].value);
		
		entryIdx = 1;
		SetWindowText(GetDlgItem(hwndDlg, IDC_LABEL_ENC_BUF_SIZE), s_encConfig[entryIdx].entry);
		SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_ENC_BUF_SIZE), s_encConfig[entryIdx].value);
		
		entryIdx = 2;
		SetWindowText(GetDlgItem(hwndDlg, IDC_LABEL_CHUNK_SIZE), s_encConfig[entryIdx].entry);
		SendMessage(GetDlgItem(hwndDlg, IDC_CMB_CHUNK_SIZE), CB_SELECTSTRING, (WPARAM)-1, (LPARAM)s_encConfig[entryIdx].value);
		
		entryIdx = 3;
		SetWindowText(GetDlgItem(hwndDlg, IDC_LABEL_CHUNK_OFFSET), s_encConfig[entryIdx].entry);
		SendMessage(GetDlgItem(hwndDlg, IDC_CMB_CHUNK_OFFSET), CB_SELECTSTRING, (WPARAM)-1, (LPARAM)s_encConfig[entryIdx].value);
		
		entryIdx = 4;
		SetWindowText(GetDlgItem(hwndDlg, IDC_CHK_USE_MAP_BIN), s_encConfig[entryIdx].entry);
		if(strcmp(s_encConfig[entryIdx].value, "true") == 0) CheckDlgButton(hwndDlg, IDC_CHK_USE_MAP_BIN, BST_CHECKED);
		
		entryIdx = 5;
		SetWindowText(GetDlgItem(hwndDlg, IDC_CHK_USE_CONV_BIN), s_encConfig[entryIdx].entry);
		if(strcmp(s_encConfig[entryIdx].value, "true") == 0) CheckDlgButton(hwndDlg, IDC_CHK_USE_CONV_BIN, BST_CHECKED);
		
		entryIdx = 6;
		SetWindowText(GetDlgItem(hwndDlg, IDC_CHK_USE_FAT_LIMIT), s_encConfig[entryIdx].entry);
		if(strcmp(s_encConfig[entryIdx].value, "true") == 0) CheckDlgButton(hwndDlg, IDC_CHK_USE_FAT_LIMIT, BST_CHECKED);
		
		/* Add code here when encoder config is added */
		
		InvalidateRect(hwndDlg, NULL, FALSE);
		
		if(s_isEncConfigVisible == FALSE) {
			for(configIdc=FIRST_ENC_CONFIG_IDC ; configIdc<=LAST_ENC_CONFIG_IDC ; configIdc++) {
				ShowWindow(GetDlgItem(hwndDlg, configIdc), SW_SHOW);
			}
			s_isEncConfigVisible = TRUE;
		}
		
	} else {
		if(s_isEncConfigVisible == TRUE) {
			for(configIdc=FIRST_ENC_CONFIG_IDC ; configIdc<=LAST_ENC_CONFIG_IDC ; configIdc++) {
				ShowWindow(GetDlgItem(hwndDlg, configIdc), SW_HIDE);
			}
			s_isEncConfigVisible = FALSE;
		}
	}
	
	return;
}



static inline BOOL LoadEncoderConfig(HWND hwndDlg, LPCSTR lpConfigPath) {
	
	FILE* pConfigFile;
	errno_t err;
	
	int entryIdx = 0;
	BOOL isEncConfig = FALSE;
	
	if((err = fopen_s(&pConfigFile, lpConfigPath, "r")) != 0) {
		EnqueueLogMessage(FALSE, "WARNING", "LoadEncoderConfig", VAStr("Failed to open encoder config file. errno=%d, lpConfigPath=\"%s\"", err, lpConfigPath));
		MessageBox(hwndDlg, VAStr("Encoder Config File Opening Failed!\n\n" "Encoder Config File Path: \n\"%s\"", lpConfigPath), "Warning", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	
	while(entryIdx < MAX_ENC_CONFIG_CNT) {
		if(fgets(s_encConfig[entryIdx].entry, sizeof(s_encConfig[entryIdx].entry), pConfigFile) != NULL) {
			s_encConfig[entryIdx].value = strchr(s_encConfig[entryIdx].entry, '=');
			if(s_encConfig[entryIdx].value != NULL) {
				*(s_encConfig[entryIdx].value++) = '\0';
				s_encConfig[entryIdx].value[strcspn(s_encConfig[entryIdx].value, "\r\n")] = '\0';
				
				if(isEncConfig == FALSE) {
					isEncConfig = (strstr(s_encConfig[entryIdx].entry, ENC_PRIMARY_KEY) != NULL);
				}
				
				entryIdx++;
				
			} else continue;
			
		} else {
			EnqueueLogMessage(FALSE, "WARNING", "LoadEncoderConfig", VAStr("Missing config in encoder config file. entryIdx=%d, MAX_ENC_CONFIG_CNT=%d, lpConfigPath=\"%s\"", entryIdx, MAX_ENC_CONFIG_CNT, lpConfigPath));
			MessageBox(hwndDlg, VAStr("Encoder Config Loading Failed!\n" "[Missing config in encoder config file.]\n" "Loaded Config: %d\n" "Required Config: %d\n\n" "Encoder Config File Path: \n\"%s\"", entryIdx, MAX_ENC_CONFIG_CNT, lpConfigPath), 
				"Warning", MB_ICONEXCLAMATION | MB_OK);
			fclose(pConfigFile);
			
			return FALSE;
		}
	}
	
	fclose(pConfigFile);
	
	if(isEncConfig == FALSE) {
		EnqueueLogMessage(FALSE, "WARNING", "LoadEncoderConfig", VAStr("Key \'" ENC_PRIMARY_KEY "\' not found in encoder config file. lpConfigPath=\"%s\"", lpConfigPath));
		MessageBox(hwndDlg, VAStr("Encoder Config Loading Failed!\n" "[Key \'" ENC_PRIMARY_KEY "\' not found in encoder config file.]\n\n" "Encoder Config File Path:\n\"%s\"", lpConfigPath), "Warning", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	
	EnqueueLogMessage(FALSE, "INFO", "LoadEncoderConfig", "Encoder config loaded successfully.");
	
	return TRUE;
}



static inline BOOL SaveEncoderConfig(HWND hwndDlg) {
	
	int configIdc, entryIdx;
	char szConfigPath[MAX_PATH_BYTES];
	
	for(configIdc=FIRST_ENC_CONFIG_IDC ; configIdc<=LAST_ENC_CONFIG_IDC ; configIdc++) {
		if(IsWindowVisible(GetDlgItem(hwndDlg, configIdc)) == FALSE) {
			EnqueueLogMessage(FALSE, "ERROR", "SaveEncoderConfig", "Encoder config not loaded.");
			return FALSE;
		}
	}
	
	for(entryIdx=0 ; (strstr(s_encConfig[entryIdx++].entry, ENC_PRIMARY_KEY) != NULL) ; ) {
		if(entryIdx == MAX_ENC_CONFIG_CNT) {
			EnqueueLogMessage(FALSE, "ERROR", "SaveEncoderConfig", "Invalid encoder config key loaded.");
			return FALSE;
		}
	}
	
	GetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_ENCODER_PATH), szConfigPath, sizeof(szConfigPath));
	DWORD dwFileAttr = MakePathBufAndGetFileAttr(szConfigPath, ENC_CONFIG_NAME, sizeof(szConfigPath));
	
	if(dwFileAttr != INVALID_FILE_ATTRIBUTES && !(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY)) {
		
		int maxValue, iValue;
		
		entryIdx = 0;
		if((maxValue = ENC_MAX_VALUE(s_encConfig[entryIdx].entry)) < 2) return FALSE;
		GetDlgItemText(hwndDlg, IDC_CMB_ENC_CNT, s_encConfig[entryIdx].value, maxValue);
		
		entryIdx = 1;
		if((maxValue = ENC_MAX_VALUE(s_encConfig[entryIdx].entry)) < 6) return FALSE;
		GetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_ENC_BUF_SIZE), s_encConfig[entryIdx].value, maxValue);
		if((iValue = atoi(s_encConfig[entryIdx].value)) < 64) {
			StringCbPrintf(s_encConfig[entryIdx].value, maxValue, "%d", 64);
		} else if(iValue > 24576) StringCbPrintf(s_encConfig[entryIdx].value, maxValue, "%d", 24576);
		
		entryIdx = 2;
		if((maxValue = ENC_MAX_VALUE(s_encConfig[entryIdx].entry)) < 3) return FALSE;
		GetDlgItemText(hwndDlg, IDC_CMB_CHUNK_SIZE, s_encConfig[entryIdx].value, maxValue);
		
		entryIdx = 3;
		if((maxValue = ENC_MAX_VALUE(s_encConfig[entryIdx].entry)) < 2) return FALSE;
		GetDlgItemText(hwndDlg, IDC_CMB_CHUNK_OFFSET, s_encConfig[entryIdx].value, maxValue);
		
		entryIdx = 4;
		if((maxValue = ENC_MAX_VALUE(s_encConfig[entryIdx].entry)) < 6) return FALSE;
		if(IsDlgButtonChecked(hwndDlg, IDC_CHK_USE_MAP_BIN) == BST_CHECKED) {
			strncpy(s_encConfig[entryIdx].value, "true", maxValue);
		} else strncpy(s_encConfig[entryIdx].value, "false", maxValue);
		
		entryIdx = 5;
		if((maxValue = ENC_MAX_VALUE(s_encConfig[entryIdx].entry)) < 6) return FALSE;
		if(IsDlgButtonChecked(hwndDlg, IDC_CHK_USE_CONV_BIN) == BST_CHECKED) {
			strncpy(s_encConfig[entryIdx].value, "true", maxValue);
		} else strncpy(s_encConfig[entryIdx].value, "false", maxValue);
		
		entryIdx = 6;
		if((maxValue = ENC_MAX_VALUE(s_encConfig[entryIdx].entry)) < 6) return FALSE;
		if(IsDlgButtonChecked(hwndDlg, IDC_CHK_USE_FAT_LIMIT) == BST_CHECKED) {
			strncpy(s_encConfig[entryIdx].value, "true", maxValue);
		} else strncpy(s_encConfig[entryIdx].value, "false", maxValue);
		
		/* Add code here when encoder config is added (Validation check needed) */
		
		for(entryIdx=0 ; entryIdx<MAX_ENC_CONFIG_CNT ; entryIdx++) {
			WritePrivateProfileString("Config", s_encConfig[entryIdx].entry, s_encConfig[entryIdx].value, szConfigPath);
		}
		
	} else {
		EnqueueLogMessage(FALSE, "ERROR", "SaveEncoderConfig", VAStr("Encoder config file not found. szConfigPath=\"%s\"", szConfigPath));
		return FALSE;
	}
	
	EnqueueLogMessage(FALSE, "INFO", "SaveEncoderConfig", "Encoder config file saved successfully.");
	
	return TRUE;
}
