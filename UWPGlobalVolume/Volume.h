#pragma once

#include "VolumeImpl.h"

Windows::Foundation::IAsyncOperation<bool>^ Start(Platform::String^ credentials_file_path);

namespace UWPGlobalVolume
{
    public ref class Volume sealed
    {
    public:
        Volume();
        Windows::Foundation::IAsyncOperation<bool>^ SetVolume(float volume);
        Windows::Foundation::IAsyncOperation<float>^ GetVolume();
        Windows::Foundation::IAsyncOperation<bool>^ SetMute(bool isMuted);
        Windows::Foundation::IAsyncOperation<bool>^ GetMute();
        void RegisterVolumeChangedNotify(VolumeChangedHandler^ volumeChangedAction);

    private:
        Microsoft::WRL::ComPtr<VolumeImpl> m_volumeImpl;
    };
}
