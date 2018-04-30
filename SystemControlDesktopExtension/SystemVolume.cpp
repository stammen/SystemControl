//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "SystemVolume.h"
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <wrl.h>
#include <wrl/client.h>

using namespace Microsoft::WRL;

HRESULT ChangeVolume(double nVolume)
{
    HRESULT hr = S_OK;

    ComPtr<IMMDevice> defaultDevice;
    ComPtr<IMMDeviceEnumerator> deviceEnumerator;
    ComPtr<IAudioEndpointVolume> endpointVolume;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);

    if (SUCCEEDED(hr))
    {
        hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    }

    if (SUCCEEDED(hr))
    {
        hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);
    }

    if (SUCCEEDED(hr))
    {
        hr = endpointVolume->SetMasterVolumeLevelScalar((float)nVolume, NULL);
    }

    return hr;
}

HRESULT SystemVolume::SetSystemVolume(double value)
{
    HRESULT hr = S_OK;
    ChangeVolume(value);
    return hr;
}

