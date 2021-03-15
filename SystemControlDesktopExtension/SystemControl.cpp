#include "Applications.h"
#include "SystemControl.h"
#include "Brightness.h"
#include "SystemVolume.h"
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
    auto response = ref new ValueSet();

    if (request->HasKey("Message"))
    {
        auto message = dynamic_cast<Platform::String^>(request->Lookup("Message"));
        if (message == "Brightness")
        {
            double value = static_cast<double>(request->Lookup("Value"));
            HRESULT hr = Brightness::SetBrightness(value);
            if (SUCCEEDED(hr))
            {
                response->Insert("Status", "OK");
            }
            else
            {
                std::wstringstream errorMessage;
                errorMessage << L"SetBrightness error: " << hr;
                response->Insert("Error", ref new Platform::String(errorMessage.str().c_str()));
            }
        }
        else if (message == "SystemVolume")
        {
            double value = static_cast<double>(request->Lookup("Value"));
            HRESULT hr = SystemVolume::SetSystemVolume(value);
            if (SUCCEEDED(hr))
            {
                response->Insert("Status", "OK");
            }
            else
            {
                std::wstringstream errorMessage;
                errorMessage << L"SystemVolume error: " << hr;
                response->Insert("Error", ref new Platform::String(errorMessage.str().c_str()));
            }
        }
        else if (message == "GetApplications")
        {
            response = Applications::GetApplications();
        }
        else if (message == "LaunchApplication")
        {
            Platform::String^ name = dynamic_cast<Platform::String^>(request->Lookup("Name"));
            response = Applications::LaunchApplication(name);
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


void SystemControl::OnAppServiceClosed(Windows::ApplicationModel::AppService::AppServiceConnection^ sender, Windows::ApplicationModel::AppService::AppServiceClosedEventArgs^ args)
{
    m_quitting = true;
}






