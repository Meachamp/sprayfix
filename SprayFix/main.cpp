#include "Sigscan.h"
#include <Windows.h>
#include <stdio.h>
#include <eiface.h>
#include "GarrysMod\Lua\Interface.h"
#include <cdll_int.h>
#include <interface.h>


#ifdef _WIN32
const char* spray_sig = "\x55\x8B\xEC\x8B\x4D\x10\xA1????\x3B\x01\x75?\xA1????\x3B\x41\x04\x75?\xA1????"
"\x3B\x41\x08\x74?\xF3\x0F\x10\x01\xF3\x0F\x11\x05????\xF3\x0F\x10\x41\x04\xF3\x0F\x11\x05????\xF3"
"\x0F\x10\x41\x08\xF3\x0F\x11\x05????\x8B\x4D\x14\xA1????\x3B\x01\x75?\xA1????\x3B\x41\x04\x75?\xA1"
"????\x3B\x41\x08\x74?\xF3\x0F\x10\x01\xF3\x0F\x11\x05????\xF3\x0F\x10\x41\x04\xF3\x0F\x11\x05????"
"\xF3\x0F\x10\x41\x08\xF3\x0F\x11\x05????\xA1????\x8D\x55\x18\x3B\x02\x8B\xC8\xA1????\x8D\x55\x1C"
"\x0F\x45\x4D\x18\x3B\x02\x89\x0D????\x8B\xC8\x0F\x45\x4D\x1C\xA1????\x89\x0D????\x8D\x4D\x20";
#endif

IVEngineServer* g_pEngine;

GMOD_MODULE_OPEN() {
	HMODULE mod = LoadLibrary("server.dll");

	MEMORY_BASIC_INFORMATION mem;

	if (!VirtualQuery(mod, &mem, sizeof(mem)))
		return false;

	unsigned char* base_addr = (unsigned char*)mem.AllocationBase;

	IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER*)mem.AllocationBase;
	IMAGE_NT_HEADERS *pe = (IMAGE_NT_HEADERS*)((unsigned long)dos + (unsigned long)dos->e_lfanew);

	if (pe->Signature != IMAGE_NT_SIGNATURE) {
		return false;
	}

	size_t base_len = (size_t)pe->OptionalHeader.SizeOfImage;

	printf("Loaded module: %p %i", base_addr, base_len);

	void* sig = FindSignature((char*)base_addr, base_len, spray_sig);

	printf("Sig result: %p", sig);

	printf("\n");

	CreateInterfaceFn factory = Sys_GetFactory("engine.dll");

	g_pEngine = (IVEngineServer*)factory(INTERFACEVERSION_VENGINESERVER, 0);



	return 0;
}


GMOD_MODULE_CLOSE() {
	return 0;
}