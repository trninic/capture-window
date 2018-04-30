

#include "stdafx.h"


#include <iostream>
#include <ole2.h>
#include <olectl.h>
#include <sstream>
#include <Shlwapi.h>


#include "easy_macros.h"
#include "init_console.h"



#include "initialize.root_win.cpp"

#define ONLY_CAPTURE_AND_SAVE 0
#if ONLY_CAPTURE_AND_SAVE
	#include "save_bitmap.cpp"
	#include "bitblt_capture.cpp"
#endif


#include "mag_capture.cpp"
#include "common_functions.cpp"
#include "capture-window_common_structs.h"



int main(int argc, char *argv[])
{
	initialize_rootwin(); // We need to init window system for it to work...

	magCapture::Initialize(); // Init the capture system 




#if ONLY_CAPTURE_AND_SAVE
	if (argc != 2) return 0;

	hwndCapTarget = StringToHwnd(argv[1]);
	HBITMAP bitmapCapture = magCapture::CaptureWindow(hwndCapTarget);

	saveBitmap("test.bmp", bitmapCapture);
#else
	if (argc != 3) return 0;

	
	HWND hwndCaller = StringToHwnd(argv[1]);  // Get the HWND of the caller process, we send the captured data to the HWND
	HWND hwndCapTarget = StringToHwnd(argv[2]);  // Get the target window to capture

	MAGIMAGEHEADER magInfo; 
	byte *capturedPixels = magCapture::CaptureWindowPixels(hwndCapTarget, &magInfo);

	if (!capturedPixels) return 0; // On error exit


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

    return 0;
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
