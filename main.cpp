#include "ofMain.h"
#include "ofApp.h"
#include "../resource.h"

//========================================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
//int main( ){
{
	if (DEBUG) {
		ofSetupOpenGL(960, 540, OF_WINDOW);
	}
	else {
		ofSetupOpenGL(1920, 1080, OF_FULLSCREEN);
	}
	
	HWND hwnd = ofGetWin32Window();
	HICON hMyIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hMyIcon);
	
	ofRunApp(new ofApp());
}
