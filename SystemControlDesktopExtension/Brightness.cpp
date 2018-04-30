#include "Brightness.h"
#include <PhysicalMonitorEnumerationAPI.h>
#include <HighLevelMonitorConfigurationAPI.h>

void ApplyBrightnessToPhysicalMonitors(HMONITOR hMonitor, DWORD brightness)
{
	DWORD numPhysicalMonitors = 0;
	LPPHYSICAL_MONITOR pPhysicalMonitors = NULL;

	// Get the number of physical monitors.
	BOOL bSuccess = GetNumberOfPhysicalMonitorsFromHMONITOR(
		hMonitor,
		&numPhysicalMonitors
	);

	if (bSuccess)
	{
		const DWORD cPhysicalMonitors = numPhysicalMonitors;

		// Allocate the array of PHYSICAL_MONITOR structures.
		pPhysicalMonitors = (LPPHYSICAL_MONITOR)malloc(
			cPhysicalMonitors * sizeof(PHYSICAL_MONITOR));

		if (pPhysicalMonitors != NULL)
		{
			memset(pPhysicalMonitors, 0, (sizeof(PHYSICAL_MONITOR) * cPhysicalMonitors));

			// Get the array.
			bSuccess = GetPhysicalMonitorsFromHMONITOR(
				hMonitor, cPhysicalMonitors, pPhysicalMonitors);

			// Use the monitor handles (not shown).
			for (DWORD i = 0; i < cPhysicalMonitors; ++i)
			{
				bool result = SetMonitorBrightness(pPhysicalMonitors[i].hPhysicalMonitor, brightness);
				auto error = GetLastError();
				if (result)
				{
					OutputDebugString(L"TRUE\n");
				}
			}
			// Close the monitor handles.
			bSuccess = DestroyPhysicalMonitors(
				cPhysicalMonitors,
				pPhysicalMonitors);

			// Free the array.
			free(pPhysicalMonitors);
		}
	}
}


BOOL CALLBACK MonitorEnumProc(HMONITOR monitor, HDC hdc, LPRECT rect, LPARAM param)
{
	ApplyBrightnessToPhysicalMonitors(monitor, (DWORD)param);
	return TRUE;
}


HRESULT Brightness::SetBrightness(double value)
{
	DWORD brightness = static_cast<DWORD>((value / 100.) * 100);
	EnumDisplayMonitors(NULL, NULL, &MonitorEnumProc, brightness);
	return S_OK;
}

