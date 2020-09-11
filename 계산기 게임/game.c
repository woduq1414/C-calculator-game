/*���α׷��� ���� ������ ����
	������ ��ư�� �̿��Ͽ� ���÷��̿� �ִ� ���� ���õ� Ƚ�� �ȿ� ��ǥ ���� �ٲٸ� ������ Ŭ���� �� �� �ִ�.
	5 �������� ���� ���� ������ �����ϴµ� �ణ ��Ģ�� �ٸ���. ���� ���������� ��� ��ư�� �� ���� ����ؾ� �ϰ�, 
	��ư�� ��� ���� �� ������ HP �̻��� ���� ��������� �� Ŭ���� �� �� �ִ�.
	�� 14���� ��ư ������ �����ϰ�, 50���� ������ �����Ѵ�.
*/





/* ���α׷��� �帧
	displayMainScreen(���� ȭ��) -> displayHowto(���� ���)
								 -> displayStageSelect(���� ����) -> initStage(�ΰ���)
	initStage : initButton(��ư �ʱ�ȭ) -> Ű �Է� ���� -> ��ư ���� -> ���� Ŭ���� or ���� Ȯ��
				���� Ŭ���� �� : OK ��ư ���, ���̺����� ����...
				���� ���� �� : ���� �ʱ�ȭ...
*/





#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h> 
#include <stdlib.h> 
#include <time.h>
#include <process.h>
#include <conio.h>
#include <string.h>
#include <Windows.h>
#include<MMSystem.h>

#pragma comment(lib,"Winmm.lib")


//�Լ� ������Ÿ�� ����
void displayStageSelect(int page);
void displayMainScreen();
void displayHowto();
void initStage();
HWND getConsoleWindowHandle();

// ��ư�� ����ϴµ� �ʿ��� ���
#define MESSAGE_BTN 0
#define ARITH_BTN 1
#define FUNC_BTN 2
#define APPEND_BTN 3
#define SELECT_BTN 7
#define CLEARED_BTN 8
#define NULL_BTN 9
#define CLEAR_BTN 10

//keyCheck �Լ����� ���� ��ũ�� �Լ�
#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000 ? 1 : 0))
#define KEYUP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000 ? 0 : 1))

//����ü ����
typedef struct _button {
	int key;
	int btnType;
	char btnText[15];
	int printX;
	int printY;
	double scale;
}Button;
typedef struct _message {
	char * displayText;
	char * btnText;
	int isMultiLine;
}Message;
typedef struct _stage {
	int level;
	//Button * button_list[10];
	char * button_list;
	int startNum;
	int goalNum;
	int moves;
	int boss;
	Message message[10];
}Stage;

//���� ���� ����
Stage s[51];
Button b[8];

CRITICAL_SECTION cs; // �Ӱ� ����
HWND hForeWindow; // �ܼ� ������

long curNum;
long startNum; 
int moves; 
long goalNum; 
int level; 
int curStage = 0;
int isCleared;
int isUsed[10] = { 0 };
RECT btnRect[9];
int button_n;
int key_arr[200] = { 0 };
int is_pushed = 0;
char image_path[11][50] = {
	"image/btn_message.bmp",
	"image/btn_arith.bmp",
	"image/btn_func.bmp",
	"image/btn_append.bmp",
	"",
	"",
	"",
	"image/btn_select.bmp",
	"image/btn_cleared.bmp",
	"image/btn_null.bmp",
	"image/btn_clear.bmp"
};
char soundPath[25];


MCI_OPEN_PARMS m_mciOpenParms;
MCI_PLAY_PARMS m_mciPlayParms;
DWORD m_dwDeviceID;
MCI_OPEN_PARMS mciOpen;
MCI_PLAY_PARMS mciPlay;
int dwID;



/*������ ���̺����� ��ȣȭ*/
void encodeStr(char * str) {
	char key[55] = "ZqejpnlaoPQEpoEcmmznsldDqqNqdJapfiubllalsdlffhLXKRT";
	for (int i = 0; i <= 52; i++) {
		str[i] = str[i] ^ key[i];
	}
	return;
}

/*���̺������� ���ų� �ջ���� �� �ʱ�ȭ*/
void initRecord() {

	FILE *f3;
	f3 = fopen("savepoint/savefile", "wb");
	char temp[55] = "s0000000000000000000000000000000000000000000000000000";
	
	encodeStr(temp);

	fprintf(f3, temp);
	fclose(f3);

}

/*�������� Ŭ�������� �� ���̺����� ����ȭ*/
void saveRecord(int level) {

	FILE *f1, *f2;
	char temp[100];
	f1 = fopen("savepoint/savefile", "r+b");
	fscanf(f1, "%s", &temp);

	encodeStr(temp);

	char save[100];
	srand(time(NULL));
	save[0] = 's';
	int sum = 0;
	for (int i = 1; i <= 50; i++) {
		int n = temp[i] - 48;
		if ((n >= 1 && n <= 9) || i == level) {

			int random = rand() % 9 + 1;
			char temp2[5];
			sprintf(temp2, "%d", random);
			save[i] = temp2[0];
			sum += random * i;
		}
		else {
			save[i] = '0';
		}
	}
	sum = sum % 100;
	
	save[51] = sum / 10 + 48;
	save[52] = sum % 10 + 48;
	save[53] = NULL;


	f2 = fopen("savepoint/savefile", "wb");

	encodeStr(save);

	fprintf(f2, save);
	fclose(f1);
	fclose(f2);
}

/*Ű�� ������ �� �������� �Էµ��� �ʰ� �� ���� �Է� �޴� �Լ�*/
int keyCheck(int vk_code, int * key_arr) {

	if (KEYDOWN(vk_code) && key_arr[vk_code] == 0) {
		if (hForeWindow == GetForegroundWindow()) {
			key_arr[vk_code] = 1;
			return 1;
		}


	}
	else if (KEYUP(vk_code) && key_arr[vk_code] == 1) {
		if (hForeWindow == GetForegroundWindow()) {
			key_arr[vk_code] = 0;
		}

	}
	return 0;
}

/*�Ҹ� ���*/
void soundPlay(char * path) {
	mciOpen.lpstrElementName = path; // ���� ��� �Է�
	mciOpen.lpstrDeviceType = "waveaudio";

	mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE,
		(DWORD)(LPVOID)&mciOpen);

	dwID = mciOpen.wDeviceID;

	mciSendCommand(dwID, MCI_PLAY, MCI_NOTIFY, // play & repeat
		(DWORD)(LPVOID)&m_mciPlayParms);
	return;
}

/*�ܼ��� �ڵ��� ������*/
HWND getConsoleWindowHandle() {

	WCHAR title[2048] = { 0 };
	GetConsoleTitle(title, 2048);
	HWND hWnd = FindWindow(NULL, title);
	SetConsoleTitle(title);
	return hWnd;
}

/*��üȭ������ ��ȯ*/
void fullscreenConsole() {
	system("cls");

	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleDisplayMode(hCon, CONSOLE_FULLSCREEN_MODE, NULL);

	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(hCon, &info); // �ܼ�â�� ������ ���´�.

	COORD bufSize;

	//�ܼ�â�� �ִ� ����� �޾ƿ´�.

	bufSize.X = 1920;
	bufSize.Y = 1080;

	int ret = 0;

	//�ܼ�â�� ����� �ٲ��ش�.
	ret = SetConsoleScreenBufferSize(hCon, bufSize);

	if (ret == 0) //�����ϸ� �����Ѵ�.
		abort();
}

/*��ũ�ѹٸ� ����*/
void remove_scrollbar() {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(handle, &info);
	COORD new_size =
	{
	   info.srWindow.Right - info.srWindow.Left + 1,
	   info.srWindow.Bottom - info.srWindow.Top + 1
	};
	SetConsoleScreenBufferSize(handle, new_size);
}

