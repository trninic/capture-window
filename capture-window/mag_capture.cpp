#pragma once
#include <Magnification.h>
#pragma comment (lib,"Magnification.lib")
#include <vector>




namespace magCapture
{



	namespace for_internal_use
	{

		byte *returnedPixelData = nullptr;
		MAGIMAGEHEADER returnedSrcheader;

		void Callback_GetPixelsData(HWND hwnd, void *srcdata, MAGIMAGEHEADER srcheader, void *destdata, MAGIMAGEHEADER destheader,
			RECT unclipped, RECT clipped, HRGN dirty)
		{

			returnedPixelData = (byte*)srcdata;
			returnedSrcheader = srcheader;
		}
	}






	bool Initialize()
	{ // Call this function only once during the execution!
		return MagInitialize() ? true : false;

		
	}


	bool UnInitialize()
	{ // Call this fucntion when you dont need to do more captures anymore
		return MagUninitialize() ? true : false;
	}




	class magCapture
	{
	private:
		HWND hwndMag;
		HWND hwndHost;




		bool IsWindowIsAboveCaptureRegion(HWND hwndWindow, HWND hwndBaseWindow)
		{
			RECT rectWindow;
			ZeroMemory(&rectWindow, sizeof(RECT));
			if (!GetWindowRect(hwndWindow, &rectWindow)) return false;
			if (
				(
				(rectWindow.left >= rectCapture.left && rectWindow.left < rectCapture.right)
					||
					(rectWindow.right <= rectCapture.right && rectWindow.right > rectCapture.left)
					||
					(rectWindow.left <= rectCapture.left && rectWindow.right >= rectCapture.right)
					)
				&&
				(
				(rectWindow.top >= rectCapture.top && rectWindow.top < rectCapture.bottom)
					||
					(rectWindow.bottom <= rectCapture.bottom && rectWindow.bottom > rectCapture.top)
					||
					(rectWindow.top <= rectCapture.top && rectWindow.bottom >= rectCapture.bottom)
					)
				)
				return true;
			else
				return false;
		}

		

	public:

		// Object parameters:
		HWND hwndTarget = NULL; // The target window to capture. Use NULL for full screen capture
		RECT rectCapture; // The capture area - what area in the screen we need to capture
		std::vector<HWND> vecExcludeWins; // Dont include in the capture the windows listed in the vector

		MAGIMAGEHEADER magBtsInfo;
		byte *capturedPixels = nullptr;

		// Object functions:

		bool StartUp()
		{ // Call this function only after you set the Object parameters
		  // Call this function to startup the object

		  // Create the host window that will store the mag child window
			hwndHost = CreateWindowEx(0x08000000 | 0x080000 | 0x80 | 0x20, g_rootwin_szClassName, NULL, 0x80000000,
				NULL, NULL, NULL, NULL, NULL, NULL, GetModuleHandle(NULL), NULL);
			if (!hwndHost) return false;
			SetLayeredWindowAttributes(hwndHost, (COLORREF)0, (BYTE)255, (DWORD)0x02);
			// Create the mag child window inside the host window
			hwndMag = CreateWindow(WC_MAGNIFIER, TEXT("MagnifierWindow"),
				WS_CHILD /*| MS_SHOWMAGNIFIEDCURSOR*/  /*| WS_VISIBLE*/,
				NULL, NULL, rectCapture.right - rectCapture.left, rectCapture.bottom - rectCapture.top,
				hwndHost, NULL, GetModuleHandle(NULL), NULL);
			if (!MagSetImageScalingCallback(hwndMag, (MagImageScalingCallback)for_internal_use::Callback_GetPixelsData)) return false; // ERROR

			magBtsInfo.width = -1;
			magBtsInfo.height = -1;
			return 0;
		}

		void Shutdown()
		{ // Call this function to shutdown the object
			vecExcludeWins.resize(0);
			MagSetImageScalingCallback(hwndMag, NULL);
			DestroyWindow(hwndHost);
		}

