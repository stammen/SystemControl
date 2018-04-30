#pragma once

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
