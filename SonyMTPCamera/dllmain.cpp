// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "Logger.h"
#include "Registry.h"
#include "CameraManager.h"
#include "ResourceLoader.h"

static DWORD dwTlsIndex;

HINSTANCE dllInstance;
Logger logger;
Registry registry(L"Software\\Retro.kiwi\\SonyMTPCamera.dll");

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
//    LPVOID lpvData;
//    BOOL fIgnore;

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        dllInstance = hModule;

        // Allocate a TLS index.
        if ((dwTlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES)
        {
            return FALSE;
        }

        registry.Open();
        logger.SetLogFilename(registry.GetString(L"", L"Logfile Name", L""));
        logger.SetLogLevel((Logger::LogLevel)registry.GetDWORD(L"", L"Log Level", (DWORD)Logger::LogLevel::Error));

        {
            Version* version = ResourceLoader::GetVersion();

            if (!version)
            {
                version = new Version(L"**Not Found**", L"No Version");
            }

            bool isWin64 = false;

#if _WIN64
            isWin64 = true;
#endif

            LOGINFO(L"DLL Starting up (%s v%s) - %d-bit", version->GetProductName().c_str(), version->GetVersion().c_str(), isWin64 ? 64 : 32);

            delete version;
        }

        // Ensure the cameras we know about are in the registry, otherwise the code will fail to find any viable candidates
        CameraManager::SetupSupportedDevices();
        registry.Close();

        // No break: Initialize the index for first thread.

    // The attached process creates a new thread. 
    case DLL_THREAD_ATTACH:
        /*// Initialize the TLS index for this thread.
        lpvData = (LPVOID)LocalAlloc(LPTR, 256);
        if (lpvData != NULL)
        {
            fIgnore = TlsSetValue(dwTlsIndex, lpvData);
        }*/
        break;

        // Release the allocated memory for this thread.

//        lpvData = TlsGetValue(dwTlsIndex);
//        if (lpvData != NULL)
//            LocalFree((HLOCAL)lpvData);

        break;

    // DLL unload due to process termination or FreeLibrary.
    case DLL_PROCESS_DETACH:
        // Release the allocated memory for this thread.
//        lpvData = TlsGetValue(dwTlsIndex);

//        if (lpvData != NULL)
//            LocalFree((HLOCAL)lpvData);

        // Release the TLS index.
        TlsFree(dwTlsIndex);
        LOGINFO(L"DLL Shutting Down");
        Logger::SetLogFilename(L"");
        break;
    }

    return TRUE;
}
