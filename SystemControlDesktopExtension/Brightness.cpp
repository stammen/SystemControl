//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "Brightness.h"
#include <PhysicalMonitorEnumerationAPI.h>
#include <HighLevelMonitorConfigurationAPI.h>

#include <ppltasks.h>
#include <wrl.h>
#include <wrl/client.h>

#include <dxgi1_6.h>
#include <d3d11_3.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <DirectXMath.h>

using namespace Microsoft::WRL;

std::mutex Brightness::s_mutex;

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		// Set a breakpoint on this line to catch Win32 API errors.
		throw Platform::Exception::CreateException(hr);
	}
}

HRESULT ApplyBrightnessToPhysicalMonitors(HMONITOR hMonitor, DWORD brightness)
{
	DWORD numPhysicalMonitors = 0;
	LPPHYSICAL_MONITOR pPhysicalMonitors = NULL;
	HRESULT hr = S_OK;

	// Get the number of physical monitors.
	BOOL bSuccess = GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &numPhysicalMonitors);

	if (bSuccess)
	{
		const DWORD cPhysicalMonitors = numPhysicalMonitors;

		// Allocate the array of PHYSICAL_MONITOR structures.
		pPhysicalMonitors = (LPPHYSICAL_MONITOR)malloc(cPhysicalMonitors * sizeof(PHYSICAL_MONITOR));

		if (pPhysicalMonitors != NULL)
		{
			memset(pPhysicalMonitors, 0, (sizeof(PHYSICAL_MONITOR) * cPhysicalMonitors));

			// Get the array of physical monitors
			bSuccess = GetPhysicalMonitorsFromHMONITOR(hMonitor, cPhysicalMonitors, pPhysicalMonitors);

			// Apply brightness setting to each physical monitor
			for (DWORD i = 0; i < cPhysicalMonitors && bSuccess; ++i)
			{
				bSuccess = SetMonitorBrightness(pPhysicalMonitors[i].hPhysicalMonitor, brightness);
			}
			// Close the monitor handles.
			DestroyPhysicalMonitors(cPhysicalMonitors, pPhysicalMonitors);

			// Free the array.
			free(pPhysicalMonitors);
		}
	}

	if (!bSuccess)
	{
		hr = GetLastError();
	}

	return hr;
}

HRESULT Brightness::SetBrightnessDXGI(DWORD brightness)
{
	std::lock_guard<std::mutex> lock(s_mutex);
	Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory = nullptr;
	ComPtr<IDXGIAdapter1> dxgiAdapter = nullptr;

	HRESULT hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
	if (SUCCEEDED(hr))
	{
		hr = dxgiFactory->EnumAdapters1(0, &dxgiAdapter);
	}

	if (SUCCEEDED(hr))
	{
		UINT adapterCount = 0;
		while (dxgiFactory->EnumAdapters1(adapterCount, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND && SUCCEEDED(hr))
		{
			UINT i = 0;
			ComPtr<IDXGIOutput> currentOutput;
			ComPtr<IDXGIOutput> bestOutput;
			float bestIntersectArea = -1;
			while (dxgiAdapter->EnumOutputs(i, &currentOutput) != DXGI_ERROR_NOT_FOUND)
			{
				DXGI_OUTPUT_DESC desc;
				hr = currentOutput->GetDesc(&desc);
				if (SUCCEEDED(hr) && desc.Monitor != NULL)
				{
					ApplyBrightnessToPhysicalMonitors(desc.Monitor, brightness);
				}
				i++;
			}

			++adapterCount;
		}
	}

	return hr;
}


BOOL CALLBACK MonitorEnumProc(HMONITOR monitor, HDC hdc, LPRECT rect, LPARAM param)
{
	ApplyBrightnessToPhysicalMonitors(monitor, (DWORD)param);
	return TRUE;
}

#if 0
HRESULT Brightness::SetBrightness(double value)
{
	HRESULT hr = S_OK;
	DWORD brightness = static_cast<DWORD>(value * 100.0);
	EnumDisplayMonitors(NULL, NULL, &MonitorEnumProc, brightness);
	return hr;
}
#else
HRESULT Brightness::SetBrightness(double value)
{
	DWORD brightness = static_cast<DWORD>(value * 100.0);
	return SetBrightnessDXGI(brightness);
}
#endif