/*���� ����*/
void gameInit() {
	fullscreenConsole(); // �ܼ�â�� ȭ�� ��ü�� �ٲ��ִ� �Լ�
	remove_scrollbar(); // ��ũ�ѹ� ����
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xfa); // ������ �Ͼ� ���� �ٲ�

	/*Ŀ�� �����*/
	CONSOLE_CURSOR_INFO cursorInfo = { 0, };
	cursorInfo.dwSize = 1;
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

	/*���� ���� & ���� ��� ��Ȱ��ȭ*/
	HANDLE stdIn = GetStdHandle(STD_INPUT_HANDLE);
	if (stdIn != INVALID_HANDLE_VALUE) {
		DWORD dwMode = 0;
		if (GetConsoleMode(stdIn, &dwMode)) {
			dwMode &= ~ENABLE_QUICK_EDIT_MODE;
			SetConsoleMode(stdIn, dwMode | ENABLE_EXTENDED_FLAGS);
		}
	}

	/*�ܼ� ������ ���*/
	hForeWindow = GetConsoleWindow();
}

/*�ܼ�â�� �ػ� ������*/
int GetDPI(HWND hWnd) {
	HANDLE user32 = GetModuleHandle(TEXT("user32"));
	FARPROC func = GetProcAddress(user32, "GetDpiForWindow");
	if (func == NULL)
		return 96;
	return ((UINT(__stdcall *)(HWND))func)(hWnd);
}

/*�޸� DC�� �̹����� ����*/
void GetBMP(HDC hdc, HDC memdc, HBITMAP image) {
	BITMAP bitmap;
	HDC bitmapDC = CreateCompatibleDC(hdc);

	GetObject(image, sizeof(bitmap), &bitmap);
	SelectObject(bitmapDC, image);
	BitBlt(memdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, bitmapDC, 0, 0, SRCCOPY);

	DeleteDC(bitmapDC);
}

