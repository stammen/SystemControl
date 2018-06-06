# Win32 System Control from a UWP App Sample

This sample demonstrates how a UWP application can call Win32 methods unavailable to a UWP application to control the following: 
* monitor brightness
* global system volume (now replaced with UWP code to set global system volume)
* enumerate applications installed on the user's system and selectively launch an application

The example uses a Win32 Desktop Extension and an AppService.

Note: This example will only work in Desktop scenarios

## Requirements

* Visual Studio 2017 with Windows Universal App Development package installed
* Windows SDK version 17134 (installed with Visual Studio 2017) or minimum SDK version 15063

## Running the Sample

* Open SystemControl.sln with Visual Studio 2017

* Select the Debug/x86 or Debug/x64 configuration. (Release/x86 and Release x/64 also work)

* Set the PackageProject project as the StartUp project

* Press F5 to build and run the solution. 


* Move the Brightness slider to change the Monitor brightness (note: setting monitor brightness is a very slow operation so moving the slider will take a while to get to the brightness setting.)

* Move the System Volume slider to change the global system volume.

* Click on the Get Applications Button. The combobox will be filled with all of the user's applications. Select an application from the combobox to launch the application.


##  Setup Instructions

In order to replicate this scenario you will need to do the following:

* Create a new Visual C# | Windows Universal | Blank App project (or use your existing UWP project). Set the Minimum platform version to 15063 and the max version to 17025 or 16299.

* Right click on the solution and select Add | New Project...

* Select Visual C++ | Windows Desktop | Windows Desktop Application. Name the project SystemControlDesktopExtension. 

* Right-click on your UWP project and select Build Dependencies | Project Dependencies...

* Select the SystemControlDesktopExtension project and click OK

* In the file SystemControlDesktopExtension.cpp delete all methods except for wWinMain. It should look like this:

```c++
#include "stdafx.h"
#include "SystemControlDesktopExtension.h"
#include "SystemControl.h"

// Global Variables:
HINSTANCE hInst; // current instance

[Platform::MTAThread]
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    hInst = hInstance; // Store instance handle in our global variable

    SystemControl sc;
    int result = sc.Run();
    return result;
}
```

* We are going to add a class called SystemControl to contain the logic of the Desktop Extension. Add a class called SystemControl to your project. 
Stub in the following public method in SystemControl.h and .cpp

```c++
int  Run();
```

### Adding UWP Support to the Desktop Extension

We need to add a few settings so our Desktop Extension can call UWP methods. Please do the following:

* Right-click on the SystemControlDesktopExtension project and select Properties. Make sure All Configurations and All Platforms are selected.

* Select C/C++ | General and add the following settings

* Set Additional #using Directories to 

```c++
$(VC_LibraryPath_VC_x86_Store)\references\;C:\Program Files (x86)\Windows Kits\10\UnionMetadata\\$(TargetPlatformVersion);C:\Program Files (x86)\Windows Kits\10\UnionMetadata;C:\Program Files (x86)\Windows Kits\10\References\Windows.Foundation.UniversalApiContract\3.0.0.0;C:\Program Files (x86)\Windows Kits\10\References\Windows.Foundation.FoundationContract\3.0.0.0;%(AdditionalUsingDirectories)
```

* Set Consume Windows Runtime Extension to Yes/(ZW)
    
* Add /Zc:twoPhase-  to the C/C++ | Commannd line
    
* Set C/C++ Precompiled Headers to Not Using Precompiled Headers
    
* Set Configuration Properties | General | Output Directory to $(SolutionDir)$(PlatformTarget)\$(Configuration)\

* Try to build the solution. All projects should build without an error.

### Adding the App Service to the UWP project

