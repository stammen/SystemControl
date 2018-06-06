#include "pch.h"
#include "VolumeImpl.h"
#include <functional>

using namespace UWPGlobalVolume;
using namespace Microsoft::WRL;
using namespace Windows::Media::Devices;
using namespace std::placeholders;

VolumeImpl::VolumeImpl()
{
    m_event = CreateEvent(nullptr, false, false, nullptr);
    // register for changes in Default audio device
    m_token = MediaDevice::DefaultAudioRenderDeviceChanged += ref new Windows::Foundation::TypedEventHandler<Platform::Object ^, DefaultAudioRenderDeviceChangedEventArgs ^>(std::bind(&VolumeImpl::OnDefaultAudioCaptureDeviceChanged, this, _1, _2));
}

VolumeImpl::~VolumeImpl()
{
    CloseHandle(m_event);
    MediaDevice::DefaultAudioCaptureDeviceChanged -= m_token;
}

HRESULT VolumeImpl::InitializeVolumeInterface()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_volumeInterface != nullptr)
    {
        return S_OK;
    }
    
    ComPtr<IActivateAudioInterfaceAsyncOperation> asyncOp;
    m_result = S_OK;
    auto id = MediaDevice::GetDefaultAudioRenderId(AudioDeviceRole::Default);
    HRESULT hr = ActivateAudioInterfaceAsync(id->Data(), __uuidof(IAudioEndpointVolume), nullptr, this, &asyncOp);
    if (SUCCEEDED(hr))
    {
        WaitForSingleObject(m_event, 1000);
    }

    return m_result;
}

void VolumeImpl::OnDefaultAudioCaptureDeviceChanged(Platform::Object^ sender, DefaultAudioRenderDeviceChangedEventArgs ^ args)
{
    // force next Set/GetVolume call to reinitialize the IAudioEndpointInterface
    std::lock_guard<std::mutex> lock(m_mutex);
    m_volumeInterface.Reset();
}


bool VolumeImpl::SetVolume(float volume)
{
    HRESULT hr = InitializeVolumeInterface();
    if (SUCCEEDED(hr) && m_volumeInterface != nullptr)
    {
        hr = m_volumeInterface->SetMasterVolumeLevelScalar(volume, NULL);
    }
    return SUCCEEDED(hr);
}


float VolumeImpl::GetVolume()
{
    float volume = 0.0f;
    HRESULT hr = InitializeVolumeInterface();
    if (SUCCEEDED(hr) && m_volumeInterface != nullptr)
    {
        hr = m_volumeInterface->GetMasterVolumeLevelScalar(&volume);
    }
    return volume;

}

//
//  ActivateCompleted()
//
//  Callback implementation of ActivateAudioInterfaceAsync function.  This will be called on MTA thread
//  when results of the activation are available.
//
HRESULT VolumeImpl::ActivateCompleted(IActivateAudioInterfaceAsyncOperation *operation)
{
    HRESULT hr = S_OK;
    HRESULT hrActivateResult = S_OK;
    ComPtr<IUnknown> punkVolumeInterface;

    // Check for a successful activation result
    hr = operation->GetActivateResult(&hrActivateResult, &punkVolumeInterface);
    if (FAILED(hr))
    {
        if (FAILED(hrActivateResult))
        {
            hr = hrActivateResult;
        }

        goto exit;
    }

    punkVolumeInterface.CopyTo(&m_volumeInterface);
    if (nullptr == m_volumeInterface)
    {
        hr = E_NOINTERFACE;
        goto exit;
    }

exit:
    SetEvent(m_event);
    m_result = hr;
    // Need to return S_OK
    return S_OK;
}