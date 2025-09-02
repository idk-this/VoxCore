//
// Created by IDKTHIS on 30.08.2025.
//

#pragma once

#if defined(PLATFORM_WINDOWS)
    #if defined(VOXCORE_BUILD_DLL)
        #define VOXCORE_API __declspec(dllexport)
    #else
        #define VOXCORE_API __declspec(dllimport)
    #endif
#else
    #define VOXCORE_API
#endif
