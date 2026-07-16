#include <Windows.h>
#include <stdio.h>
#include <KnownFolders.h>

#include "resource.h"

#include "my_userprog.h"
#include "my_screen.h"
#include "my_main.h"
#include "my_window.h"
#include "my_asm.h"
#include "my_memory.h"
#include "my_file.h"


#define APP_VERSION (100)

static void openConsole();
static void checkLicense();
void setupMainProg();

struct Program mainProg;
struct Program* activeProg = &userProg;




int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, PWSTR pCmdLine, int nCmdShow) {
	//checkLicense();
	setupMemory();
	setupMainProg();
	setupUserProg();
	openConsole();
	loadUserCodeFromDisk();
	setupWindow(hInstance, nCmdShow);
	reloadUserScript();

	//my_test_fn();

	runLoop();
	return 0;
}

void reloadUserScript() {
	DWORD read;
	SetFilePointer(file, 0, NULL, FILE_BEGIN);
	ReadFile(file, sys->progsrc, 0x2000, &read, NULL);
	sys->progsrc[read] = 0;

	printf("============\n");
	printf("%s\n", sys->progsrc);
	printf("============\n");

	size_t codesize = 0x2000;
	const char* err = assemble_string(usersignal, &codesize, sys->progsrc);

	if (err) {
		printf("err = %s\n", err);
	}
	else {
		activeProg->init();
		printf("size = %llu\n", codesize);
	}
}

static void checkLicense() {
	SYSTEMTIME time;
	GetSystemTime(&time);
	if (time.wYear > 2025 || time.wMonth > 7 || time.wDay > 29) {
		MessageBox(NULL, L"This HRAM beta version has expired, please get a new one, thanks!", L"HRAM beta version expired", 0);
		ExitProcess(0);
	}
}

static void init() {

}

static void openConsole() {
	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
}

static void tick(DWORD delta, DWORD now) {
	sys->time = now;
	activeProg->tick(delta, now);
}

static void mouseMoved(int x, int y) {
	sys->mousex = x;
	sys->mousey = y;
	activeProg->mouseMoved(x, y);
}

static void togglekeystate(int vk, int down) {
	UINT8 byteindex = vk / 8;
	UINT8 bit = vk % 8;
	sys->keys[byteindex] = (sys->keys[byteindex] & ~(1 << bit)) | (down << bit);
}

static void mouseDown(int b) {
	activeProg->mouseDown(b);
}

static void mouseUp(int b) {
	activeProg->mouseUp(b);
}

static void mouseWheel(int d) {
	activeProg->mouseWheel(d);
}

static void keyDown(int vk) {
	togglekeystate(vk, 1);
	activeProg->keyDown(vk);
}

static void keyUp(int vk) {
	togglekeystate(vk, 0);
	activeProg->keyUp(vk);
}

static void syskeyDown(int vk) {
	togglekeystate(vk, 1);
	if (vk == VK_RETURN) { toggleFullscreen(); }
	activeProg->syskeyDown(vk);
}

static void syskeyUp(int vk) {
	togglekeystate(vk, 0);
	activeProg->syskeyUp(vk);
}

static void keyChar(const char ch) {
	activeProg->keyChar(ch);
}

static void sysChar(const char ch) {
	activeProg->sysChar(ch);
}

static void setupMainProg() {
	mainProg.init = init;
	mainProg.tick = tick;
	mainProg.mouseMoved = mouseMoved;
	mainProg.mouseDown = mouseDown;
	mainProg.mouseUp = mouseUp;
	mainProg.mouseWheel = mouseWheel;
	mainProg.keyDown = keyDown;
	mainProg.keyUp = keyUp;
	mainProg.syskeyDown = syskeyDown;
	mainProg.syskeyUp = syskeyUp;
	mainProg.keyChar = keyChar;
	mainProg.sysChar = sysChar;
}