/*Ư�� �� ������ �̹��� ���*/
void paintTransparent(char * fileName, int printX, int printY, int printW, int printH, int loadX, int loadY, int loadW, int loadH) {

	int dpi;
	//�ܼ�â�� �ڵ��� �����´�.
	HWND hWnd = getConsoleWindowHandle();

	dpi = GetDPI(hWnd); //�ܼ�â�� �ػ� ������ �����´�.

	HBITMAP image = (HBITMAP)LoadImage(NULL, fileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	HDC hdc = GetDC(hWnd);
	HDC memdc = CreateCompatibleDC(hdc);

	//�̹��������� �ֱ� ���� ������ 640*480���� �����.
	//HBITMAP bitmap = CreateCompatibleBitmap(hdc,printW,printH);
	HBITMAP bitmap = CreateCompatibleBitmap(hdc, printW, printH);
	//�ش� �̹��� ������ �޸� ������ �����Ų��.
	//SelectObject(memdc, bitmap);
	SelectObject(memdc, image);

	//image�� memdc�� ��ƿ����� �Ѵ�.
	GetBMP(hdc, memdc, image);

	GdiTransparentBlt(hdc, printX, printY, printW, printH,
		memdc, loadX, loadY, loadW, loadH,
		RGB(255, 204, 153));

	//(hdc, ��� ��ġ x, ��� ��ġ y, ����� �̹��� width, ����� �̹��� height,
	//memdc, �ҷ��� �̹��� x, �ҷ��� �̹��� y, �ҷ��� �̹��� width, �ҷ��� �̹��� height, 
	//������ ����)

	DeleteDC(memdc);
	DeleteObject(bitmap);
	ReleaseDC(hWnd, hdc);
}

/*��ư ����ϰ� ��ư�� RECT ���� ��ȯ*/
RECT paintBtn(Button * b) {
	

	paintTransparent(image_path[b->btnType], b->printX, b->printY, 150 * b->scale, 120 * b->scale, 0, 0, 151, 118);

	HWND hWnd = getConsoleWindowHandle();
	HDC hdc = GetDC(hWnd);
	HFONT font = CreateFont(30, 0, 0, 0, 300, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "a�ٸ�����");
	(HFONT)SelectObject(hdc, font);

	SetTextColor(hdc, RGB(255, 255, 255));
	SetBkMode(hdc, TRANSPARENT);
	RECT rect = { b->printX, b->printY, b->printX + 150 * b->scale, b->printY + 120 * b->scale };
	DrawTextA(hdc, b->btnText, -1, &rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	GdiFlush();
	return rect;
}

/*��ư�� ��ġ�� ���� ��ư�� ����ϰ� �ٽ� ���� ��ư ���*/
void paintPushedBtn(Button * b) {

	is_pushed = 1;
	HWND hWnd = getConsoleWindowHandle();
	HDC hdc = GetDC(hWnd);
	HFONT font = CreateFont(30, 0, 0, 0, 300, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "a�ٸ�����");
	(HFONT)SelectObject(hdc, font);

	SetTextColor(hdc, RGB(200, 200, 200));
	SetBkMode(hdc, TRANSPARENT);
	RECT rect = { b->printX, b->printY, b->printX + 150 * b->scale, b->printY + 120 * b->scale };

	paintTransparent(image_path[b->btnType], b->printX, b->printY, 150 * b->scale, 120 * b->scale, 150, 0, 149, 120);
	DrawTextA(hdc, b->btnText, -1, &rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	GdiFlush();
	Sleep(120);

	paintTransparent(image_path[b->btnType],b->printX, b->printY, 150 * b->scale, 120 * b->scale, 0, 0, 151, 120);



	SetTextColor(hdc, RGB(255, 255, 255));
	SetBkMode(hdc, TRANSPARENT);

	DrawTextA(hdc, b->btnText, -1, &rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	GdiFlush();
	//LeaveCriticalSection(&cs2);
	return;
}

/*���� ��, ���� �̵�, ��ǥ �� �� ���*/
void displayCurNum(int type) {

	EnterCriticalSection(&cs);
	if (type == 2) {
		moves = 0;
		curNum = -1000000;
	}

	char temp[50];
	HWND hWnd = getConsoleWindowHandle();
	HDC hdc = GetDC(hWnd);
	HFONT font = CreateFont(120, 65, 0, 0, 500, 1, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "digital-7 Mono");

	HFONT font2 = CreateFont(45, 24, 0, 0, 500, 1, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "digital-7 Mono");
	(HFONT)SelectObject(hdc, font);


	SetBkMode(hdc, TRANSPARENT);
	RECT rect = { 733,230,733 + 466 - 45,230 + 119 };

	
	if (s[curStage].boss != 1) {
		paintTransparent("image/displayer_1.bmp", 733, 230, 466, 119, 0, 0, 466, 119);
	}
	else {
		paintTransparent("image/displayer_2.bmp", 733, 230, 466, 119, 0, 0, 466, 119);
	}
	
	if (s[curStage].boss != 1) {
		SetTextColor(hdc, RGB(162, 180, 158));
	}
	else {
		SetTextColor(hdc, RGB(247, 107, 107));
	}


	DrawTextA(hdc, "888888", -1, &rect, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);


	RECT rect2 = { 910, 156, 1032, 201 };

	SetBkColor(hdc, RGB(93, 116, 88));
	SetBkMode(hdc, OPAQUE);

	(HFONT)SelectObject(hdc, font2);
	
	if (s[curStage].boss != 1) {
		DrawTextA(hdc, "               ", -1, &rect2, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	}
	if (type > 0) {

		if (moves <= 1) {
			SetTextColor(hdc, RGB(255, 0, 0));
			sprintf(temp, "%d", moves > 0 ? --moves : 0);
		}
		else {
			SetTextColor(hdc, RGB(62, 62, 62));
			sprintf(temp, "%d", moves > 0 ? --moves : 0);
		}


	}
	else {

		SetTextColor(hdc, RGB(62, 62, 62));
		sprintf(temp, "%d", moves);
	}
	
	if (s[curStage].boss != 1) {
		DrawTextA(hdc, temp, -1, &rect2, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	}

	if (type == 0) {

		if (s[curStage].boss != 1) {
			RECT rect3 = { 1056, 156, 1175, 201 };
			SetTextColor(hdc, RGB(62, 62, 62));
			sprintf(temp, "%d", goalNum);
		
			DrawTextA(hdc, temp, -1, &rect3, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
		}
		else {
			SetBkColor(hdc, RGB(255, 155, 155));
			RECT rect3 = { 916, 150, 1175, 191 };
			SetTextColor(hdc, RGB(62, 62, 62));
			sprintf(temp, "%d", goalNum);

			DrawTextA(hdc, temp, -1, &rect3, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
		}

		
	}



	GdiFlush();
	Sleep(100);

	sprintf(temp, "%d", curNum);

	SetBkMode(hdc, TRANSPARENT);

	if ((type != 0 && moves == 0 && curNum != goalNum) || type == 2) SetTextColor(hdc, RGB(255, 0, 0));
	else SetTextColor(hdc, RGB(0, 0, 0));
	(HFONT)SelectObject(hdc, font);
	if (type != 2) {
		DrawTextA(hdc, temp, -1, &rect, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);
	}
	else {
		DrawTextA(hdc, "Error", -1, &rect, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);
	}
	
	GdiFlush();

	if (isEnd()) {

		Sleep(300);
		soundPlay("sound/eff_clear.wav");
		if (s[curStage].boss != 1) {
			paintTransparent("image/displayer_1.bmp", 733, 230, 466, 119, 0, 0, 466, 119);
		}
		else {
			paintTransparent("image/displayer_2.bmp", 733, 230, 466, 119, 0, 0, 466, 119);
		}

		if (s[curStage].boss != 1) {
			SetTextColor(hdc, RGB(162, 180, 158));
		}
		else {
			SetTextColor(hdc, RGB(247, 107, 107));
		}

		
		DrawTextA(hdc, "888888", -1, &rect, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);
		SetTextColor(hdc, RGB(0, 0, 0));
		Sleep(200);
		DrawTextA(hdc, "WIN", -1, &rect, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);
		Button t = { VK_NUMPAD9, MESSAGE_BTN, "OK", 1061, 488, 1.15 };

		saveRecord(s[curStage].level);

		paintBtn(&t);

		if (s[curStage].boss == 1) {
			HFONT font2 = CreateFont(45, 24, 0, 0, 500, 1, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "digital-7 Mono");
			(HFONT)SelectObject(hdc, font2);
			SetBkColor(hdc, RGB(255, 155, 155));
			RECT rect3 = { 916, 150, 1175, 191 };
			SetBkMode(hdc, OPAQUE);
			SetTextColor(hdc, RGB(62, 62, 62));
			sprintf(temp, "     %d     ", goalNum - curNum);

			DrawTextA(hdc, temp, -1, &rect3, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
		}


		isCleared = 1;

	}
	GdiFlush();
	isEnd();

	is_pushed = 0;


	LeaveCriticalSection(&cs);
}

/*Ư�� ���ڿ� ġȯ ( ' -> ' ��ư ����� ���� �ʿ�)*/
char *replaceAll(char *orig, char *rep, char *with) {
	char *result; // the return string
	char *ins;    // the next insert point
	char *tmp;    // varies
	int len_rep;  // length of rep (the string to remove)
	int len_with; // length of with (the string to replace rep with)
	int len_front; // distance between rep and end of last rep
	int count;    // number of replacements

	// sanity checks and initialization
	if (!orig || !rep)
		return NULL;
	len_rep = strlen(rep);
	if (len_rep == 0)
		return NULL; // empty rep causes infinite loop during count
	if (!with)
		with = "";
	len_with = strlen(with);

	// count the number of replacements needed
	ins = orig;
	for (count = 0; tmp = strstr(ins, rep); ++count) {
		ins = tmp + len_rep;
	}

	tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

	if (!result)
		return NULL;

	// first time through the loop, all the variable are set correctly
	// from here on,
	//    tmp points to the end of the result string
	//    ins points to the next occurrence of rep in orig
	//    orig points to the remainder of orig after "end of rep"
	while (count--) {
		ins = strstr(orig, rep);
		len_front = ins - orig;
		tmp = strncpy(tmp, orig, len_front) + len_front;
		tmp = strcpy(tmp, with) + len_with;
		orig += len_front + len_rep; // move to next "end of rep"
	}
	strcpy(tmp, orig);
	return result;
}

/*��ư�� ���� ��� ����*/
void executeBtn(Button * bb) {

	
	sprintf(soundPath, "sound/eff_button%d.wav", bb->key - VK_NUMPAD0);
	HANDLE thread10 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)soundPlay, (void *)soundPath, 0, NULL);

	if (bb->btnType == CLEAR_BTN) {
		sprintf(soundPath, "sound/eff_button9.wav");

		HANDLE thread10 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)soundPlay, (void *)soundPath, 0, NULL);
	}
	


	Sleep(100);

	char * text = bb->btnText;

	if (bb->btnType == ARITH_BTN) {
		int num1;
		char oper;
		sscanf(text, "%c %d", &oper, &num1);
		switch (oper) {
		case '+':
			curNum += num1;
			break;
		case '-':
			curNum -= num1;
			break;
		case 'x':
			curNum *= num1;
			break;
		case '/':
			curNum /= num1;
			break;

		}
		
	}
	else if (bb->btnType == FUNC_BTN) {
		
		char oper[15];
		char oper2;
		sscanf(text, "%s %c", oper, &oper2);
		if (!strcmp(oper, "Reverse")) {
			int r = 0;
			int a = curNum;
			while (a) {
				r = (r * 10) + (a % 10);
				a = a / 10;
			}
			curNum = r;

		}
		else if (!strcmp(oper, "+/-")) {
			curNum = -curNum;
	
		}
		else if (!strcmp(oper, "<<")) {
			curNum /= 10;
	
		}
		else if (!strcmp(oper, "x��")) {
			curNum = curNum * curNum;
		}
		else if (!strcmp(oper, "x��")) {
			curNum = curNum * curNum * curNum;
		}
		else if (!strcmp(oper, "SUM")) {
			int temp = curNum;
			int s = 0;
			while (temp != 0) {
				s += temp % 10;
				temp /= 10;
			}
			curNum = s;
		}
		else if (!strcmp(oper, "Shift")) {
			if (oper2 == '>') {
				int temp = curNum;
				int c = 1;
				while (temp / 10 != 0) {
					temp /= 10;
					c *= 10;
				}
				curNum = (curNum / 10) + ((curNum % 10) * c);
			}
			else {
				int temp = curNum;
				int c = 1;
				while (temp / 10 != 0) {
					temp /= 10;
					c *= 10;
				}

				curNum = (curNum / c) + ((curNum % c) * 10);
			}

			
		}
		else if (strstr(text, "=>") != NULL) {
			char num1[15], num2[15];
			sscanf(text, "%s => %s", &num1, &num2);

			char * tempNum;
			char tempNum2[15];
			sprintf(tempNum2, "%d", curNum);
			tempNum = replaceAll(tempNum2, num1, num2);

	
			
			curNum = atoi(tempNum);
			
		}


	}
	else if (bb->btnType == APPEND_BTN) {
	
		int num1;
		sscanf(text, "%d", &num1);
		
		int temp = num1;
		int c = 1;
		while (temp != 0) {
			temp /= 10;
			c *= 10;
		}

		if (curNum >= 0) {
			curNum = curNum * c + num1;
		}
		else {
			curNum = -(-curNum * c + num1);
		}
		
		
		
		
	}
	else if (bb->btnType == CLEAR_BTN) {

		curNum = startNum;
		moves = s[curStage].moves;
		
		

		Sleep(200);

		if (s[curStage].boss == 1) {
			for (int i = 0; i < 9; i++) {
				isUsed[i] = 0;
			}

			for (int i = 0; i < button_n; i++) {
				btnRect[i] = paintBtn(&b[i]);
			}
		}

	
	}


	if (bb->btnType == CLEAR_BTN) {
		HANDLE thread3 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)displayCurNum, (void *)0, 0, NULL);

	}
	else if (curNum <= 999999 && curNum >= -999999) {
		HANDLE thread3 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)displayCurNum, (void *)1, 0, NULL);

	}
	
	else{
		curNum = startNum;
		moves = s[curStage].moves;

		Sleep(200);
		HANDLE thread3 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)displayCurNum, (void *)2, 0, NULL);

	}


}

