#include "Sigscan.h"
#include <stdio.h>
#include <eiface.h>
#include "GarrysMod/Lua/Interface.h"
#include <cdll_int.h>
#include <interface.h>
#include <cstring>
#include "detours/detours.h"
#include <filesystem.h>
#include <platform.h>
#include <utlbuffer.h>

#ifdef _WIN32
const char* spray_sig = "\x55\x8B\xEC\x83\xEC\x20\x56\x8B\x75\x08\xF3\x0F\x10\x46\x2C";
const char* srv_module = "server.dll";
const char* eng_module = "engine.dll";
const char* fs_module = "filesystem_stdio.dll";
#elif POSIX
const char* spray_sig = "\x55\x89\xE5\x56\x53\x83\xEC\x40\x8B\x5D\x08\xF3\x0F\x10\x05????\x0F\x2F\x43\x2C";
const char* srv_module = "server_srv.so";
const char* eng_module = "engine_srv.so";
const char* fs_module = "filesystem_stdio.so";
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
	CSysModule* mod = Sys_LoadModule(srv_module);

	__func_TE_Spray sig = (__func_TE_Spray)FindSignature((char*)mod, 10*1024*1024, spray_sig);

	CreateInterfaceFn factory = Sys_GetFactory(eng_module);
	CSysModule* fs = Sys_LoadModule(fs_module);
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