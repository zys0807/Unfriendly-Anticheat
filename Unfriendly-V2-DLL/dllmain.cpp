// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "DLLInjectorDector.h"
#include "ThreadIDs.h"
#include "ErrorHandler.h"
#include "AntiDebug.h"
#include "NamePipe.h"
#include "Driver Request.h"
#include "DLLInjectionDetector\Utils.h"


HANDLE hDriver;
ULONG UsermodeantiCheatProcessID = 0;

BOOL AntiDebugAntiKill = FALSE;
HANDLE hAntiDebug = NULL;
DWORD WINAPI AntiDebugThread()
{
	AntiDebug::HideThread(GetCurrentThread());
	while (1)
	{
		Sleep(200);
		if (AntiDebug::CheckRemoteDebuggerPresentAPI())
		{
			ErrorHandler::ErrorMessage("0x701", 6);
		}
		Sleep(200);
		if (AntiDebug::IsDebuggerPresentAPI())
		{
			ErrorHandler::ErrorMessage("0x702", 6);
		}
		Sleep(200);
		if (AntiDebug::ModuleBoundsHookCheck())
		{
			ErrorHandler::ErrorMessage("0x705", 6);
		}
		Sleep(200);

		if (CUtils::IsSuspendedThread(ThreadID::TIDMain)
			|| CUtils::IsSuspendedThread(ThreadID::TIDAntiDebug))
		{
			ErrorHandler::ErrorMessage("0x10090 ( Thread Mismatched )", 5);
		}

		AntiDebugAntiKill = TRUE;
	}
}

BOOL CheckUsermodePrcoessKill = FALSE;
HANDLE hCheckUsermodePrcoess = NULL;
DWORD WINAPI CheckUsermodePrcoess()
{
	AntiDebug::HideThread(GetCurrentThread());
	while (1)
	{
		if (UsermodeantiCheatProcessID == 0)
		{
			Sleep(200);
		}
		else
		{
			if (!ErrorHandler::isProcessRunning((int)UsermodeantiCheatProcessID))
			{
				ErrorHandler::ErrorMessage("0x901 ( Game Stopped Running )", 3);
			}
		}

		if (CUtils::IsSuspendedThread(ThreadID::TIDMain)
			|| CUtils::IsSuspendedThread(ThreadID::TIDAntiDebug))
		{
			ErrorHandler::ErrorMessage("0x32156 ( Thread Mismatched )", 6);
		}



		CheckUsermodePrcoessKill = TRUE;
	}
}

BOOL BoolMain = FALSE;
HANDLE hMain = NULL;
DWORD WINAPI Main()
{
	while (1)
	{
		HANDLE hDriver = CreateFileA("\\\\.\\UnfriendlyDriver", GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

		if (hDriver)
		{
			Sleep(200);
			KERNEL_READ_REQUEST Check = DriverRequest::CheckProcessIDs();

			UsermodeantiCheatProcessID = Check.UsermodeProgram;

			wchar_t text_buffer[500] = { 0 }; //temporary buffer
			swprintf(text_buffer, _countof(text_buffer), L"Process ID: %s", UsermodeantiCheatProcessID); // convert
			OutputDebugStringW(text_buffer);

			if (UsermodeantiCheatProcessID != 0)
			{
				hCheckUsermodePrcoess = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Main, NULL, 0, &ThreadID::TIDCheckUsermodePrcoess);
				while (1)
				{
					if (CUtils::IsSuspendedThread(ThreadID::TIDMain)
						|| CUtils::IsSuspendedThread(ThreadID::TIDAntiDebug)
						|| CUtils::IsSuspendedThread(ThreadID::TIDCheckUsermodePrcoess))
					{
						ErrorHandler::ErrorMessage("0x10090 ( Thread Mismatched )", 5);
					}
					Sleep(3000);
					if (CheckUsermodePrcoessKill && AntiDebugAntiKill)
					{
						CheckUsermodePrcoessKill = FALSE;
						AntiDebugAntiKill = FALSE;
					}
					else
					{
						ErrorHandler::ErrorMessage("0x9452 ( Thread Mismatched )", 5);
					}
				}
			}
			else
			{
				ErrorHandler::ErrorMessage("0x32145 ( Mismatched Results )", 1);
			}
		}
		else
		{
			ErrorHandler::ErrorMessage("0x100 ( Driver Not Found )", 1);
		}
	}
	return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		InitializeDLLCheck();
		InitializeThreadCheck();
		hMain = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Main, NULL, 0, &ThreadID::TIDMain);
		hAntiDebug = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AntiDebugThread, NULL, 0, &ThreadID::TIDAntiDebug);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