/*
������ �̰���� ��ȯ
i) �������� �ƴ� ���
	if (move >= 0 AND curNum == GoalNum) => return 1
	else => return 0;
ii) ������ �� ���
	if (move == 0 AND curNum >= GoalNum) => return 1
	else => return 0;
*/
int isEnd() {

	if (s[curStage].boss != 1) {
		if (moves >= 0 && curNum == s[curStage].goalNum) {
			/*����*/
			return 1;
		}
		else if (moves <= 0 && curNum != s[curStage].goalNum) {

			/*����*/
			curNum = startNum;
			moves = s[curStage].moves;
			Sleep(200);

			HANDLE thread3 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)displayCurNum, (void *)0, 0, NULL);


			return 0;
		}
		
	}
	else {

		if (moves == 0 && curNum >= s[curStage].goalNum) {

			/*����*/
			
			return 1;
		}
		else if (moves == 0 && curNum < s[curStage].goalNum){

			/*����*/
			curNum = startNum;
			moves = s[curStage].moves;
			Sleep(200);
			for (int i = 0; i < 9; i++) {
				isUsed[i] = 0;
			}

			for (int i = 0; i < button_n; i++) {
				
				btnRect[i] = paintBtn(&b[i]);
			}
			HANDLE thread3 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)displayCurNum, (void *)0, 0, NULL);
			return 0;
		}


		

	}
	return 0;
	

}

/*(x,y)�� RECT �ȿ� �ִ� �� (���콺 �Է� �ޱ� ���� ���)*/
int isInsideRect(RECT * r, int x, int y) {

	if ((x >= r->left) && (x <= r->right) && (y <= r->bottom) && (y >= r->top) && hForeWindow == GetForegroundWindow()) return 1;
	else return 0;

}

/*�޼����� ���÷��̿� ������ (Ʃ�丮�� ���)*/
void showMessage(Message * m) {
	HWND hWnd = getConsoleWindowHandle();
	HDC hdc = GetDC(hWnd);
	Button tt;

	tt.key = VK_NUMPAD5;
	tt.btnType = MESSAGE_BTN;
	strcpy(tt.btnText, m->btnText);
	tt.printX = 876;
	tt.printY = 648;
	tt.scale = 1.15;
	RECT messageRect = paintBtn(&tt);
	if (s[curStage].boss != 1) {
		paintTransparent("image/displayer_1.bmp", 733, 230, 466, 119, 0, 0, 466, 119);
	}
	else {
		paintTransparent("image/displayer_2.bmp", 733, 230, 466, 119, 0, 0, 466, 119);
	}
	Sleep(400);

	
	
	SetTextColor(hdc, RGB(0, 0, 0));
	SetBkMode(hdc, TRANSPARENT);
	
	if (m->isMultiLine >= 1) {
		HFONT font3 = CreateFont(35, 0, 0, 0, 500, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "a�ٸ�����");
		(HFONT)SelectObject(hdc, font3);
		RECT rect = { 733 + 30,255,733 + 466 - 45,325 }; 
		DrawTextA(hdc, m->displayText, -1, &rect, DT_WORDBREAK | DT_LEFT); 

	}
	else {
		HFONT font3 = CreateFont(35, 0, 0, 0, 500, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "a�ٸ�����");
		(HFONT)SelectObject(hdc, font3);
		RECT rect = { 733 + 30,230,733 + 466 - 45,230 + 119 };
		DrawTextA(hdc, m->displayText, -1, &rect, DT_SINGLELINE | DT_VCENTER | DT_LEFT); 

	}

	GdiFlush();


	while (1) {
		if (keyCheck(VK_NUMPAD5, key_arr)) {
			break;
		}
		POINT p;
		if (keyCheck(VK_LBUTTON, key_arr)) {
			GetCursorPos(&p);
	
			if (isInsideRect(&messageRect, p.x, p.y)) {

				break;
			}
		}

			

		Sleep(50);
	}

	paintPushedBtn(&tt);


}

/*initStage���� ���� ��ư ������ �ʱ�ȭ ��*/
int initButton(Button * b) {

	char * st = s[curStage].button_list;

	char st2[80];
	int k;
	for (k = 0; *(st + k) != NULL; k++) {
		st2[k] = *(st + k);
	}
	st2[k] = NULL;

	int num1, num2 = NULL;
	char * num3;


	char *ch;
	ch = strtok(st2, ",");

	int i = 0;
	while (ch != NULL) {
		if (sscanf(ch, "+ %d", &num1)) {
			b[i].btnType = ARITH_BTN;
		}
		else if (sscanf(ch, "x %d", &num1)) {
			b[i].btnType = ARITH_BTN;
		}
		else if (sscanf(ch, "- %d", &num1)) {
			b[i].btnType = ARITH_BTN;
		}
		else if (sscanf(ch, "/ %d", &num1)) {
			b[i].btnType = ARITH_BTN;
		}
		else if (sscanf(ch, "%d => %d", &num1, &num2) && strstr(ch, "=>")) {
			b[i].btnType = FUNC_BTN;
		}
		else if (sscanf(ch, "%d", &num1)) {
			b[i].btnType = APPEND_BTN;
		}
		else if (!strcmp(ch, "Reverse")) {
			b[i].btnType = FUNC_BTN;
		}
		else if (!strcmp(ch, "+/-")) {
			b[i].btnType = FUNC_BTN;
		}
		else if (!strcmp(ch, "<<")) {
			b[i].btnType = FUNC_BTN;
		}
		else if (!strcmp(ch, "x��")) {
			b[i].btnType = FUNC_BTN;
		}
		else if (!strcmp(ch, "x��")) {
			b[i].btnType = FUNC_BTN;
		}
		else if (!strcmp(ch, "SUM")) {
			b[i].btnType = FUNC_BTN;
		}
		else if (!strcmp(ch, "Shift <")) {
			b[i].btnType = FUNC_BTN;
		}
		else if (!strcmp(ch, "Shift >")) {
			b[i].btnType = FUNC_BTN;
		}
		else if (!strcmp(ch, "CLR")) {
			b[i].btnType = CLEAR_BTN;
		}



		strcpy(b[i].btnText, ch);

		b[i].printX = 686 + 190 * i;
		b[i].printY = 900;
		b[i].scale = 1.15;

		ch = strtok(NULL, ",");
		i++;
	}
	for (int j = 0; j < i; j++) {


		if (!strcmp(b[j].btnText, "CLR")) {

			b[j].key = VK_NUMPAD9;
			b[j].printX = 1061;
			b[j].printY = 488;
		}
		else {
			int slot = j + 1;
			b[j].key = VK_NUMPAD0 + slot;
			if (slot % 3 == 0) {
				b[j].printX = 1061;
			}
			else if (slot % 3 == 1) {
				b[j].printX = 686;
			}
			else if (slot % 3 == 2) {
				b[j].printX = 876;
			}

			if ((slot - 1) / 3 == 0) {
				b[j].printY = 798;
			}
			else if ((slot - 1) / 3 == 1) {
				b[j].printY = 648;
			}
			else if ((slot - 1) / 3 == 2) {
				b[j].printY = 488;
			}
		}


	}

	return i;
}

