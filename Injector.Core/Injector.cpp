#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN  
#include "windows.h"
#include <iostream>
#include <stdio.h>

#using <System.Dll>
using namespace System;
using namespace System::Diagnostics;
#include <msclr/marshal.h>

static HHOOK _Hook; static LPVOID pMem;
static unsigned int I_WM_LOADASM =
	::RegisterWindowMessage("LOADASM!"), 
I_WM_UNLOAD = 
	::RegisterWindowMessage("UNLOAD!");

#define chars ctx.marshal_as<const char*>

__declspec(dllexport)
LRESULT WINAPI PayloadStage1(int nCode, WPARAM wParam, LPARAM lParam)
{
	static size_t Size; static LRESULT Result; Result =
		CallNextHookEx(_Hook, nCode, wParam, lParam);
	// Check Vars
	if (!nCode) {
		auto msg = (CWPSTRUCT*) lParam;
		if (msg && msg->message == I_WM_LOADASM) {
			msclr::interop::marshal_context ctx;
			MessageBox(GetActiveWindow(), (char*) 
				msg->wParam, "Injection Target", 0);
			// Load The Assembly
			auto Assembly = Reflection::Assembly::LoadFile
				(gcnew String((char*)msg->wParam));
			MessageBox(GetActiveWindow(), Assembly ? 
				chars(Assembly->ToString()): 
				"NULL.", "Assembly Info", 0);
			// Load Class
			char* className = "Payload";
			char* methodName = "Execute";
			auto TypeName = gcnew String(className);
			auto Type = Assembly->GetType(TypeName);
			//const char* T1 = ctx.marshal_as<const char*>(Type->ToString());
			MessageBox(GetActiveWindow(), Type ? className : "NULL.", "Step 2", 0);

		/*	auto Method = Type->GetMethod(gcnew String("Execute"), 
				Reflection::BindingFlags::Public |
				Reflection::BindingFlags::Static);*/
			//Method->Invoke(nullptr, gcnew array<Object^>(0));
			// Verify
			Size = strlen((char*)msg->wParam);
		}
		if (msg && msg->message == I_WM_UNLOAD) {
			MessageBox(GetActiveWindow(), "Loader.Core", "Eject Msg!", 0);
			VirtualFree(pMem, strlen((char*) pMem), MEM_RELEASE);
			UnhookWindowsHookEx(_Hook);
		}
	}; return Result;
};

namespace InjectorCore
{
	public ref class Injector : Object
	{
		HWND WHandle;
		DWORD ThreadId;
		HMODULE hInstance;
		HANDLE hProc;
	public:
		Injector(Process^ Proc) {
			// Get our DLL's Handle
			HMODULE hInstanceL;
			GetModuleHandleEx
			(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
				LPCTSTR(&PayloadStage1), 
			(HMODULE*) &hInstanceL);
			hInstance = hInstanceL;
			// Prepare C-Sharp Objects
			WHandle = (HWND)Proc->MainWindowHandle.ToPointer();
			ThreadId = GetWindowThreadProcessId(WHandle, NULL);
			// Print Info
			printf("Payload: %x, WHandle: %x, Process: %i, Thread: %i\n",
				hInstance, WHandle, Proc->Id, ThreadId);
			// Open Handle to Proc
			hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Proc->Id);
		}; 
		~Injector() {
			CloseHandle(hProc);
			FreeLibrary(hInstance);
		};
		void Inject(String^ AsmStr) {
			msclr::interop::marshal_context ctx;
			const char* AsmInfo = 
				ctx.marshal_as<const char*>(AsmStr);
			size_t Len = strlen(AsmInfo);
			// Write Assembly Path
			pMem = VirtualAllocEx(hProc, 0, Len, MEM_COMMIT, PAGE_READWRITE);
			BOOL Result = WriteProcessMemory(hProc, pMem, AsmInfo, Len, 0); 
			// Print Out Injection Info
			printf("hProc: %x, pMem: %p, String -> %s\n", hProc, pMem, Result ? AsmInfo : "");
			_Hook = ::SetWindowsHookEx(WH_CALLWNDPROC, &PayloadStage1, hInstance, ThreadId);
			printf("Hook Id: %x\n", _Hook); SendMessage(WHandle, I_WM_LOADASM, (WPARAM) pMem, 0);
		};
		void Eject() {
			SendMessage(WHandle, I_WM_UNLOAD, 0, 0);
		};
	};
};