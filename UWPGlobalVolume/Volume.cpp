#include "pch.h"
#include "Volume.h"
#include "VolumeImpl.h"
#include <ppltasks.h>

using namespace Concurrency;
using namespace Microsoft::WRL;
using namespace UWPGlobalVolume;

Volume::Volume()
{
}

Windows::Foundation::IAsyncOperation<bool>^ Volume::SetVolume(float volume)
{
    return create_async([this, volume]
    {
        if (m_volumeImpl == nullptr)
        {
            // Create a new WASAPI capture instance
            m_volumeImpl = Make<VolumeImpl>();
        }

        return m_volumeImpl->SetVolume(volume);
    });
}

Windows::Foundation::IAsyncOperation<float>^ Volume::GetVolume()
{
    return create_async([this]
    {
        if (m_volumeImpl == nullptr)
        {
            // Create a new WASAPI capture instance
            m_volumeImpl = Make<VolumeImpl>();
        }

        return m_volumeImpl->GetVolume();
    });
}

Windows::Foundation::IAsyncOperation<bool>^ Volume::SetMute(bool isMuted)
{
    return create_async([this, isMuted]
    {
        if (m_volumeImpl == nullptr)
        {
            // Create a new WASAPI capture instance
            m_volumeImpl = Make<VolumeImpl>();
        }

        return m_volumeImpl->SetMute(isMuted);
    });
}

Windows::Foundation::IAsyncOperation<bool>^ Volume::GetMute()
{
    return create_async([this]
    {
        if (m_volumeImpl == nullptr)
        {
            // Create a new WASAPI capture instance
            m_volumeImpl = Make<VolumeImpl>();
        }

        return m_volumeImpl->GetMute();
    });
}

void Volume::RegisterVolumeChangedNotify(VolumeChangedHandler^ volumeChangedAction)
{
    if (m_volumeImpl == nullptr)
    {
        m_volumeImpl = Make<VolumeImpl>();
    }

    m_volumeImpl->VolumeChangedAction = volumeChangedAction;
}