/*���� ���������� ������ ���������� �ʱ�ȭ ��*/
void initStage() {

	if (s[curStage].boss != 1) {
		paintTransparent("image/calculator_1.bmp", 960 - 319, 0, 639, 1080, 0, 0, 550, 929);

	}
	else {
		paintTransparent("image/calculator_2.bmp", 960 - 319, 0, 639, 1080, 0, 0, 550, 929);
	}


	int c = 0;
	while (s[curStage].message[c].btnText) c++;

	for (int i = 0; i < c; i++) {
		Message m = s[curStage].message[i];
		showMessage(&m);
	}


	
	if (s[curStage].boss != 1) {
		paintTransparent("image/calculator_1.bmp", 960 - 319, 0, 639, 1080, 0, 0, 550, 929);

	}
	else {
		paintTransparent("image/calculator_2.bmp", 960 - 319, 0, 639, 1080, 0, 0, 550, 929);
	}
	
	
	if (s[curStage].boss != 1) {
		paintTransparent("image/displayer_1.bmp", 733, 230, 466, 119, 0, 0, 466, 119);
	}
	else {
		paintTransparent("image/displayer_2.bmp", 733, 230, 466, 119, 0, 0, 466, 119);
	}

	paintTransparent("image/btn_back.bmp", 689, 961, 540, 85, 0, 0, 490, 92);
	RECT backRect = { 689, 961, 1229, 1046 };


	HWND hWnd = getConsoleWindowHandle();
	HDC hdc = GetDC(hWnd);
	HFONT font3 = CreateFont(30, 0, 0, 0, 500, 1, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "a�ٸ�����");

	(HFONT)SelectObject(hdc, font3);
	
	SetTextColor(hdc, RGB(255, 255, 255));
	SetBkMode(hdc, TRANSPARENT);
	RECT rect = { 730, 57, 915, 87 };
	
	char temp[15];
	sprintf(temp, "LEVEL : %d", s[curStage].level);
	
	DrawTextA(hdc, temp, -1, &rect, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
	GdiFlush();



	startNum = s[curStage].startNum;
	curNum = startNum;
	moves = s[curStage].moves;
	goalNum = s[curStage].goalNum;
	level = s[curStage].level;

	isCleared = 0;
	
	
	for (int i = 0; i < 9; i++) {
		isUsed[i] = 0;
	}


	button_n = initButton(b);





	for (int i = 0; i < button_n; i++) {
		btnRect[i] = paintBtn(&b[i]);
	}



	displayCurNum(0);


	while (1) {
		POINT p;
		if (keyCheck(VK_LBUTTON, key_arr)) {
			GetCursorPos(&p);

			if (isInsideRect(&backRect, p.x, p.y)) {
				displayStageSelect((s[curStage].level - 1) / 25 + 1);
				return;
			}


			if (GetAsyncKeyState(VK_UP)) {
				char tmp[15];
				sprintf(tmp, "%d %d", p.x, p.y);
				MessageBox(0, "Hello", tmp, 1);

			}

			for (int i = 0; i < button_n; i++) {
				if (isCleared == 1) {
					if (isInsideRect(&btnRect[button_n - 1], p.x, p.y)) {
						if (curStage != 49) {
							curStage++;
						}
						else {
							displayStageSelect(2);
							return;
						}

						HANDLE thread1 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)initStage, NULL, 0, NULL);
						return;
					}
				}
				else {
					if (isInsideRect(&btnRect[i], p.x, p.y)) {
						if (is_pushed == 0) {
							if (s[curStage].boss != 1) {
								HANDLE thread1 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)executeBtn, &b[i], 0, NULL);
								
								paintPushedBtn(&b[i]);
							}
							else {
								if (isUsed[i] == 0) {

									HANDLE thread1 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)executeBtn, &b[i], 0, NULL);
									

									paintPushedBtn(&b[i]);


									if (strcmp(b[i].btnText, "CLR")) {
										Button t = { NULL, NULL_BTN, "", b[i].printX, b[i].printY, 1.15 };
										paintBtn(&t);
										isUsed[i] = 1;
									}
								}

							}
							
						}

					}
				}


			}
		}
		else {

			if (keyCheck(VK_SPACE, key_arr)) {
				displayStageSelect((s[curStage].level - 1) / 25 + 1);
				return;
			}


			for (int i = 0; i < button_n; i++) {

				if (isCleared == 1) {
					if (keyCheck(b[button_n - 1].key, key_arr)) {

						if (curStage != 49) {
							curStage++;
						}
						else {
							displayStageSelect(2);
							return;
						}
						
						isCleared = 0;
						HANDLE thread1 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)initStage, NULL, 0, NULL);
						return;
					}
				}
				else {
					if (keyCheck(b[i].key, key_arr)) {

						if (s[curStage].boss != 1) {
							HANDLE thread1 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)executeBtn, &b[i], 0, NULL);
							
							paintPushedBtn(&b[i]);
						}
						else {
							
							if (isUsed[i] == 0) {
								HANDLE thread1 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)executeBtn, &b[i], 0, NULL);

								paintPushedBtn(&b[i]);
								
								
								if (strcmp(b[i].btnText, "CLR")) {
									Button t = { NULL, NULL_BTN, "", b[i].printX, b[i].printY, 1.15 };
									paintBtn(&t);
									isUsed[i] = 1;
								}
							}
							
						}

					}
				}

			}

		}






		Sleep(50);

	}

}

/*�������� ����â �̵�*/
void displayStageSelect(int page) {

	FILE *f1;
	char saved[100];
	f1 = fopen("savepoint/savefile", "rb");
	if (f1 == NULL) {

		initRecord();
		f1 = fopen("savepoint/savefile", "rb");
	}
	fscanf(f1, "%s", &saved);

	
	encodeStr(saved);

	fclose(f1);



	RECT stageRect[26];
	RECT selectRect;
	RECT mainRect;
	Button tt[26];

	paintTransparent("image/levelSelect.bmp", 0, 0, 1920, 1080, 0, 0, 1888, 1068);

	Button s = { SELECT_BTN, VK_LEFT, "<", 300,960, 1 };

	if (page != 1) {
		paintTransparent(image_path[SELECT_BTN], 140, 510, 214 * 0.5, 139 * 0.5, 0, 0, 214, 139);
		RECT t = { 140, 510, 140 + 214 * 0.5, 510 + 139 * 0.5 };
		selectRect = t;
	}
	else {
		paintTransparent(image_path[SELECT_BTN], 1670, 510, 214 * 0.5, 139 * 0.5, 0, 0, 214, 139);
		RECT t = { 1670, 510, 1670 + 214 * 0.5, 510 + 139 * 0.5 };
		selectRect = t;
		paintTransparent(image_path[SELECT_BTN], 140, 510, 214 * 0.5, 139 * 0.5, 0, 0, 214, 139);
		RECT t2 = { 140, 510, 140 + 214 * 0.5, 510 + 139 * 0.5 };
		mainRect = t2;
	}



	int sum = 0;
	for (int i = 1; i <= 50; i++) {
		int n;

		n = saved[i] - 48;
		if (n >= 1 && n <= 9) sum += i * n;
	}
	saved[51] -= 48;
	saved[52] -= 48;
	int checkSum = (saved[51]) * 10 + (saved[52]);

	if (sum % 100 != checkSum) {

		initRecord();
		f1 = fopen("savepoint/savefile", "rb");
		fscanf(f1, "%s", &saved);
	}


	for (int i = 1; i <= 25; i++) {
		char temp[15];
		sprintf(temp, "%d", i + (page - 1) * 25);

		

		tt[i].printX = 385 + ((i - 1) % 5) * 250;
		tt[i].printY = 225 + ((i - 1) / 5) * 150;
		int n = saved[i + (page - 1) * 25] - 48;
		if (n >= 1 && n <= 9) {
			tt[i].btnType = CLEARED_BTN;
		}
		else {
			if (i % 5 != 0) {
				tt[i].btnType = MESSAGE_BTN;
			}
			else {
				tt[i].btnType = CLEAR_BTN;
			}
		}

		tt[i].key = 0;
		strcpy(tt[i].btnText, temp);
		tt[i].scale = 1;

		stageRect[i] = paintBtn(&tt[i]);
	}


	

	while (1) {
		POINT p;
	
		if (keyCheck(VK_LBUTTON, key_arr)) {
			GetCursorPos(&p);

			for (int i = 1; i <= 25; i++) {

				if (isInsideRect(&stageRect[i], p.x, p.y)) {

					paintPushedBtn(&tt[i]);
					curStage = i + (page - 1) * 25 - 1;
					soundPlay("sound/eff_button3.wav");
					
					paintTransparent("image/background.bmp", 0, 0, 1920, 1080, 0, 0, 75, 75);

					HANDLE thread7 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)initStage, NULL, 0, NULL);
					return;


				}


			}

			if (isInsideRect(&selectRect, p.x, p.y)) {



				if (page == 1) {
					HANDLE thread9 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)displayStageSelect, (void *)2, 0, NULL);
				}
				else {
					HANDLE thread9 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)displayStageSelect, (void *)1, 0, NULL);
				}

				return;


			}

			if (isInsideRect(&mainRect, p.x, p.y)) {
	
				HANDLE thread9 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)displayMainScreen, NULL, 0, NULL);
				return;
			}





		}
		Sleep(30);
	}

}

