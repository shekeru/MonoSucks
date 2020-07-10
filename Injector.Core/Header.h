#pragma once
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN  
#include "windows.h"

#using <System.Dll>
using namespace System;
using namespace System::Diagnostics;
using namespace System::Runtime::InteropServices;

#include <msclr/marshal.h>
#define chars ctx.marshal_as<const char*>

void PayloadStage2(array<String^>^ Info);
__declspec(dllexport) LRESULT WINAPI PayloadStage1
	(int nCode, WPARAM wParam, LPARAM lParam);

#define MonoFn(Name) (Name) \
	GetProcAddress(Mono, #Name)
#define MonoType(Name, ...) typedef \
	void* (__cdecl* Name) (__VA_ARGS__)

MonoType(mono_domain_get, void);
MonoType(mono_get_root_domain, void);
MonoType(mono_thread_attach, void*);
MonoType(mono_runtime_invoke, void*, void*, void*, void*);
MonoType(mono_domain_assembly_open, void*, const char*);
MonoType(mono_assembly_get_image, void*);
MonoType(mono_class_from_name, void*, const char*, const char*);
MonoType(mono_class_get_method_from_name, void*, const char*, int);