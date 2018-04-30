# Win32 System Control from a UWP App Sample

This sample demonstrates how a UWP application can call Win32 methods unavailable to a UWP application to control monitor brightness and global system volume. 
The example uses a Win32 Desktop Extension and an AppService.

Note: This example will only work in Desktop scenarios

## Requirements

* Visual Studio 2017 with Windows Universal App Development package installed
* Windows SDK version 17025 (installed with Visual Studio 2017) or minimum SDK version 15063

## Running the Sample

* Open SystemControl.sln with Visual Studio 2017

* Select the Debug/x86 or Debug/x64 configuration. (Release/x86 and Release x/64 also work)

* Set the SystemControl project as the StartUp project

* Press F5 to build and run the solution. 


* Move the Brightness slider to change the Monitor brightness (note: setting monitor brightness is a very slow operation so moving the slider will take a while to get to the brightness setting.)

* Move the System Volume slider to change the global system volume.


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

    * Set Additional #using Directories to $(VC_ReferencesPath_x86)\store\references;C:\Program Files (x86)\Windows Kits\10\UnionMetadata;C:\Program Files (x86)\Windows Kits\10\References\Windows.Foundation.UniversalApiContract\3.0.0.0;C:\Program Files (x86)\Windows Kits\10\References\Windows.Foundation.FoundationContract\3.0.0.0;%(AdditionalUsingDirectories)
    
    * Set Consume Windows Runtime Extension to Yes/(ZW)
    
    * Add /Zc:twoPhase-  to the C/C++ | Commannd line
    
    * Set C/C++ Precompiled Headers to Not Using Precompiled Headers
    
    * Set Configuration Properties | General | Output Directory to $(SolutionDir)$(PlatformTarget)\$(Configuration)\

* Try to build the solution. All projects should build without an error.

### Adding the App Service to the UWP project

In order for the UWP App to be able to communicate with the Desktop Extension, we will use an [AppService](https://docs.microsoft.com/en-us/windows/uwp/launch-resume/convert-app-service-in-process)
to send messages between the two processes.

* In the UWP project, right-click on the file Package.appxmanifest and select View Code

* Modify the Package tag to the following:

```xml
<Package
  xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10"
  xmlns:mp="http://schemas.microsoft.com/appx/2014/phone/manifest"
  xmlns:uap="http://schemas.microsoft.com/appx/manifest/uap/windows10"
  xmlns:desktop="http://schemas.microsoft.com/appx/manifest/desktop/windows10" 
  xmlns:rescap="http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities" 
  IgnorableNamespaces="uap mp desktop rescap">
```

* Add the AppService and Desktop Extension to the Application section

```xml
  <Extensions>
    <uap:Extension Category="windows.appService">
      <uap:AppService Name="com.stammen.systemcontrol.appservice" />
    </uap:Extension>
    <desktop:Extension Category="windows.fullTrustProcess" Executable="DesktopExtensions/SystemControlDesktopExtension.exe" />
  </Extensions>
```

* Your Application section should now look something like this:

```xml
    <Application Id="App"
      Executable="$targetnametoken$.exe"
      EntryPoint="SystemControl.App">
      <uap:VisualElements
        DisplayName="SystemControl"
        Square150x150Logo="Assets\Square150x150Logo.png"
        Square44x44Logo="Assets\Square44x44Logo.png"
        Description="SystemControl"
        BackgroundColor="transparent">
        <uap:DefaultTile Wide310x150Logo="Assets\Wide310x150Logo.png"/>
        <uap:SplashScreen Image="Assets\SplashScreen.png" />
      </uap:VisualElements>
      <Extensions>
        <uap:Extension Category="windows.appService">
          <uap:AppService Name="com.stammen.systemcontrol.appservice" />
        </uap:Extension>
        <desktop:Extension Category="windows.fullTrustProcess" Executable="DesktopExtensions/SystemControlDesktopExtension.exe" />
      </Extensions>
    </Application>
```

Note: change com.stammen.systemcontrol.appservice to a name that makes sense for your app!

* Add the following Capability to the Capabilities section

```xml
  <Capabilities>
    <Capability Name="internetClient" />
    <rescap:Capability Name="runFullTrust" />
  </Capabilities>
```

* Right-click on your UWP project and select Unload Project

* Right-click on your UWP project and select Edit Project

* Add the following xml near the bottom of the xml

```xml
  <ItemGroup Label="DesktopExtensions">
    <Content Include="$(SolutionDir)\$(Platform)\$(Configuration)\SystemControlDesktopExtension.exe">
      <Link>DesktopExtensions\SystemControlDesktopExtension.exe</Link>
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
    <Content Include="$(SolutionDir)\$(Platform)\$(Configuration)\SystemControlDesktopExtension.pdb">
      <CopyToOutputDirectory>Always</CopyToOutputDirectory>
    </Content>
  </ItemGroup>
 ```
 
* Right-click on your UWP project and select Reload Project

* This xml will automatically add the SystemControlDesktopExtension.exe to the correct location in your AppX.

* Build and run the project. There should be no errors.

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
       