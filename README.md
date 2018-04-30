# System Control from a UWP App Sample

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

* Select Visual C# | Windows Classic Desktop | Windows Form App. Name the project LauncherExtension. Select at least .NET framework 4.6.1.



