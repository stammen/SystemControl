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
#include <rpc.h>
#include <Shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <propkey.h>
#include <ppltasks.h>
#include <string>
#include <vector>

using namespace Windows::Foundation::Collections;
using namespace Platform::Collections;

std::mutex Applications::s_mutex;

HRESULT CreateLink(LPCWSTR lpszPathObj, LPCSTR lpszPathLink, LPCWSTR lpszDesc)
{
    HRESULT hres;
    IShellLink* psl;

    // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
    // has already been called.
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres))
    {
        IPersistFile* ppf;

        // Set the path to the shortcut target and add the description. 
        psl->SetPath(lpszPathObj);
        psl->SetDescription(lpszDesc);

        // Query IShellLink for the IPersistFile interface, used for saving the 
        // shortcut in persistent storage. 
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

        if (SUCCEEDED(hres))
        {
            WCHAR wsz[MAX_PATH];

            // Ensure that the string is Unicode. 
            MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH);

            // Save the link by calling IPersistFile::Save. 
            hres = ppf->Save(wsz, TRUE);
            ppf->Release();
        }
        psl->Release();
    }
    return hres;
}

HRESULT BindToCsidlItem(int csidl, IShellItem ** ppsi)
{
    *ppsi = NULL;
    HRESULT hr;
    PIDLIST_ABSOLUTE pidl;
    hr = SHGetSpecialFolderLocation(NULL, csidl, &pidl);
    if (SUCCEEDED(hr)) {
        hr = SHCreateShellItem(NULL, NULL, pidl, ppsi);
        CoTaskMemFree(pidl);
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

void PrintDetail(IShellItem2 *psi2, const SHCOLUMNID *pscid, PCTSTR pszLabel)
{
    LPWSTR pszValue;
    HRESULT hr = psi2->GetString(PKEY_ParsingPath, &pszValue);
    if (SUCCEEDED(hr)) {
        OutputDebugString(pszValue);
        CoTaskMemFree(pszValue);
    }
}

HRESULT LaunchApp(LPWSTR path)
{
    HRESULT hr = S_OK;

    std::wstring wPath(path);
    std::vector<std::wstring> tokens;

    std::wstring delimiters = L"{}";
    size_t current;
    size_t next = -1;
    do
    {
        current = next + 1;
        next = wPath.find_first_of(delimiters, current);
        tokens.push_back(wPath.substr(current, next - current));
    } while (next != std::string::npos);

    // check if path started with a Known Folder GUID
    if (tokens.size() > 1)
    {
        UUID uuid;
        auto ok = UuidFromString((RPC_WSTR)tokens[1].c_str(), &uuid);
        if (ok == RPC_S_OK)
        {
            PWSTR pszPath = NULL;
            hr = SHGetKnownFolderPath(uuid, 0, NULL, &pszPath);
            if (SUCCEEDED(hr))
            {
                std::wstring finalPath = pszPath;
                finalPath.append(tokens[2]);
                HINSTANCE result = ShellExecute(NULL, NULL, finalPath.c_str(), L"", NULL, SW_SHOWNORMAL);
                OutputDebugString(finalPath.c_str());
                // The calling application is responsible for calling CoTaskMemFree 
                // to free this resource after use.
                CoTaskMemFree(pszPath);
            }
        }
    }
    else
    {
        CComPtr<IApplicationActivationManager> AppActivationMgr = nullptr;
        if (SUCCEEDED(hr))
        {
            hr = CoCreateInstance(CLSID_ApplicationActivationManager, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&AppActivationMgr));
            if (FAILED(hr))
            {
                //OutputDebugString(L"LaunchApp %s: Failed to create Application Activation Manager. hr = 0x%08lx \n", wPath.c_str(), hr);
            }
        }
        if (SUCCEEDED(hr))
        {
            DWORD pid = 0;
            hr = AppActivationMgr->ActivateApplication(wPath.c_str(), nullptr, AO_NONE, &pid);
            if (FAILED(hr))
            {
                //OutputDebugString(L"LaunchApp %s: Failed to Activate App. hr = 0x%08lx \n", wPath.c_str(), hr);
            }
        }
    }


    return hr;
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
                            OutputDebugString(appName->Data());
                            
                            PWSTR pszFilePath = NULL;
                            hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                            CoTaskMemFree(pszFilePath);


                            LPWSTR path;
                            HRESULT hr = psi2->GetString(PKEY_ParsingPath, &path);
                            if (SUCCEEDED(hr))
                            {
                                LaunchApp(path);
                                OutputDebugString(path);
                                CoTaskMemFree(path);
                            }

                            found = true;
                        }
                    }
                    psi2->Release();
                }
                psi->Release();
            }
            psiFolder->Release();
        }

        result->Insert("Status", "OK");
    }

    return result;
}

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
        if (hr == S_OK) 
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

        auto temp = ref new Platform::Array<Platform::String^>(applications->Size);
        for (unsigned int i = 0; i < applications->Size; ++i)
        {
            temp[i] = applications->GetAt(i);
        }

        result->Insert("Status", "OK");
        result->Insert("Applications", temp);
    }

    return result;
}


