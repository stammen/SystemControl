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
