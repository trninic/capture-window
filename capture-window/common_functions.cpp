
HWND StringToHwnd(std::string strPointer)
{
	std::stringstream ssTmp;
	ssTmp << strPointer;
	long long unsigned int tmp;
	ssTmp >> std::hex >> tmp;
	HWND outHwnd = reinterpret_cast<HWND>(tmp);
	return outHwnd;
}


