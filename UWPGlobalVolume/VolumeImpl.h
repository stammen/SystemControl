#pragma once

#include <wrl\implements.h>
#include <mmdeviceapi.h>
#include <Endpointvolume.h>
#include <mutex>

namespace UWPGlobalVolume
{
    // Primary WASAPI Capture Class
    class VolumeImpl :
        public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags< Microsoft::WRL::ClassicCom>, Microsoft::WRL::FtmBase, IActivateAudioInterfaceCompletionHandler >
    {
    public:
        VolumeImpl();
        ~VolumeImpl();

        bool SetVolume(float volume);
        float GetVolume();

    private:
        std::mutex m_mutex;
        HRESULT m_result;
        HANDLE m_event;
        Windows::Foundation::EventRegistrationToken m_token;

        HRESULT InitializeVolumeInterface();

        void OnDefaultAudioCaptureDeviceChanged(Platform::Object^ sender, Windows::Media::Devices::DefaultAudioRenderDeviceChangedEventArgs ^ args);

        // IActivateAudioInterfaceCompletionHandler
        STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation *operation);
        Microsoft::WRL::ComPtr<IAudioEndpointVolume> m_volumeInterface;
    };
}