/*���� �̵�*/
void displayMainScreen() {
	

	paintTransparent("image/background.bmp", 0, 0, 1920, 1080, 0, 0, 75, 75);

	paintTransparent("image/main_1.bmp", 640, 0, 640, 1080, 0, 0, 1000, 1687);

	int curSelect = 1;
	RECT startRect = { 739, 616, 1179,697 };
	RECT howtoRect = { 739, 742, 1179, 824 };
	RECT exitRect = { 739, 872, 1179, 953 };

	

	while (1) {
		POINT p;

		if (keyCheck(VK_RETURN, key_arr)) {
			if (curSelect == 1) {
				HANDLE thread8 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)displayStageSelect, (void *)1, 0, NULL);
				return;
			}
			else if (curSelect == 2) {
				HANDLE thread8 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)displayHowto, NULL, 0, NULL);
				return;
			}
			else {
				exit(1);
			}
		}else if (keyCheck(VK_DOWN, key_arr)) {
			if (curSelect != 3) curSelect += 1;
			char temp[20];
			sprintf(temp, "image/main_%d.bmp", curSelect);
			paintTransparent(temp, 640, 0, 640, 1080, 0, 0, 1000, 1687);
		}
		else if (keyCheck(VK_UP, key_arr)) {
			if (curSelect != 1) curSelect -= 1;
			char temp[20];
			sprintf(temp, "image/main_%d.bmp", curSelect);
			paintTransparent(temp, 640, 0, 640, 1080, 0, 0, 1000, 1687);
		}
		else if (keyCheck(VK_LBUTTON, key_arr)) {
			GetCursorPos(&p);


			if (isInsideRect(&startRect, p.x, p.y)) {

				HANDLE thread8 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)displayStageSelect, (void *)1, 0, NULL);
				return;


			}
			else if (isInsideRect(&howtoRect, p.x, p.y)) {
				HANDLE thread8 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)displayHowto, NULL, 0, NULL);
				return;
			}


			else if (isInsideRect(&exitRect, p.x, p.y)) {
				exit(1);
			}


		
		}


		Sleep(30);
	}
	Sleep(10000);
}

/*���� ��� �̵�*/
void displayHowto() {

	paintTransparent("image/howto.bmp", 0, 0, 1950, 1080, 0, 0, 1812, 1018);
	while (!keyCheck(VK_LBUTTON, key_arr) && !keyCheck(VK_RETURN, key_arr)
		
		
		
		
		) Sleep(50);
	HANDLE thread = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)displayMainScreen, NULL, 0, NULL);
	return;
}

