취업 포트폴리오 겸 개인 연구 프로젝트

# Zero Run Encoder - GUI
## 프로젝트 소개
- Zero Run Encoder 를 위한 Windows GUI App 입니다.
- Zero Run Encoder 에 대해선 [zero-run-encoder-0re](https://github.com/sch1991/zero-run-encoder-0re)를 참조해 주세요.

<br><br>

## 실행 방법
### - 초기 설정 (GUI App 에 Zero Run Encoder 연결)
![image1](https://github.com/user-attachments/assets/5aecb43b-ac14-4fe8-b0fe-69e68028c88f)

1. 메인 창에서 'Config' 버튼을 누른다.

![image2](https://github.com/user-attachments/assets/fb7419cd-0580-4d4a-ae88-5a9a7c1a19f6)
![image3](https://github.com/user-attachments/assets/f744eb79-4d7a-40a6-bd02-a854aeb36456)

2. 'GUI App Config'의 'Encoder Path'에 'ZeroRunEncoder.0re-v1.25' 폴더를 Drag-and-drop 한다.

![image4](https://github.com/user-attachments/assets/57cdb2d5-ab6e-4193-8427-9f8aec4d75c1)

3. 'GUI App Config'와 'Encoder Config'의 설정값을 조정한 후 'Save' 버튼을 누른다.
```
- Zero Run Encoder 연결 후에도 'Encoder Config'가 보이지 않는 경우
1. 'ZeroRunEncoder.0re-v1.25' 폴더 안에서 'config.ini'을 삭제한다.
2. 메인 창에서 Details 모드를 선택 후 '0re File Path'에 아무 파일의 경로를 넣고 'Run' 버튼을 누른다.
3. 'Config' 버튼을 눌러 'Encoder Config'가 보이는지 확인한다.
```

<br><br>

## 주요 기능 및 사용법
### - 도움말
- 메인 창 메뉴에서 About(A) > Help(H) 를 누르거나 키보드 F1을 눌러 확인 가능하다.
- App에 대한 소개글과 제공되는 모든 단축키, 주의 사항을 볼 수 있다.
```
<주의 사항>
- 이 프로그램은 ANSI 전용 App으로 유니코드를 사용할 경우, '?'로 표시될 수 있으며 정상 작동이 보장되지 않습니다.
- IME의 입력 언어가 영어가 아닌 상태에서 직접 경로를 입력하면 문자가 정상적으로 입력되지 않을 수 있습니다.
- 경로를 직접 입력할 경우, 입력 가능한 용량은 259 Byte로 제한됩니다. (2-byte 문자를 입력할 경우 주의가 필요합니다)
- 이러한 이유로, 가능하면 Drag-and-drop 기능으로 경로를 입력해 주시기 바랍니다.
```

<br>

### - 폴더 아카이빙
- 'Create Archive' 모드 : 입력된 폴더를 하나의 파일로 아카이브하여 '*.arc' 파일을 생성한다.
- 'Extract Archive' 모드 : 입력된 '*.arc' 파일을 추출한다.
  #### 사용법
![image5](https://github.com/user-attachments/assets/0a25ecda-0eb8-40af-9a1f-689d0cc0d7ba)
1. 메인 창 메뉴에서 Tools(T) > Archiver(A) 를 누르거나 키보드 Ctrl+Shift+A를 눌러 Archiver 창을 띄운다.
2. 폴더를 아카이브 파일로 만들려면 Create Archive 모드, 아카이브 파일을 추출하려면 Extract Archive 모드를 선택한다.
3. 각 모드에 맞게 'Input Folder Path' 혹은 'Archive File Path' 를 입력하고 'Run' 버튼을 누른다. (Output Path는 자동으로 입력된다.)

<br>

### - 파일/폴더 압축
- 'Encoding' 모드 : 입력된 파일 혹은 폴더를 부호화 및 압축 과정을 거쳐 하나의 0re 파일로 생성한다.
- 'Decoding' 모드 : 입력된 0re 파일을 복호화 및 압축 해제 과정을 거쳐 원래의 파일 혹은 폴더로 복원한다.
- 'Details' 모드 : 입력된 0re 파일의 상세 정보를 확인한다.
  #### 사용법
![image6](https://github.com/user-attachments/assets/5c8e4844-4cdf-4b3c-93c7-06bc86844dcf)
1. 작업할 모드를 선택한다.
2. 각 모드에 맞게 'Input File or Folder Path' 혹은 '0re File Path'를 입력한다.
3. 각 모드에 맞게 'Output File or Folder Path' 혹은 'Output Folder Path'를 입력한다.
4. 'Run' 버튼을 누른다.
```
Encoding 모드에서 'Output File or Folder Path'를 폴더 경로로 입력한 경우, 0re 파일 생성 경로는 "(폴더 경로)\(입력 파일 혹은 폴더 이름).0re" 로 설정된다
```

<br>

### - 프로그램에 연결하여 파일 열기
- '\*.0re' 혹은 '\*.arc' 파일을 열 때, ['추가 앱' > '이 PC에서 다른 앱 찾기' > 'ZeroRunEncoder.GUI.exe'] 로 연결하여 실행하면 프로그램이 'Decoding' 모드 혹은 'Extract Archive' 모드로 실행되며, Input & Output Path 가 자동으로 초기화됩니다.

<br><br>

## 컴파일 방법
### - 환경
- OS : Windows 10 x64
- IDE : Dev-C++ 5.11
- Compiler : TDM-GCC 4.9.2 64-bit
### - 요구 사항
- Linker Option : -Wl,--stack,33554432 -lole32 -lshlwapi -lcomctl32
- Include File DirPath : ZeroRunEncoder.GUI\include
- Resource File DirPath : ZeroRunEncoder.GUI\resource

<br><br>

## 라이선스
- 이 프로젝트는 Apache License 2.0 하에 배포됩니다. 자세한 내용은 [LICENSE](LICENSE) 파일을 참조하세요.

<br><br>

## 노트
- 모든 중단 작업은 우아한 종료를 목표로 하므로, 강제 종료는 삼가해 주시기 바랍니다.
