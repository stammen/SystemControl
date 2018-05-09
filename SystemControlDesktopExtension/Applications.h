//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#pragma once

#include <mutex>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class Applications
{
public:
    static Windows::Foundation::Collections::ValueSet^ GetApplications();
    static Windows::Foundation::Collections::ValueSet^ LaunchApplication(Platform::String^ name);

private:
    static std::mutex s_mutex;
};
