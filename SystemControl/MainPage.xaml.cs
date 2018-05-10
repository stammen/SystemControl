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
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Foundation.Metadata;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;


// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace SystemControl
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
        }


        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            await LaunchDesktopExtension();
        }

        private async Task LaunchDesktopExtension()
        {
            if (ApiInformation.IsApiContractPresent("Windows.ApplicationModel.FullTrustAppContract", 1, 0))
            {
                try
                {
                    await FullTrustProcessLauncher.LaunchFullTrustProcessForCurrentAppAsync();
                }
                catch(Exception ex)
                {
                    Debug.WriteLine(ex.Message);
                }
            }
        }

        private async void BrightnessSlider_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            ValueSet message = new ValueSet();
            message.Add("Message", "Brightness");
            double value = e.NewValue / 100.0;
            message.Add("Value", value);
            var app = App.Current as App;
            var result = await app.SendMessage(message);
        }

        private async void SystemVolumeSlider_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            ValueSet message = new ValueSet();
            message.Add("Message", "SystemVolume");
            double value = e.NewValue / 100.0;
            message.Add("Value", value);
            var app = App.Current as App;
            var result = await app.SendMessage(message);
        }

        private async void GetApplications_Click(object sender, RoutedEventArgs e)
        {
            ValueSet message = new ValueSet();
            message.Add("Message", "GetApplications");
            var app = App.Current as App;
            var result = await app.SendMessage(message);

            try
            {
                if (result.ContainsKey("Applications"))
                {
                    var applications = result["Applications"] as string[];
                    applicationsComboBox.Items.Clear();
                    applicationsComboBox.Items.Add("Applications");
                    applicationsComboBox.SelectedIndex = 0;

                    foreach (var s in applications)
                    {
                        applicationsComboBox.Items.Add(s);
                    }
                }
            }
            catch(Exception ex)
            {
                Debug.WriteLine("Exception: " + ex.Message);
            }

        }

        private async void ComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (applicationsComboBox.SelectedIndex > 0)
            {
                ValueSet message = new ValueSet();
                message.Add("Message", "LaunchApplication");
                var name = applicationsComboBox.Items[applicationsComboBox.SelectedIndex] as string;
                message.Add("Name", name);
                var app = App.Current as App;
                var result = await app.SendMessage(message);
            }
        }
    }
}
