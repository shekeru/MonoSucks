#include "basic.h"
#include <vcclr.h>

Process^ SelectWindow()
{
	Process^ _self = Process::GetCurrentProcess();
	auto pList = Process::GetProcesses(); int S;
	// Filter Procs
	for (int I = 0; I < pList->Length; I++)
		if (!String::IsNullOrEmpty(pList[I]->MainWindowTitle) 
			&& pList[I]->Id != _self->Id) printf
		(" [%i]: %s\n", I, pList[I]->MainWindowTitle);
	std::cout << "Please Select Window >> "; 
	std::cin >> S; std::cout << "\n"; 
	return pList[S];
}

using namespace Payload;
int main(array<System::String ^> ^args)
{
	auto Proc = SelectWindow();
	auto Core = gcnew Injector;
	Core->Inject(Proc, "");
	return 0;
}
