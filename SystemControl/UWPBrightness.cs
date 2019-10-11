//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************


using System;
using System.Threading.Tasks;
using Windows.Graphics.Display;
using Windows.UI.StartScreen;

namespace Utils
{
    sealed class UWPBrightness
    {
        private BrightnessOverride m_brightnessOverride = null;

        public UWPBrightness()
        {
            m_brightnessOverride = BrightnessOverride.GetForCurrentView();
        }

        ~UWPBrightness()
        {
            if (m_brightnessOverride != null)
            {
                m_brightnessOverride.StopOverride();
            }
        }

        public bool SetBrightness(double value)
        {
            if(!BrightnessOverride.GetForCurrentView().IsSupported)
            {
                return false;
            }
            
            if(m_brightnessOverride == null)
            {
                m_brightnessOverride = BrightnessOverride.GetForCurrentView();
            }

            m_brightnessOverride.SetBrightnessLevel(value, DisplayBrightnessOverrideOptions.None);
            m_brightnessOverride.StartOverride();

            return true;
        }

    }
}