In order for the UWP App to be able to communicate with the Desktop Extension, we will use an [AppService](https://docs.microsoft.com/en-us/windows/uwp/launch-resume/convert-app-service-in-process)
to send messages between the two processes.


We will now add the code for the AppService to the UWP project.

* Add the following to App.xaml.cs

```csharp
using System.Threading.Tasks;
using Windows.ApplicationModel.AppService;
using Windows.ApplicationModel.Background;

private AppServiceConnection _appServiceConnection;
private BackgroundTaskDeferral _appServiceDeferral;

public async Task<ValueSet> SendMessage(ValueSet message)
{
    ValueSet result = new ValueSet();

    if (_appServiceConnection != null)
    {
        var response = await _appServiceConnection.SendMessageAsync(message);
        if (response.Status == AppServiceResponseStatus.Success)
        {
            result = response.Message;
        }
    }
    else
    {
        result.Add("Error", "AppService has no connection to Win32 process");
    }
    return result;
}

protected override void OnBackgroundActivated(BackgroundActivatedEventArgs args)
{
    base.OnBackgroundActivated(args);
    IBackgroundTaskInstance taskInstance = args.TaskInstance;
    AppServiceTriggerDetails appService = taskInstance.TriggerDetails as AppServiceTriggerDetails;
    _appServiceDeferral = taskInstance.GetDeferral();
    taskInstance.Canceled += OnAppServicesCanceled;
    _appServiceConnection = appService.AppServiceConnection;
    _appServiceConnection.RequestReceived += OnAppServiceRequestReceived;
    _appServiceConnection.ServiceClosed += AppServiceConnection_ServiceClosed;
}

private async void OnAppServiceRequestReceived(AppServiceConnection sender, AppServiceRequestReceivedEventArgs args)
{
    AppServiceDeferral messageDeferral = args.GetDeferral();
    ValueSet message = args.Request.Message;
    string text = message["Request"] as string;

    if ("Value" == text)
    {
        ValueSet returnMessage = new ValueSet();
        returnMessage.Add("Response", "True");
        await args.Request.SendResponseAsync(returnMessage);
    }
    messageDeferral.Complete();
}

private void OnAppServicesCanceled(IBackgroundTaskInstance sender, BackgroundTaskCancellationReason reason)
{
    _appServiceDeferral.Complete();
}

private void AppServiceConnection_ServiceClosed(AppServiceConnection sender, AppServiceClosedEventArgs args)
{
    _appServiceDeferral.Complete();
}
        
### Add the AppService connection to the Desktop Extension

* Modify SystemControl.h to the following:

```c++
#include <ppltasks.h>

class SystemControl
{
public:
    SystemControl();
    ~SystemControl();
    int Run();

private:
    void SystemControlThread();
    Concurrency::task<Windows::ApplicationModel::AppService::AppServiceConnectionStatus> ConnectToAppService(Platform::String^ serviceName, Platform::String^ packageFamilyName);
    void OnRequestReceived(Windows::ApplicationModel::AppService::AppServiceConnection^ sender, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs^ args);
    void OnAppServiceClosed(Windows::ApplicationModel::AppService::AppServiceConnection^ sender, Windows::ApplicationModel::AppService::AppServiceClosedEventArgs^ args);
    Windows::Foundation::Collections::ValueSet^ HandleRequest(Windows::Foundation::Collections::ValueSet^ message);

    Windows::ApplicationModel::AppService::AppServiceConnection^ m_appService;
    bool m_quitting;
    bool m_appServiceConnected;
};
```

* Modify SystemControl.cpp to the following:

```c++
#include "SystemControl.h"
#include <string>
#include <sstream>     
#include <functional>

#include <collection.h>  
#include <thread>        
#include <ppltasks.h>

using namespace concurrency;
using namespace Windows::ApplicationModel::AppService;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::System;
using namespace std::placeholders;

SystemControl::SystemControl()
{

}

SystemControl::~SystemControl()
{
    m_quitting = true;
}

void SystemControl::SystemControlThread()
{
    m_quitting = false;
    m_appServiceConnected = false;

    try
    {
        auto task = ConnectToAppService(L"com.stammen.systemcontrol.appservice", Windows::ApplicationModel::Package::Current->Id->FamilyName);
        auto result = task.get(); // blocks until Connection task completes

        if (result == AppServiceConnectionStatus::Success)
        {
            // keep running until it is time to exit
            while (!m_quitting)
            {
                Sleep(1000);
            }
        }
    }
    catch (Platform::Exception^ ex)
    {
        m_quitting = true;
        OutputDebugString(ex->Message->Data());
    }
}

int SystemControl::Run()
{
    // run screen capture on separate thread
    std::thread t(&SystemControl::SystemControlThread, this);
    t.join();
    return 0;
}

Concurrency::task<AppServiceConnectionStatus> SystemControl::ConnectToAppService(Platform::String^ serviceName, Platform::String^ packageFamilyName)
{
    m_appService = ref new AppServiceConnection();
    m_appService->RequestReceived += ref new TypedEventHandler<AppServiceConnection^, AppServiceRequestReceivedEventArgs^>(std::bind(&SystemControl::OnRequestReceived, this, _1, _2));
    m_appService->ServiceClosed += ref new TypedEventHandler<AppServiceConnection^, AppServiceClosedEventArgs^>(std::bind(&SystemControl::OnAppServiceClosed, this, _1, _2));

    // Here, we use the app service name defined in the app service provider's Package.appxmanifest file in the <Extension> section.
    m_appService->AppServiceName = serviceName;

    // Use Windows.ApplicationModel.Package.Current.Id.FamilyName within the app service provider to get this value.
    m_appService->PackageFamilyName = packageFamilyName;

    return create_task(m_appService->OpenAsync()).then([this](AppServiceConnectionStatus status)
    {
        if (status != AppServiceConnectionStatus::Success)
        {
            m_appService = nullptr;
        }
        else
        {
            OutputDebugString(L"Connected to AppService.\n");
            m_appServiceConnected = true;
        }

        return status;
    });
}

void SystemControl::OnRequestReceived(AppServiceConnection^ sender, AppServiceRequestReceivedEventArgs^ args)
{
    // Get a deferral because we use an async API below to respond to the message
    // and we don't want this call to get cancelled while we are waiting.
    auto messageDeferral = args->GetDeferral();
    auto request = args->Request->Message;
    ValueSet^ response = HandleRequest(request);

    if (request->HasKey("Message"))
    {
        auto message = dynamic_cast<Platform::String^>(request->Lookup("Message"));
        if (message == "Hello")
        {
            auto name = dynamic_cast<Platform::String^>(request->Lookup("Name"));
            response->Insert("Status", "OK");
            response->Insert("Message", "Hello " + name + "!");
        }
        else if (message == "Quit")
        {
            m_quitting = true;
            response->Insert("Status", "OK");
        }
    }
    else
    {
        response->Insert("Error", "Missing Message parameter");
    }

    create_task(args->Request->SendResponseAsync(response)).then([messageDeferral](AppServiceResponseStatus response)
    {
        messageDeferral->Complete();
    });
}

ValueSet^ SystemControl::HandleRequest(ValueSet^ message)
{
    ValueSet^ response = ref new ValueSet();

    return response;
}


void SystemControl::OnAppServiceClosed(Windows::ApplicationModel::AppService::AppServiceConnection^ sender, Windows::ApplicationModel::AppService::AppServiceClosedEventArgs^ args)
{
    m_quitting = true;
}
```

Note: change com.stammen.systemcontrol.appservice to the name you specified in your package.appmanifest!

This code will keep the Desktop Extension running until it is told to quit or the connection to the AppService is closed. 

### Add a Packaging Project to Package the Win32 .exe with the UWP App

We will use a Packaging Project to Package the Win32 .exe with the UWP App. The Packaging Project will add both the Win32 .exe and the UWP App to the AppX package.
Add a Packaging Project to your solution.

* Right-click on the Solution and select Add | New Project...

* Select Visual C# | Windows Universal  | Windows Application Packaging Project

* Name the project PackageProject and click OK

* Set PackageProject as the Startup Project

* In the PackageProject right click on Applications and select Add Reference

* Select both the UWP Project and the Win32 project 

* Expand the Applications folder and right-click on the UWP project. Select Set as EntryPoint.

* Right-click on the Package.appxmanifest file in the PackageProject. Select View Code.

* Replace all instances of PackageProject with the name of your App. (It this example it is SystemControl)

* Modify the Package section to add the Desktop xmlns attribute

```xml
<Package
  xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10"
  xmlns:mp="http://schemas.microsoft.com/appx/2014/phone/manifest"
  xmlns:uap="http://schemas.microsoft.com/appx/manifest/uap/windows10"
  xmlns:rescap="http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities"
  xmlns:desktop="http://schemas.microsoft.com/appx/manifest/desktop/windows10" 
  IgnorableNamespaces="uap mp rescap desktop">
 ```

* Add the Desktop Extension to the Application Section

```xml
  <Applications>
    <Application Id="App"
      Executable="$targetnametoken$.exe"
      EntryPoint="$targetentrypoint$">
      <uap:VisualElements
        DisplayName="SystemControl"
        Description="SystemControl"
        BackgroundColor="transparent"
        Square150x150Logo="Images\Square150x150Logo.png"
        Square44x44Logo="Images\Square44x44Logo.png">
        <uap:DefaultTile
          Wide310x150Logo="Images\Wide310x150Logo.png" />
      </uap:VisualElements>
      <Extensions>
        <desktop:Extension Category="windows.fullTrustProcess" Executable="SystemControlDesktopExtension/SystemControlDesktopExtension.exe" />
      </Extensions>
    </Application>
  </Applications>
 ```
 
* Add the AppService to the Application section

```xml
  <Extensions>
    <uap:Extension Category="windows.appService">
      <uap:AppService Name="com.stammen.systemcontrol.appservice" />
    </uap:Extension>
  </Extensions>
```

* Your Application section should now look something like this:

```xml
  <Applications>
    <Application Id="App"
      Executable="$targetnametoken$.exe"
      EntryPoint="$targetentrypoint$">
      <uap:VisualElements
        DisplayName="SystemControl"
        Description="SystemControl"
        BackgroundColor="transparent"
        Square150x150Logo="Images\Square150x150Logo.png"
        Square44x44Logo="Images\Square44x44Logo.png">
        <uap:DefaultTile
          Wide310x150Logo="Images\Wide310x150Logo.png" />
      </uap:VisualElements>
      <Extensions>
          <uap:Extension Category="windows.appService">
            <uap:AppService Name="com.stammen.systemcontrol.appservice" />
          </uap:Extension>
        <desktop:Extension Category="windows.fullTrustProcess" Executable="SystemControlDesktopExtension/SystemControlDesktopExtension.exe" />
      </Extensions>
    </Application>
  </Applications>
```

Note: change com.stammen.systemcontrol.appservice to a name that makes sense for your app!

The Packaging Project is now configured to correctly build your application.


### Modify the UWP app to launch the Desktop Extension

In order to use FullTrustProcessLauncher we need to add a reference to the Windows Desktop Extension for the UWP

We will use the UWP Method FullTrustProcessLauncher to launch the Desktop Extension app from the UWP app. In order to use FullTrustProcessLauncher we need to do the following:

* In the UWP project, right-click on References and select Add Reference. 

* Click on the Universal Windows tab. Select Extensions

* Carefully select the Windows Desktop Extension for the UWP (version 10.0.15063). Click on OK.

* Add the following methods to MainPage.xaml.cs

```csharp
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Foundation.Metadata;

protected override async void OnNavigatedTo(NavigationEventArgs e)
{
    await LaunchDesktopExtension();
}

private async Task LaunchDesktopExtension()
{
    if (ApiInformation.IsApiContractPresent("Windows.ApplicationModel.FullTrustAppContract", 1, 0))
    {
        await FullTrustProcessLauncher.LaunchFullTrustProcessForCurrentAppAsync();
    }
}
```

In order to debug the Desktop Extension, edit the Debugging Properties of the UWP Project.

* Right-click on the UWP project and select Properties

* In the Debugging tab, set Background task process to Native Only

* Set a breakpoint in SystemControlDesktopExtension.cpp

* Build and run the app. The app should stop at the breakpoint.

### Send a Message from the UWP App to the Desktop Extension

We will now send a message to the Desktop Extension. Add a button to MainPage.xaml and create a Button click handler

* In MainPage.xaml add

```xml
<Grid>
	<StackPanel Orientation="Vertical" Margin="10">
		<Button Click="Button_Click" Content="Send Message" />
		<TextBlock x:Name="resultText" Margin="10, 10"/>
	</StackPanel>
</Grid>
```

 * In MainPage.xaml.cs add the following method:
 
 ```csharp
private async void Button_Click(object sender, RoutedEventArgs e)
{
    ValueSet message = new ValueSet();
    message.Add("Message", "Hello");
    message.Add("Name", "Jim");
    var app = App.Current as App;
    var result = await app.SendMessage(message);
    if(result.ContainsKey("Status"))
    {
        string status = result["Status"] as string;
        if(status == "OK")
        {
            resultText.Text = result["Message"] as string;
        }
    }
}
```

* Build and run the app. You should be able to send to and receive a message from the Desktop Extension via the AppService.

Note: The Title Bar of your App may be Black. We will update these instructions with a fix for this.


### Adding Win32 API Functions to the Desktop Extension

Take a look at SystemControl.cpp, SystemVolume.cpp, Brightness.cpp, Applications.cpp in this repo for how you can add Win32 API function via the AppService and Desktop Extension
 
