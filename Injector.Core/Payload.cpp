#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN  
#include "windows.h"

#using <System.Dll>
using namespace System;
using namespace System::Diagnostics;
#include <msclr/marshal.h>

static HHOOK _Hook;
static unsigned int I_WM_LOADASM =
	::RegisterWindowMessage("LOADASM!");
#define chars ctx.marshal_as<const char*>

__declspec(dllexport)
LRESULT WINAPI PayloadStage1(int nCode, WPARAM wParam, LPARAM lParam)
{
	static size_t Size; static LRESULT Result; Result =
		CallNextHookEx(_Hook, nCode, wParam, lParam);
	// Check Vars
	if (!nCode) {
		auto msg = (CWPSTRUCT*) lParam;
		static HWND _WH = GetActiveWindow();
		// if has path
		if (msg && msg->message == I_WM_LOADASM) {
			//MessageBox(_WH, (char*) msg->wParam, "Injection Target", 0);
			msclr::interop::marshal_context ctx;
			auto ModuleData = (gcnew String((char*) 
				msg->wParam))->Split('|');
			// Load The Assembly By Path
			auto Assembly = Reflection::Assembly
				::LoadFile(ModuleData[0]);
			if (Assembly) {
				//MessageBox(_WH, chars(Assembly->FullName), "Assembly Loaded", 0);
				// Load Class By Name
				auto Type = Assembly->GetType(ModuleData[1]);
				if (Type) {
					//MessageBox(_WH, chars(Type->FullName), 
					//	"Payload Found", 0);
					auto Method = Type->GetMethod(ModuleData[2],
						Reflection::BindingFlags::Public |
						Reflection::BindingFlags::Static);
					if (Method) {
						//MessageBox(_WH, chars(Method->Name), "Method Found", 0);
						Method->Invoke(nullptr, gcnew array<Object^>(0));
					}
				}
			}
		}
	}; return Result;
};

namespace InjectorCore
{
	public ref class Stage1 : Object
	{
	public:
		static void Inject(Process^ Proc, String^ AsmStr) {
			msclr::interop::marshal_context ctx;
			const char* AsmInfo = 
				ctx.marshal_as<const char*>(AsmStr);
			size_t Len = strlen(AsmInfo);
			// Get our DLL's Handle
			HMODULE hInstance;
			GetModuleHandleEx
			(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
				LPCTSTR(&PayloadStage1),
				(HMODULE*)&hInstance);
			// Prepare C-Sharp Objects
			HWND WHandle = (HWND)Proc->MainWindowHandle.ToPointer();
			DWORD ThreadId = GetWindowThreadProcessId(WHandle, NULL);
			// Print Info
			printf("Location: %p, WHandle: %x, Process: %i, Thread: %i\n",
				hInstance, WHandle, Proc->Id, ThreadId);
			// Open Handle to Proc
			HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Proc->Id);
			// Write Assembly Path
			PVOID pMem = VirtualAllocEx(hProc, 0, Len, MEM_COMMIT, PAGE_READWRITE);
			BOOL Result = WriteProcessMemory(hProc, pMem, AsmInfo, Len, 0); 
			printf("hProc: %x, pMem: %p, Bytes -> %i\n", hProc, pMem, Result ? Len : 0);
			// Hooking Methods
			_Hook = ::SetWindowsHookEx(WH_CALLWNDPROC, &PayloadStage1, hInstance, ThreadId);
			printf("Hook Id: %x\n", _Hook); SendMessage(WHandle, I_WM_LOADASM, (WPARAM) pMem, 0);
			// Releasing Resources
			UnhookWindowsHookEx(_Hook);
			VirtualFree(pMem, Len, MEM_RELEASE);
			CloseHandle(hProc); FreeLibrary(hInstance);
		};
	};
};