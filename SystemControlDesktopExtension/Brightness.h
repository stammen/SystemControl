#pragma once

#include <mutex>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class Brightness
{
public:
	static HRESULT SetBrightness(double value);

private:
	static HRESULT DoSetBrightness();
	static HRESULT SetBrightnessDXGI(DWORD brightness);
	static std::mutex s_mutex;
};
