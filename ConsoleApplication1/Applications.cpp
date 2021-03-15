//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "Applications.h"
#include <collection.h>
#include <windows.h>
#include <atlbase.h>
#include <Shellapi.h>
#include <shlobj.h>
#include <propkey.h>
#include <string>
#include <sstream>     
#include <iostream>
#include <ppltasks.h>

using namespace Windows::ApplicationModel;
using namespace Windows::Storage;
using namespace Windows::Foundation::Collections;
using namespace Platform::Collections;
using namespace Windows::Management::Deployment;
using namespace concurrency;

std::mutex Applications::s_mutex;

HRESULT LaunchUWPApp(LPCWSTR aumid)
{
    Platform::String^ id = ref new  Platform::String(aumid);

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

        auto appListEntry = p->GetAppListEntries()->GetAt(0);

        if (appListEntry->AppUserModelId == id)
        {
            auto task = create_task(appListEntry->LaunchAsync());
            task.wait();
            bool result = task.get();
            std::cout << "LaunchAsync result:" << result << std::endl;
            if (result == true)
            {
                return S_OK;
            }
            else
            {
                break;
            }
        }
    }

    // last resort
    CComPtr<IApplicationActivationManager> AppActivationMgr = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_ApplicationActivationManager, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&AppActivationMgr));
    if (SUCCEEDED(hr))
    {
        DWORD pid = 0;
        hr = AppActivationMgr->ActivateApplication(aumid, nullptr, AO_NONE, &pid);
    }
    return hr;
}

HRESULT LaunchAppFromShortCut(IShellItem* psi)
{
    HRESULT hr;
    IShellLink* psl;

    // save the shortcut file to the app's temporary folder as we are only going to use it once
    StorageFolder^ localFolder = ApplicationData::Current->TemporaryFolder;
    Platform::String^ path = localFolder->Path + L"\\shortcut.lnk";

    // Get a pointer to the IShellLink interface. 
    hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hr))
    {
        IPersistFile* ppf = nullptr;

        PIDLIST_ABSOLUTE pidl;

        hr= SHGetIDListFromObject(psi, &pidl);
        if (SUCCEEDED(hr))
        {
            // set the shortcut info for this app
            hr = psl->SetIDList(pidl);
            CoTaskMemFree(pidl);
        }

        // Query IShellLink for the IPersistFile interface, used for saving the 
        // shortcut in persistent storage. 
        if (SUCCEEDED(hr))
        {
            hr = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
            if (SUCCEEDED(hr))
            {
                // Save the link by calling IPersistFile::Save. 
                hr = ppf->Save(path->Data(), TRUE);
                ppf->Release();
                if (SUCCEEDED(hr))
                {
                    // launch the app using its newly created shortcut
                    HINSTANCE hInstance = ShellExecute(NULL, NULL, path->Data(), L"", NULL, SW_SHOWNORMAL);
#pragma warning (disable: 4302 4311)
                    int result = (int)hInstance;
#pragma warning  (default: 4302 4311)  

                    if (result >= 32)
                    {
                        hr = S_OK;
                    }
                    else
                    {
                        hr = result;
                    }
                }
            }
        }
        psl->Release();
    }
    return hr;
}

Platform::String^ GetDisplayName(IShellItem *psi, SIGDN sigdn)
{
    LPWSTR pszName = nullptr;
    Platform::String^ result = ref new Platform::String();
    HRESULT hr = psi->GetDisplayName(sigdn, &pszName);
    if (SUCCEEDED(hr)) {
        result = ref new Platform::String(pszName);
        CoTaskMemFree(pszName);
    }

    return result;
}

Platform::String^ GetParsingPath(IShellItem2  *psi2)
{
    LPWSTR pszValue;
    Platform::String^ result = ref new Platform::String();
    HRESULT hr = psi2->GetString(PKEY_ParsingPath, &pszValue);
    if (SUCCEEDED(hr)) {
        result = ref new Platform::String(pszValue);
        CoTaskMemFree(pszValue);
    }
    return result;
}

Windows::Foundation::Collections::ValueSet^ Applications::LaunchApplication(Platform::String^ name)
{
    ValueSet^ result = ref new ValueSet;
    IShellItem *psiFolder;
    bool found = false;
    
    HRESULT hr = SHGetKnownFolderItem(FOLDERID_AppsFolder, KF_FLAG_DEFAULT, NULL, IID_PPV_ARGS(&psiFolder));
    if (SUCCEEDED(hr))
    {
        IEnumShellItems *pesi;
        hr = psiFolder->BindToHandler(NULL, BHID_EnumItems, IID_PPV_ARGS(&pesi));
        if (hr == S_OK)
        {
            IShellItem *psi;
            while (pesi->Next(1, &psi, NULL) == S_OK && !found)
            {
                IShellItem2 *psi2;
                if (SUCCEEDED(psi->QueryInterface(IID_PPV_ARGS(&psi2))))
                {
                    auto appName = GetDisplayName(psi2, SIGDN_NORMALDISPLAY);
                    if (!appName->IsEmpty())
                    {
                        if (appName == name)
                        {
                            found = true;
                            hr = LaunchAppFromShortCut(psi);
                            if (hr != S_OK) // We may be on Windows 10S. We can only launch UWP apps
                            {
                                auto aumid = GetParsingPath(psi2);
                                hr = LaunchUWPApp(aumid->Data());
                            }
                        }
                    }
                    psi2->Release();
                }
                psi->Release();
            }
            psiFolder->Release();
        }

        if (SUCCEEDED(hr))
        {
            result->Insert("Status", "OK");
        }
        else
        {
            std::wstringstream errorMessage;
            errorMessage << L"LaunchApplication error: " << hr;
            result->Insert("Error", ref new Platform::String(errorMessage.str().c_str()));
        }
    }

    return result;
}

// This method returns an array of Application Names from the Windows Application folder
ValueSet^ Applications::GetApplications()
{
    ValueSet^ result = ref new ValueSet;
    Vector<Platform::String^>^ applications = ref new Vector<Platform::String^>();

    IShellItem *psiFolder;
    HRESULT hr = SHGetKnownFolderItem(FOLDERID_AppsFolder, KF_FLAG_DEFAULT, NULL, IID_PPV_ARGS(&psiFolder));
    if (SUCCEEDED(hr)) 
    {
        IEnumShellItems *pesi;
        hr = psiFolder->BindToHandler(NULL, BHID_EnumItems, IID_PPV_ARGS(&pesi));
        if (SUCCEEDED(hr))
        {
            IShellItem *psi;
            while (pesi->Next(1, &psi, NULL) == S_OK)
            {
                IShellItem2 *psi2;
                if (SUCCEEDED(psi->QueryInterface(IID_PPV_ARGS(&psi2)))) 
                {
                    auto name = GetDisplayName(psi2, SIGDN_NORMALDISPLAY);
                    if (!name->IsEmpty())
                    {
                        applications->Append(name);
                    }
                    psi2->Release();
                }
                psi->Release();
            }
            psiFolder->Release();
        }

        if (SUCCEEDED(hr))
        {
            auto temp = ref new Platform::Array<Platform::String^>(applications->Size);
            for (unsigned int i = 0; i < applications->Size; ++i)
            {
                temp[i] = applications->GetAt(i);
            }

            result->Insert("Applications", temp);
        }
    }

    if (SUCCEEDED(hr))
    {
        result->Insert("Status", "OK");
    }
    else
    {
        std::wstringstream errorMessage;
        errorMessage << L"LaunchApplication error: " << hr;
        result->Insert("Error", ref new Platform::String(errorMessage.str().c_str()));
    }

    return result;
}


