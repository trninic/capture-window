

#include "stdafx.h"
#include <iostream>
#include <ole2.h>
#include <olectl.h>
#include <sstream>
#include <Shlwapi.h>
#include <stdlib.h>

#include "easy_macros.h"
#include "init_console.h"
#include "initialize.root_win.cpp"

#define DEBUG_TEST_CAPTURE 0
#if DEBUG_TEST_CAPTURE
	#include "save_bitmap.cpp"
#endif

#include "mag_capture.cpp"
#include "common_functions.cpp"
#include "capture-window_common_structs.h"


int main(int argc, char *argv[])
{
	initialize_rootwin(); // We need to init window system for it to work...
	magCapture::Initialize(); // Init the capture system 

#if DEBUG_TEST_CAPTURE

	HWND hwndCapTarget = (HWND)0x00B40830;
	HBITMAP bitmapCapture = magCapture::CaptureWindow(hwndCapTarget);

	saveBitmap("test.bmp", bitmapCapture);

	
#else
	if (argc != 3) return EXIT_FAILURE;

	
	HWND hwndCaller = StringToHwnd(argv[1]);  // Get the HWND of the caller process, we send the captured data to the HWND
	HWND hwndCapTarget = StringToHwnd(argv[2]);  // Get the target window to capture

	MAGIMAGEHEADER magInfo; 
	byte *capturedPixels = magCapture::CaptureWindowPixels(hwndCapTarget, &magInfo);

	if (!capturedPixels) return EXIT_FAILURE; 


	// Send the captured pixels to the caller process (using it's hwndCaller)
	COPYDATASTRUCT stCOPYDATASTRUCT;
	ZeroMemory(&stCOPYDATASTRUCT, sizeof(COPYDATASTRUCT));

	magCapturedData *imageSendData = (magCapturedData *)malloc(CAPTURED_DATA_SIZE(magInfo.cbSize));
	imageSendData->height = magInfo.height;
	imageSendData->width = magInfo.width;
	imageSendData->cbsize = magInfo.cbSize;
	memcpy(imageSendData->pixels, capturedPixels, magInfo.cbSize);

	ZeroMemory(&stCOPYDATASTRUCT, sizeof(COPYDATASTRUCT));
	stCOPYDATASTRUCT.cbData = CAPTURED_DATA_SIZE(magInfo.cbSize);
	stCOPYDATASTRUCT.lpData = imageSendData;
	SendMessage(hwndCaller, WM_COPYDATA, COMMON_STRUCTS_CAPTURE_PIXELSDATA, (LPARAM)&stCOPYDATASTRUCT);



#endif
	magCapture::UnInitialize();

    return EXIT_SUCCESS;
}







// ------------------------------------- WINDOW PROCESSING ----------------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}
