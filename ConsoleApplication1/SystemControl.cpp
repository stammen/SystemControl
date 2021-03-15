#include "Applications.h"
#include "SystemControl.h"
#include <string>
#include <sstream>     
#include <functional>
#include <thread>

#include <collection.h>  
#include <thread>        
#include <ppltasks.h>
#include <iostream>

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;

SystemControl::SystemControl()
{

}

SystemControl::~SystemControl()
{
}

void SystemControl::SystemControlThread(const std::wstring& name)
{
    Platform::String^ app = ref new Platform::String(name.c_str());
    INPUT inputs[1];
    ZeroMemory(inputs, sizeof(inputs));
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_MENU;
    UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
        
    Applications::LaunchApplication(app);

    ZeroMemory(inputs, sizeof(inputs));
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_MENU;
    inputs[0].ki.dwFlags = KEYEVENTF_KEYUP;
    uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));

}

int SystemControl::Run(const std::wstring& name)
{
    std::thread t(&SystemControl::SystemControlThread, this, name);
    t.join();
    return 0;
}






