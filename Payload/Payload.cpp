#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN  
#include "windows.h"
#include <iostream>
#include <stdio.h>

static HHOOK _Hook;
static unsigned int WM_FUCKSHIT =
::RegisterWindowMessage("FUCKFUCKFUCK!");

__declspec(dllexport)
LRESULT WINAPI PayloadStage1(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (!nCode) {
		const auto msg = (CWPSTRUCT*) lParam;
		if (msg && msg->message == WM_FUCKSHIT) {
			MessageBox(GetActiveWindow(), "Oh boy", "ho boy...", 0);
		}
	}; return CallNextHookEx(_Hook, nCode, wParam, lParam);
};

#using <System.Dll>
using namespace System;
using namespace System::Diagnostics;
#include <msclr/marshal.h>

namespace Payload
{
	public ref class Injector : System::Object
	{
	public:
		HMODULE hInstance;
		Injector() {
			// Get our DLL's Handle
			HMODULE hInstanceL;
			GetModuleHandleEx
			(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
				LPCTSTR(&PayloadStage1), 
			(HMODULE*) &hInstanceL);
			hInstance = hInstanceL;
		}; 
		~Injector() {
			FreeLibrary(hInstance);
		};
		void Inject(Process^ Proc, String^ AsmStr) {
			msclr::interop::marshal_context ctx;
			const char* AsmInfo = 
				ctx.marshal_as<const char*>(AsmStr);
			size_t Len = strlen(AsmInfo);
			// Prepare C-Sharp Objects
			HWND WHandle = (HWND)Proc->MainWindowHandle.ToPointer();
			DWORD ThreadId = GetWindowThreadProcessId(WHandle, NULL);
			printf("Payload: %x, WHandle: %x, Process: %i, Thread: %i\n",
				hInstance, WHandle, Proc->Id, ThreadId);
			// Write Assembly Path
			HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Proc->Id);
			LPVOID pMem = VirtualAllocEx(hProc, (PVOID) AsmInfo, Len, MEM_COMMIT, PAGE_READWRITE);
			BOOL Result = WriteProcessMemory(hProc, pMem, nullptr, 0, 0); printf
				("hProc: %x, pMem: %p, Bytes -> %i\n", hProc, pMem, Result ? 0: 0);
			// Hook Our function
			_Hook = ::SetWindowsHookEx(WH_CALLWNDPROC, &PayloadStage1, hInstance, ThreadId);
				printf("Did Set Hook? %x\n", _Hook);
			SendMessage(WHandle, WM_FUCKSHIT, 0, 0); UnhookWindowsHookEx(_Hook);
			VirtualFreeEx(hProc, pMem, 0, MEM_RELEASE); 
			CloseHandle(hProc); printf("Injection Over.\n");
		};
	};
};