		void Capture_ReForceOnlyTarget()
		{ // Call this if you want that only the window will captured and not windows above the window
		  // Reset the list of window to exclude
			vecExcludeWins.resize(0);
			vecExcludeWins.reserve(24);

			bool isMaximaized = false;
			WINDOWPLACEMENT windowplacment;
			ZeroMemory(&windowplacment, sizeof(WINDOWPLACEMENT));
			if (GetWindowPlacement(hwndTarget, &windowplacment) && windowplacment.showCmd == SW_SHOWMAXIMIZED) isMaximaized = true;

		
			for (HWND hwndTopPriority = GetTopWindow(NULL); hwndTopPriority != hwndTarget; hwndTopPriority = GetNextWindow(hwndTopPriority, GW_HWNDNEXT))
			{
				if (!IsWindowVisible(hwndTopPriority)) continue;
				if (!isMaximaized && !IsWindowIsAboveCaptureRegion(hwndTopPriority, hwndTarget)) continue;
				vecExcludeWins.push_back(hwndTopPriority);
				if (vecExcludeWins.size() >= 24) break;
			}

			MagSetWindowFilterList(hwndMag, MW_FILTERMODE_EXCLUDE, vecExcludeWins.size(), &vecExcludeWins[0]);
		}

		bool RectCapture_InitializeForWindow()
		{ //This function will set the rectCapture parameter according to the size and position of the window.
		  // Use this function only after the object know the hwndTarget parameter
			return GetWindowRect(hwndTarget, &rectCapture) ? true : false; // return false on error
		}

		bool Capture()
		{ // this function is for capture screen according to rectCapture
			for_internal_use::returnedPixelData = nullptr;
			capturedPixels = nullptr;
			MagSetWindowSource(hwndMag, rectCapture);
			if (!for_internal_use::returnedPixelData) return false; // error
			magBtsInfo = for_internal_use::returnedSrcheader;
			capturedPixels = for_internal_use::returnedPixelData;
			for_internal_use::returnedPixelData = nullptr;
			return true;
		}

	};

	byte* CaptureWindowPixels(HWND hwndTarget, MAGIMAGEHEADER* magInfo)
	{
		magCapture windowCapture;
		HBITMAP outputBitmap = NULL;

		// Set object parameters
		windowCapture.hwndTarget = hwndTarget;
		windowCapture.RectCapture_InitializeForWindow();

		// Startup the object
		windowCapture.StartUp();

		// Capture
		windowCapture.Capture_ReForceOnlyTarget(); // Force that only the window will captured

		windowCapture.Capture();

		windowCapture.Shutdown();

		*magInfo = windowCapture.magBtsInfo;

		return windowCapture.capturedPixels;

	}

	HBITMAP CreateHbitmabFromPixels(byte* capturedPixels, MAGIMAGEHEADER& magInfo)
	{
		if (!capturedPixels) return NULL;
		// Convert it to HBITMAP object
		BITMAPINFO bi;
		ZeroMemory(&bi, sizeof(BITMAPINFO));
		bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth = magInfo.width;
		bi.bmiHeader.biHeight = -magInfo.height;  //negative so (0,0) is at top left
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biBitCount = 32;
		byte *bitmapPixels;
		HBITMAP outputBitmap = CreateDIBSection(GetDC(NULL), &bi, DIB_RGB_COLORS, (void**)&bitmapPixels, NULL, 0);
		memcpy(bitmapPixels, capturedPixels, sizeof(byte)*magInfo.cbSize);
		return outputBitmap;
	}

	HBITMAP CaptureWindow(HWND hwndTarget)
	{ // On error: return NULL
		HBITMAP outputBitmap = NULL;
		MAGIMAGEHEADER magInfo;
		byte *capturedPixels = CaptureWindowPixels(hwndTarget, &magInfo);
		pp magInfo.width ss magInfo.height ee;
		return CreateHbitmabFromPixels(capturedPixels, magInfo);

	}

}

