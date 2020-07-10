#include "Header.h"

static HHOOK _Hook;
static unsigned int I_WM_LOADASM =
	::RegisterWindowMessage("LOADASM!");

namespace InjectorCore
{
	public ref class Stage1 : Object
	{
	public:
		static void Inject(Process^ Proc, String^ AsmStr) {
			msclr::interop::marshal_context ctx;
			const char* AsmInfo = chars(AsmStr);
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

void PayloadStage2(array<String^>^ Info) {
	msclr::interop::marshal_context ctx;
	HMODULE Mono = LoadLibrary("mono.dll");
	// Mono Functions
	auto GetDomain = MonoFn(mono_domain_get);
	auto RootDomain = MonoFn(mono_get_root_domain);
	auto ThreadAttach = MonoFn(mono_thread_attach);
	auto AssemblyOpen = MonoFn(mono_domain_assembly_open);
	auto GetImage = MonoFn(mono_assembly_get_image);
	auto GetClassByName = MonoFn(mono_class_from_name);
	auto ClassMethod = MonoFn(mono_class_get_method_from_name);
	auto RuntimeInvoke = MonoFn(mono_runtime_invoke);
	// Start Mono Thread
	ThreadAttach(RootDomain());
	//Open the domain, the assembly, Image, Method, Invocation
	auto Image = GetImage(AssemblyOpen(GetDomain(), chars(Info[0])));
	auto Class = GetClassByName(Image, chars(Info[1]), chars(Info[2]));
	auto Method = ClassMethod(Class, chars(Info[3]), 0);
	RuntimeInvoke(Method, 0, 0, 0); FreeLibrary(Mono);
};

__declspec(dllexport)
LRESULT WINAPI PayloadStage1(int nCode, WPARAM wParam, LPARAM lParam)
{
	static size_t Size; static LRESULT Result; Result =
		CallNextHookEx(_Hook, nCode, wParam, lParam);
	// Check Vars
	if (!nCode) {
		auto msg = (CWPSTRUCT*)lParam;
		static HWND _WH = GetActiveWindow();
		// if has path
		if (msg && msg->message == I_WM_LOADASM) {
			//MessageBox(_WH, (char*)msg->wParam, "Injection Target", 0);
			auto ModuleData = (gcnew String((char*)
				msg->wParam))->Split('|');
			PayloadStage2(ModuleData);
		}
	}; return Result;
};