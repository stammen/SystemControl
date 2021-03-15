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
using namespace Windows::Management::Deployment;

SystemControl::SystemControl()
{

}

SystemControl::~SystemControl()
{
}

void SystemControl::SystemControlThread(const std::wstring& name)
{
    Platform::String^ app = ref new Platform::String(name.c_str());
    std::wcout << L"Launching: " << name << std::endl;

    auto pm = ref new PackageManager();
    auto packages = pm->FindPackagesForUser(L"");
    for (Package^ p : packages)
    {
        std::wcout << "DisplayName:" << p->DisplayName->Data() << std::endl;

        auto appListEntries = p->GetAppListEntries();
        if (appListEntries->Size == 0)
        {
            continue;
        }

        auto appListEntry = appListEntries->GetAt(0);
       
        if (appListEntry->DisplayInfo->DisplayName == app)
        {
            auto task = create_task(appListEntry->LaunchAsync());
            task.wait();
            bool result = task.get();
            std::cout << "LaunchAsync result:" << result << std::endl;
            if (result == true)
            {
                return;
            }
            else
            {
                break;
            }
        }
    }

    if (name != L"SystemControl")
    {
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
    else
    {
        auto task = create_task(Package::Current->GetAppListEntriesAsync());
        task.wait();
        auto appListEntries = task.get();
        std::cout << "appListEntries" << std::endl;
        auto task2 = create_task(appListEntries->GetAt(0)->LaunchAsync());
        task2.wait();
        bool result = task2.get();
        std::cout << "LaunchAsync result:" << result << std::endl;
    }
}

int SystemControl::Run(const std::wstring& name)
{
    std::thread t(&SystemControl::SystemControlThread, this, name);
    t.join();
    return 0;
}






