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

	bool exists = g_pFileSystem->FileExists(custname);
	if (exists) {
		//Try to read the first 32 bytes to inspect VTF header
		CUtlBuffer buf;
		g_pFileSystem->ReadFile(custname, "LOCAL", buf, 0x20);

		//here be dragons. don't look too close. 
		buf.SeekPut(CUtlBuffer::SEEK_HEAD, 0x14);
		buf.SeekGet(CUtlBuffer::SEEK_HEAD, 0x14);
		//compare ref -> 0x818000
		if (buf.GetInt() == 0x818000)
			return;
	}

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
	__func_TE_Spray sig = (__func_TE_Spray)FindSignature(base_addr, base_len, spray_sig);

	CreateInterfaceFn factory = Sys_GetFactory("engine.dll");
	CSysModule* fs = Sys_LoadModule("filesystem_stdio.dll");
	CreateInterfaceFn fsFactory = Sys_GetFactory(fs);

	g_pEngine = (IVEngineServer*)factory(INTERFACEVERSION_VENGINESERVER, 0);
	g_pFileSystem = (IFileSystem*)fsFactory(FILESYSTEM_INTERFACE_VERSION, 0);
	g_pFileSystem->Connect(Sys_GetFactoryThis());

	if (g_pFileSystem->Init() != INIT_OK) {
		printf("Unable to initialize Filesystem!!\n");
		return 1;
	}

	g_pFileSystem->AddSearchPath("", "LOCAL");
	detour_TE_Spray = new MologieDetours::Detour<__func_TE_Spray>(sig, hk_TE_Spray);

	return 0;
}


GMOD_MODULE_CLOSE() {
	if (detour_TE_Spray) {
		delete detour_TE_Spray;
		detour_TE_Spray = nullptr;
	}
	return 0;
}