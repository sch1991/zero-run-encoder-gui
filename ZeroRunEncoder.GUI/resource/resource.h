#ifndef RESOURCE_H
#define RESOURCE_H

/* App icon, menu and accelerator ids */
#define IDI_APP_ICON				 1000
#define IDR_MAIN_MENU				 1001
#define IDR_ACCELERATOR				 1002
#define ID_ACC_SHOW_HELP			 1100
#define ID_ACC_NEXT_MODE			 1101
#define ID_ACC_SAVE_SETTINGS		 1102
#define ID_ACC_RESET_SETTTINGS		 1103
#define ID_ACC_CLEAR_CONSOLE		 1104
#define ID_ACC_SHOW_ARCHIVER		 1105
#define ID_ACC_LOG_RECORD			 1106
#define ID_ACC_DEBUG_LOGGING		 1107
#define ID_ACC_OPEN_LOGS			 1108
/* Main window command and control ids */
#define ID_ENCODING_MODE			 1200
#define ID_DECODING_MODE			 1201
#define ID_DETAILS_MODE				 1202
#define ID_RUN_BUTTON				 1203
#define ID_STOP_BUTTON				 1204
#define ID_CONFIG_BUTTON			 1205
#define ID_CLOSE_BUTTON				 1206
#define ID_SET_INPUT_PATH			 1207
#define ID_SET_OUTPUT_PATH			 1208
#define IDC_LABEL_INPUT_PATH		 1300
#define IDC_LABEL_OUTPUT_PATH		 1301
#define IDC_EDIT_INPUT_PATH			 1302
#define IDC_EDIT_OUTPUT_PATH		 1303
#define IDC_EDIT_CONSOLE_OUTPUT		 1304

/* Config dialog ids */
#define IDD_CONFIG_DIALOG			 2000
#define ID_SAVE_BUTTON				 2100
#define ID_CANCEL_BUTTON			 2101
#define ID_SET_ENCODER_PATH			 2102
#define IDC_EDIT_ENCODER_PATH		 2200
#define IDC_EDIT_ARC_BUF_SIZE		 2201
#define IDC_CHK_USE_SAVE_LAST		 2202
#define IDC_LABEL_ENC_CNT			 2300
#define IDC_CMB_ENC_CNT				 2301
#define IDC_LABEL_ENC_BUF_SIZE		 2302
#define IDC_EDIT_ENC_BUF_SIZE		 2303
#define IDC_LABEL_ENC_BUF_UNIT		 2304
#define IDC_LABEL_CHUNK_SIZE		 2305
#define IDC_CMB_CHUNK_SIZE			 2306
#define IDC_LABEL_CHUNK_UNIT		 2307
#define IDC_LABEL_CHUNK_OFFSET		 2308
#define IDC_CMB_CHUNK_OFFSET		 2309
#define IDC_CHK_USE_MAP_BIN			 2310
#define IDC_CHK_USE_CONV_BIN		 2311
#define IDC_CHK_USE_FAT_LIMIT		 2312
/* Config dialog macros, modify this when encoder cofig is added */
#define FIRST_ENC_CONFIG_IDC		 2300
#define LAST_ENC_CONFIG_IDC			 2312
#define MAX_ENC_CONFIG_CNT			 7

/* Archiver dialog ids */
#define IDD_ARCHIVER_DIALOG			 3000
#define ID_ARC_CREATE_MODE			 3100
#define ID_ARC_EXTRACT_MODE			 3101
#define ID_ARC_RUN_BUTTON			 3102
#define ID_ARC_CLOSE_BUTTON			 3103
#define ID_ARC_SET_INPUT_PATH		 3104
#define IDC_LABEL_ARC_INPUT_PATH	 3105
#define IDC_LABEL_ARC_OUTPUT_PATH	 3106
#define IDC_EDIT_ARC_INPUT_PATH		 3107
#define IDC_EDIT_ARC_OUTPUT_PATH	 3108

/* Help dialog ids */
#define IDD_HELP_DIALOG				 4000
#define IDC_EDIT_HELP_CONTENT		 4100

#define IDC_STATIC					 -1

#endif
