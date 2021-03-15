#include "Applications.h"
#include "SystemControl.h"
#include <string>
#include <sstream>     
#include <functional>

#include <collection.h>  
#include <thread>        
#include <ppltasks.h>
#include <iostream>

using namespace concurrency;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::System;
using namespace std::placeholders;

SystemControl::SystemControl()
{

}

SystemControl::~SystemControl()
{
}



int SystemControl::Run(const std::wstring& name)
{
    Platform::String^ app = ref new Platform::String(name.c_str());

    if (name != L"SystemControl")
    {
        Applications::LaunchApplication(app);
        INPUT inputs[2];
        ZeroMemory(inputs, sizeof(inputs));
        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = VK_MENU;
        inputs[1].type = INPUT_KEYBOARD;
        inputs[1].ki.wVk = VK_MENU;
        inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
        UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));

    }

    if (name == L"SystemControl")
    {
        std::cout << "SystemControl" << std::endl;

        auto task = create_task(Package::Current->GetAppListEntriesAsync());

        create_task(task).then([this](IVectorView<AppListEntry^>^ appListEntries)
        {
            std::cout << "appListEntries" << std::endl;
            auto task2 = create_task(appListEntries->GetAt(0)->LaunchAsync());
            create_task(task2).then([this](bool result)
            {
                std::cout << "LaunchAsync result:" << result << std::endl;
            });
        });
    }

    Sleep(5000);

    return 0;
}






