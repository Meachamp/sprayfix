#include "Sigscan.h"
#include <Windows.h>
#include <stdio.h>
#include <eiface.h>
#include "GarrysMod\Lua\Interface.h"
#include <cdll_int.h>
#include <interface.h>
#include <cstring>
#include "detours/detours.h"
#include <filesystem.h>
#include <platform.h>
#include <utlbuffer.h>

#ifdef _WIN32
//const char* spray_sig = "\x55\x8B\xEC\x8B\x4D\x10\xA1";
const char* spray_sig = "\x55\x8B\xEC\x83\xEC\x20\x56\x8B\x75\x08\xF3\x0F\x10\x46\x2C";
#endif

typedef void(*__func_TE_Spray)(void* trace, int player);
IVEngineServer* g_pEngine;
IFileSystem* g_pFileSystem;
MologieDetours::Detour<__func_TE_Spray>* detour_TE_Spray = nullptr;

void hk_TE_Spray(void* trace, int player) {
	player_info_s info;
	g_pEngine->GetPlayerInfo(player, &info);
	 
	int cst = info.customFiles[0];

	char logohex[32];
	Q_binarytohex((byte *)&cst, sizeof(cst), logohex, sizeof(logohex));
	char custname[512];
	Q_snprintf(custname, sizeof(custname), "garrysmod/download/user_custom/%c%c/%s.dat", logohex[0], logohex[1], logohex);

	printf(custname);
	bool exists = g_pFileSystem->FileExists(custname);

	if (exists) {
	}


	printf("Exists %i\n", exists);

	detour_TE_Spray->GetOriginalFunction()(trace, player);
}

GMOD_MODULE_OPEN() {
	HMODULE mod = LoadLibrary("server.dll");

	MEMORY_BASIC_INFORMATION mem;
	if (!VirtualQuery(mod, &mem, sizeof(mem)))
		return false;
	char* base_addr = (char*)mem.AllocationBase;
	IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER*)mem.AllocationBase;
	IMAGE_NT_HEADERS *pe = (IMAGE_NT_HEADERS*)((unsigned long)dos + (unsigned long)dos->e_lfanew);
	if (pe->Signature != IMAGE_NT_SIGNATURE) {
		return false;
	}

	size_t base_len = (size_t)pe->OptionalHeader.SizeOfImage;

	printf("Loaded module: %p %i", base_addr, base_len);
	__func_TE_Spray sig = (__func_TE_Spray)FindSignature(base_addr, base_len, spray_sig);
	printf("Sig result: %p", sig);
	printf("\n");

	printf("%x", *(int*)sig);

	CreateInterfaceFn factory = Sys_GetFactory("engine.dll");
	CSysModule* fs = Sys_LoadModule("filesystem_stdio.dll");
	printf("Module: %p", fs);
	CreateInterfaceFn fsFactory = Sys_GetFactory(fs);
	g_pEngine = (IVEngineServer*)factory(INTERFACEVERSION_VENGINESERVER, 0);
	printf("\n");
	if (!fsFactory)
		return 1;
	g_pFileSystem = (IFileSystem*)fsFactory(FILESYSTEM_INTERFACE_VERSION, 0);

	printf("\nFS %p \n", g_pFileSystem);
	detour_TE_Spray = new MologieDetours::Detour<__func_TE_Spray>(sig, hk_TE_Spray);

	//CUtlBuffer buf;
	//buf.PutInt64(5);
	g_pFileSystem->Connect(Sys_GetFactoryThis());
	if (g_pFileSystem->Init() != INIT_OK)
		printf("BAD INIT\n");
	g_pFileSystem->AddSearchPath("", "LOCAL");
	//g_pFileSystem->WriteFile("test.dat", "", buf);

	bool exists = g_pFileSystem->FileExists("garrysmod/download/user_custom/5a/5a73fb37.dat");

	printf("EXISTS %i\n", exists);

	printf("\n");
	return 0;
}


GMOD_MODULE_CLOSE() {
	delete detour_TE_Spray;
	detour_TE_Spray = nullptr;
	return 0;
}