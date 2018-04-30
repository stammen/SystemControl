#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class Brightness
{
public:
	static HRESULT SetBrightness(double value);
};