/*�������� ����ü �迭 �ʱ�ȭ*/
void setStageInfo() {
	s[0].level = 1;
	s[0].button_list = "+ 2,- 1,CLR";
	s[0].moves = 3;
	s[0].startNum = 2;
	s[0].goalNum = 5;
	s[0].message[0].displayText = "�ȳ�!!!!!";
	s[0].message[0].btnText = "HI";
	s[0].message[1].displayText = "�ݰ���!!!!";
	s[0].message[1].btnText = "HELLO";
	s[0].message[2].displayText = "���� ��ǥ�� GOAL�� ���ڸ� ����°ž�!";
	s[0].message[2].btnText = "OH";
	s[0].message[2].isMultiLine = 1;
	s[0].message[3].displayText = "���� ������ MOVES �ȿ�!!!";
	s[0].message[3].btnText = "OK";
	s[0].message[3].isMultiLine = 1;
	s[0].message[4].displayText = "�׷��� ���Ⱑ ���峪�� ��ư�� �� �� ����!";
	s[0].message[4].btnText = "WOW!";
	s[0].message[4].isMultiLine = 1;
	s[0].message[5].displayText = "�ƹ�ư ����� ����!!!!!!!!!!!!!!!!!!";
	s[0].message[5].btnText = "THANKS";
	s[0].message[5].isMultiLine = 1;

	s[1].level = 2;
	s[1].button_list = "x 2,- 2,CLR";
	s[1].moves = 4;
	s[1].startNum = 3;
	s[1].goalNum = 22;

	s[2].level = 3;
	s[2].button_list = "+ 3,x 7,- 2,CLR";
	s[2].moves = 3;
	s[2].startNum = 5;
	s[2].goalNum = 24;

	s[3].level = 4;
	s[3].button_list = "+ 1,x 2,- 3,/ 4,CLR";
	s[3].moves = 4;
	s[3].startNum = 36;
	s[3].goalNum = 14;

	s[4].level = 5;
	s[4].button_list = "+ 5,/ 3,- 11,x 4,CLR";
	s[4].moves = 4;
	s[4].startNum = 10;
	s[4].goalNum = 5;
	s[4].boss = 1;
	s[4].message[0].displayText = "�ȳ�!!!!";
	s[4].message[0].btnText = "HI";
	s[4].message[1].displayText = "5 ������������ ������ ������!";
	s[4].message[1].btnText = "����";
	s[4].message[1].isMultiLine = 1;
	s[4].message[2].displayText = "������ HP �̻��� ���ڸ� ����� �̰�!";
	s[4].message[2].btnText = "WOW";
	s[4].message[2].isMultiLine = 1;
	s[4].message[3].displayText = "��� ��ư�� �� ������ ����ؾ� ��!";
	s[4].message[3].btnText = "OH";
	s[4].message[3].isMultiLine = 1;
	s[4].message[4].displayText = "������ ����� ��ư�� (NULL)�� �ٲ������!";
	s[4].message[4].btnText = "YEAH";
	s[4].message[4].isMultiLine = 1;
	s[4].message[5].displayText = "����� ����!!!!!!";
	s[4].message[5].btnText = "THANKS";

	////////////////////////////////////////////////////////////////////////////////

	s[5].level = 6;
	s[5].button_list = "2,x 2,CLR";
	s[5].moves = 3;
	s[5].startNum = 1;
	s[5].goalNum = 222;
	s[5].message[0].displayText = "�ȳ�!!!!!";
	s[5].message[0].btnText = "HELLO";
	s[5].message[1].displayText = "���ο� ��ư�� ã�Ҿ�!!";
	s[5].message[1].btnText = "YEAH";
	s[5].message[2].displayText = "������ �ڸ� �ڿ� ���ڸ� �߰����ִ� ��ư�̾�!";
	s[5].message[2].btnText = "OK";
	s[5].message[2].isMultiLine = 1;
	s[5].message[3].displayText = "����� ����!!";
	s[5].message[3].btnText = "THANKS";


	s[6].level = 7;
	s[6].button_list = "+ 3,4,- 8,CLR";
	s[6].moves = 4;
	s[6].startNum = 5;
	s[6].goalNum = 79;

	s[7].level = 8;
	s[7].button_list = "2,4,6,8,/ 2,CLR";
	s[7].moves = 4;
	s[7].startNum = 1;
	s[7].goalNum = 44;

	s[8].level = 9;
	s[8].button_list = "x 3,3,-3,CLR";
	s[8].moves = 4;
	s[8].startNum = 3;
	s[8].goalNum = 60;

	s[9].level = 10;
	s[9].button_list = "+ 3,x 2,/ 5,9,7,CLR";
	s[9].moves = 5;
	s[9].startNum = -10;
	s[9].goalNum = 333;
	s[9].boss = 1;

	////////////////////////////////////////////////////////////////////////////////

	s[10].level = 11;
	s[10].button_list = "- 21,<<,CLR";
	s[10].moves = 3;
	s[10].startNum = 1234;
	s[10].goalNum = 100;
	s[10].message[0].displayText = "�ȳ�!!!!";
	s[10].message[0].btnText = "HI";
	s[10].message[1].displayText = "���ο� ��ư�̾�!!";
	s[10].message[1].btnText = "����";
	s[10].message[2].displayText = "������ �ڸ��� �����ִ� ��ư�̾�!";
	s[10].message[2].btnText = "OK";
	s[10].message[2].isMultiLine = 1;
	s[10].message[3].displayText = "������!!";
	s[10].message[3].btnText = "THANKS";

	s[11].level = 12;
	s[11].button_list = "<<,/ 2,CLR";
	s[11].moves = 3;
	s[11].startNum = 42;
	s[11].goalNum = 1;

	s[12].level = 13;
	s[12].button_list = "x 2,- 9,<<,CLR";
	s[12].moves = 4;
	s[12].startNum = 171;
	s[12].goalNum = 23;

	s[13].level = 14;
	s[13].button_list = "12,<<,CLR";
	s[13].moves = 6;
	s[13].startNum = 1;
	s[13].goalNum = 1111;

	s[14].level = 15;
	s[14].button_list = "<<,<<,x 11,+ 100,5,CLR";
	s[14].moves = 5;
	s[14].startNum = 100;
	s[14].goalNum = 10000;
	s[14].boss = 1;

	////////////////////////////////////////////////////////////////////////////////

	s[15].level = 16;
	s[15].button_list = "+/-,-2,CLR";
	s[15].moves = 3;
	s[15].startNum = 2;
	s[15].goalNum = 4;
	s[15].message[0].displayText = "�ȳ�!!!!!!";
	s[15].message[0].btnText = "HI";
	s[15].message[1].displayText = "���ο� ��ư!!";
	s[15].message[1].btnText = "����";
	s[15].message[2].displayText = "����� ������, ������ ����� �ٲ���!";
	s[15].message[2].btnText = "YEAH";
	s[15].message[2].isMultiLine = 1;
	s[15].message[3].displayText = "����� ����!!";
	s[15].message[3].btnText = "THANKS";

	s[16].level = 17;
	s[16].button_list = "+ 4,+ 2,+/-,CLR";
	s[16].moves = 3;
	s[16].startNum = 0;
	s[16].goalNum = -6;

	s[17].level = 18;
	s[17].button_list = "/ 3,- 2,+/-,CLR";
	s[17].moves = 5;
	s[17].startNum = 72;
	s[17].goalNum = -4;

	s[18].level = 19;
	s[18].button_list = "6,+ 5,/ 8,+/-,CLR";
	s[18].moves = 5;
	s[18].startNum = 14;
	s[18].goalNum = 12;

	s[19].level = 20;
	s[19].button_list = "+/-,99,+ 5,<<,- 99,CLR";
	s[19].moves = 5;
	s[19].startNum = 99;
	s[19].goalNum = 999;
	s[19].boss = 1;

	////////////////////////////////////////////////////////////////////////////////

	s[20].level = 21;
	s[20].button_list = "x��,+ 4,CLR";
	s[20].moves = 4;
	s[20].startNum = 6;
	s[20].goalNum = 10004;
	s[20].message[0].displayText = "���ο� ��ư!!";
	s[20].message[0].btnText = "����";
	s[20].message[1].displayText = "������ ������\n��ư�̾�!";
	s[20].message[1].btnText = "WOW";
	s[20].message[1].isMultiLine = 1;
	s[20].message[2].displayText = "�׷��� �� ������� �� �ִµ�..";
	s[20].message[2].btnText = "HMM..";
	s[20].message[2].isMultiLine = 1;
	s[20].message[3].displayText = "�� ����� 6�ڸ� �ۿ� ǥ�� ����!!";
	s[20].message[3].btnText = "OH";
	s[20].message[3].isMultiLine = 1;
	s[20].message[4].displayText = "����� ����!!";
	s[20].message[4].btnText = "THANKS";

	s[21].level = 22;
	s[21].button_list = "x��,9,/ 3,CLR";
	s[21].moves = 4;
	s[21].startNum = 15;
	s[21].goalNum = 253;

	s[22].level = 23;
	s[22].button_list = "x��,+/-,- 1,CLR";  
	s[22].moves = 4;
	s[22].startNum = 5;
	s[22].goalNum = 126;

	s[23].level = 24;
	s[23].button_list = "x��,x��,<<,CLR";
	s[23].moves = 5;
	s[23].startNum = 22;
	s[23].goalNum = 36;

	s[24].level = 25;
	s[24].button_list = "x��,<<,1,2,3,4,CLR"; 
	s[24].moves = 6;
	s[24].startNum = 9;
	s[24].goalNum = 100000;
	s[24].boss = 1;

	////////////////////////////////////////////////////////////////////////////////

	s[25].level = 26;
	s[25].button_list = "Reverse,+ 9,CLR";
	s[25].moves = 3;
	s[25].startNum = 50;
	s[25].goalNum = 104;
	s[25].message[0].displayText = "�ȳ�!!";
	s[25].message[0].btnText = "HI";
	s[25].message[1].displayText = "�� ���ο� ��ư�� ã�Ҿ�!!!";
	s[25].message[1].btnText = "OH";
	s[25].message[1].isMultiLine = 1;
	s[25].message[2].displayText = "Reverse ��ư�̾�!!!";
	s[25].message[2].btnText = "WOW";
	s[25].message[3].displayText = "�� �̷� ��ư�� �־�� �ϴ��� �𸣰�����..";
	s[25].message[3].btnText = "YEAH";
	s[25].message[3].isMultiLine = 1;
	s[25].message[4].displayText = "����� ����!!";
	s[25].message[4].btnText = "THANKS";

	s[26].level = 27;
	s[26].button_list = "Reverse,/ 3,+ 3,CLR";
	s[26].moves = 4;
	s[26].startNum = 72;
	s[26].goalNum = 55;

	s[27].level = 28;
	s[27].button_list = "<<,5,Reverse,CLR";
	s[27].moves = 4;
	s[27].startNum = 63;
	s[27].goalNum = 53;

	s[28].level = 29;
	s[28].button_list = "Reverse,+/-,x 3,- 2,CLR";
	s[28].moves = 6;
	s[28].startNum = 12;
	s[28].goalNum = 96;

	s[29].level = 30;
	s[29].button_list = "98,76,9,<<,<<,Reverse,CLR";
	s[29].moves = 6;
	s[29].startNum = 666;
	s[29].goalNum = 969696;
	s[29].boss = 1;

	////////////////////////////////////////////////////////////////////////////////

	s[30].level = 31;
	s[30].button_list = "/ 2,1,1 => 2,CLR";
	s[30].moves = 3;
	s[30].startNum = 38;
	s[30].goalNum = 291;
	s[30].message[0].displayText = "�� ���ο� ��ư�̾�!!!!!!!!!!!!!";
	s[30].message[0].btnText = "WOW";
	s[30].message[0].isMultiLine = 1;
	s[30].message[1].displayText = "a -> b ��ư�� ���� ���� ��� a�� b�� �ٲ���!";
	s[30].message[1].btnText = "YEAH";
	s[30].message[1].isMultiLine = 1;
	s[30].message[2].displayText = "���� �������� �𸣰ھ�!";
	s[30].message[2].btnText = "WHAT?";
	s[30].message[2].isMultiLine = 1;
	s[30].message[3].displayText = "����� ����!!";
	s[30].message[3].btnText = "THANKS";

	s[31].level = 32;
	s[31].button_list = "/ 2,6,<<,2 => 7,CLR";
	s[31].moves = 5;
	s[31].startNum = 50;
	s[31].goalNum = 17;

	s[32].level = 33;
	s[32].button_list = "1 => 2,/ 2,Reverse,CLR";
	s[32].moves = 5;
	s[32].startNum = 62;
	s[32].goalNum = 16;

	s[33].level = 34;
	s[33].button_list = "+ 50,5 => 1,1 => 10,Reverse,CLR";
	s[33].moves = 5;
	s[33].startNum = 50;
	s[33].goalNum = 101;

	s[34].level = 35;
	s[34].button_list = "10 => 99,10,x 10,9,11 => 99,CLR";
	s[34].moves = 5;
	s[34].startNum = 11;
	s[34].goalNum = 999876;
	s[34].boss = 1;

	////////////////////////////////////////////////////////////////////////////////

	s[35].level = 36;
	s[35].button_list = "3 => 1,SUM,2 => 3,CLR";
	s[35].moves = 3;
	s[35].startNum = 1231;
	s[35].goalNum = 4;
	s[35].message[0].displayText = "�ȳ�!!";
	s[35].message[0].btnText = "HI";
	s[35].message[1].displayText = "���ο� ��ư�� �ϳ� �� ���Ծ�!";
	s[35].message[1].btnText = "OH";
	s[35].message[1].isMultiLine = 1;
	s[35].message[2].displayText = "SUM ��ư�̾�!";
	s[35].message[2].btnText = "OK";
	s[35].message[3].displayText = "�� �ڸ� ������ ������ �ٲ���!!";
	s[35].message[3].btnText = "����";
	s[35].message[3].isMultiLine = 1;
	s[35].message[4].displayText = "����� ����!!";
	s[35].message[4].btnText = "THANKS";

	s[36].level = 37;
	s[36].button_list = "+ 5,x 2,SUM,CLR";
	s[36].moves = 4;
	s[36].startNum = 12;
	s[36].goalNum = 32;

	s[37].level = 38;
	s[37].button_list = "3,+ 33,SUM,3 => 1,CLR";
	s[37].moves = 4;
	s[37].startNum = 3;
	s[37].goalNum = 8;

	s[38].level = 39;
	s[38].button_list = "x 9,+ 9,44 => 43,SUM,CLR";
	s[38].moves = 4;
	s[38].startNum = 142;
	s[38].goalNum = 143;

	s[39].level = 40;
	s[39].button_list = "SUM,SUM,x��,1 => 9,Reverse,100,CLR";
	s[39].moves = 6;
	s[39].startNum = 123456;
	s[39].goalNum = 12345;
	s[39].boss = 1;

	////////////////////////////////////////////////////////////////////////////////

	s[40].level = 41;
	s[40].button_list = "Shift >,+ 3,CLR";
	s[40].moves = 3;
	s[40].startNum = 123;
	s[40].goalNum = 234;
	s[40].message[0].displayText = "�Ƹ��� ������ ��ư�̾�!";
	s[40].message[0].btnText = "����";
	s[40].message[1].displayText = "�� �ڸ� ������ ��ġ�� �Ű���!!";
	s[40].message[1].btnText = "WOW";
	s[40].message[1].isMultiLine = 1;
	s[40].message[2].displayText = "�Ẹ�� �� �� ����!!";
	s[40].message[2].btnText = "WHAT?";
	s[40].message[3].displayText = "����� ����!!!!!!";
	s[40].message[3].btnText = "THANKS";

	s[41].level = 42;
	s[41].button_list = "+ 1,1,9,89 => 99,Shift >,CLR";
	s[41].moves = 5;
	s[41].startNum = 98;
	s[41].goalNum = 2000;

	s[42].level = 43;
	s[42].button_list = "+ 11,3 => 1,SUM,Shift <,CLR";
	s[42].moves = 5; // 4
	s[42].startNum = 212;
	s[42].goalNum = 1; // 3

	s[43].level = 44;
	s[43].button_list = "25 => 12,21 => 3,12 => 5,Shift >,Reverse,CLR";
	s[43].moves = 6;
	s[43].startNum = 2152;
	s[43].goalNum = 13;

	s[44].level = 45;
	s[44].button_list = "Shift <,Shift >,x 7,x 5,1,Reverse,<<,CLR";
	s[44].moves = 7;
	s[44].startNum = 13579;
	s[44].goalNum = 888888;
	s[44].boss = 1;

	////////////////////////////////////////////////////////////////////////////////

	s[45].level = 46;
	s[45].button_list = "39 => 93,/ 3,9,31 => 00,CLR";
	s[45].moves = 7;
	s[45].startNum = 9;
	s[45].goalNum = 3001;
	s[45].message[0].displayText = "�ȳ�!!!!!!!!!!";
	s[45].message[0].btnText = "����";
	s[45].message[1].displayText = "���� ���ο� ��ư�� ����!!!!!!!!";
	s[45].message[1].btnText = "OK";
	s[45].message[1].isMultiLine = 1;
	s[45].message[2].displayText = "����� ����!!!!!!";
	s[45].message[2].btnText = "THANKS";

	s[46].level = 47;
	s[46].button_list = "Reverse,- 3,Shift <,10 => 87,78 => 11,x��,CLR";
	s[46].moves = 6;
	s[46].startNum = 1004;
	s[46].goalNum = 12321;

	s[47].level = 48;
	s[47].button_list = "50 => 0,25 => 525,51 => 5,Shift >,CLR";
	s[47].moves = 6;
	s[47].startNum = 1025;
	s[47].goalNum = 52;

	s[48].level = 49;
	s[48].button_list = "x 2,10,SUM,x��,10 => 1,1 => 78,CLR";
	s[48].moves = 7; 
	s[48].startNum = 2;
	s[48].goalNum = 30;

	s[49].level = 50;
	s[49].button_list = "Shift <,Shift <,01 => 10,Reverse,10,10 => 11,x 100,x 99,CLR";
	s[49].moves = 8; // 5 7 2 6 3 1 4 8 
	s[49].startNum = 1;
	s[49].goalNum = 999999;
	s[49].boss = 1;
	s[49].message[0].displayText = "�ȳ�!!!!!!!!!!";
	s[49].message[0].btnText = "����";
	s[49].message[1].displayText = "���� ������� �Դٴ� �����!!";
	s[49].message[1].btnText = "YEAH";
	s[49].message[1].isMultiLine = 1;
	s[49].message[2].displayText = "�̹� ���������� ����ž�!!!!!!!!";
	s[49].message[2].btnText = "OK";
	s[49].message[2].isMultiLine = 1;
	s[49].message[3].displayText = "�� ��!!";
	s[49].message[3].btnText = "THANKS";
}


int main() {
	InitializeCriticalSection(&cs); //�Ӱ� ���� �ʱ�ȭ (������ ��� �� �ʿ�)

	gameInit(); //��üȭ��, ��ũ�ѹ� �� ����
	
	Sleep(1500);

	setStageInfo(); //�������� ����ü �迭 �ʱ�ȭ
	
	HANDLE thread9 = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)displayMainScreen, NULL, 0, NULL); //����ȭ�� �����ֱ�

	while (1) {
		Sleep(1000); //����Ǵ� ���� �����Լ� ������ �ʰ� ��
	}

	DeleteCriticalSection(&cs);
}