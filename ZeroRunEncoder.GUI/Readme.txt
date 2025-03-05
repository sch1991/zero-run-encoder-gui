[USER]
# Program Name : ZeroRunEncoder.GUI

# Description : 
- Conditional probability-based conversion + Exponential-golomb coding.
- The program converts file data to include as many zero-runs as possible and encodes them.

# Usage : 
- Initial Setup : On first run, press the 'Config' button and set the "Encoder path" to the folder path of "ZeroRunEncoder.0re" (This folder is included by default in the program's directory).
- Operation : 
        0. (For folder archiving without encoding, select Tools > Archiver)
        1. In the main window, select a mode.
        2. Set the paths. (Drag-and-drop is recommended)
        3. Click the 'Run' button.
- File Association : 
        1. Right-click the '*.0re' or '*.arc' file and choose Open with.
        2. Select Add app and then Look for another app on this PC.
        3. Navigate to the program folder and choose "ZeroRunEncoder.GUI.exe".

# Hotkeys : 
- Help Show/Hide:		F1
- Next Mode : 		F2
- Set Input Path : 		F3
- Set Output Path :		F4
- Config : 			F5
- Run / Stop : 		Enter
- Close : 			Esc
- Save Settings : 		Ctrl+S
- Reset Settings : 		Ctri+Shift+R
- Clear Text Panel : 		Ctrl+Shift+C
- Log Record On/Off : 	Ctrl+Alt+L
- Debug Logging On/Off : 	Ctrl+Alt+D
- Open Logs Folder : 	Ctrl+Alt+F

# Warning : 
- This program is an ANSI application. Any Unicode characters you enter may be displayed as '?'.
- When manually typing a path, the input is limited to 259 characters and only English characters are allowed..
- For paths that contain non-English characters, please use the drag-and-drop feature.

# Auther : ESonia
[END]

[DEVELPOER]
# Environment : 
- Windows 10 x64
- Dev-C++ 5.11
- GCC 4.9.2 64-bit

# Requirements : 
- To compile this program, the following requirements must be met.
- Linker Option : -Wl,--stack,33554432 -lole32 -lshlwapi -lcomctl32
- Include File DirPath : ZeroRunEncoder.GUI\include
- Resource File DirPath : ZeroRunEncoder.GUI\resource
[END]