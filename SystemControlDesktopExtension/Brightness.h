//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
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
