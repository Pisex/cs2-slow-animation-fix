#include <stdio.h>
#include "slow_animation_fix.h"
#include "metamod_oslink.h"
#include "schemasystem/schemasystem.h"

slow_animation_fix g_slow_animation_fix;
PLUGIN_EXPOSE(slow_animation_fix, g_slow_animation_fix);
IVEngineServer2* engine = nullptr;
CGameEntitySystem* g_pGameEntitySystem = nullptr;
CEntitySystem* g_pEntitySystem = nullptr;
CGlobalVars *gpGlobals = nullptr;

const char* g_szMap;

IUtilsApi* g_pUtils;
IPlayersApi* g_pPlayers;

void StartupServer()
{
	g_pGameEntitySystem = GameEntitySystem();
	g_pEntitySystem = g_pUtils->GetCEntitySystem();
	gpGlobals = g_pUtils->GetCGlobalVars();

	char szBuffer[64];
	g_SMAPI->Format(szBuffer, sizeof(szBuffer), "%s", gpGlobals->mapname);
	if(szBuffer[0] != '\0') g_szMap = strdup(szBuffer);
	else g_szMap = strdup("unknown");
}

bool slow_animation_fix::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetEngineFactory, g_pSchemaSystem, ISchemaSystem, SCHEMASYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer2, SOURCE2ENGINETOSERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pGameResourceServiceServer, IGameResourceService, GAMERESOURCESERVICESERVER_INTERFACE_VERSION);

	g_SMAPI->AddListener( this, this );

	return true;
}

bool slow_animation_fix::Unload(char *error, size_t maxlen)
{
	ConVar_Unregister();
	
	return true;
}

void slow_animation_fix::AllPluginsLoaded()
{
	char error[64];
	int ret;
	
	g_pUtils = (IUtilsApi *)g_SMAPI->MetaFactory(Utils_INTERFACE, &ret, NULL);
	if (ret == META_IFACE_FAILED)
	{
		g_SMAPI->Format(error, sizeof(error), "Missing Utils system plugin");
		ConColorMsg(Color(255, 0, 0, 255), "[%s] %s\n", GetLogTag(), error);
		std::string sBuffer = "meta unload "+std::to_string(g_PLID);
		engine->ServerCommand(sBuffer.c_str());
		return;
	}

	g_pPlayers = (IPlayersApi *)g_SMAPI->MetaFactory(PLAYERS_INTERFACE, &ret, NULL);
	if (ret == META_IFACE_FAILED)
	{
		g_SMAPI->Format(error, sizeof(error), "Missing Players system plugin");
		ConColorMsg(Color(255, 0, 0, 255), "[%s] %s\n", GetLogTag(), error);
		std::string sBuffer = "meta unload "+std::to_string(g_PLID);
		engine->ServerCommand(sBuffer.c_str());
		return;
	}

	g_pUtils->StartupServer(g_PLID, StartupServer);
	g_pUtils->CreateTimer(1800.0f, [](){
		int iPlayers = 0;
		for (int i = 0; i < 64; i++)
		{
			if(g_pPlayers->IsFakeClient(i)) continue;
			iPlayers++;
		}

		if(g_szMap[0] && iPlayers == 0) {
			char szBuffer[256];
			if(!engine->IsMapValid(g_szMap))
			{
				g_SMAPI->Format(szBuffer, sizeof(szBuffer), "ds_workshop_changelevel %s", g_szMap);
				engine->ServerCommand(szBuffer);
			}
			else engine->ChangeLevel(g_szMap, nullptr);
		}
		
		return 1800.0f;
	});
}

///////////////////////////////////////
const char* slow_animation_fix::GetLicense()
{
	return "GPL";
}

const char* slow_animation_fix::GetVersion()
{
	return "1.0";
}

const char* slow_animation_fix::GetDate()
{
	return __DATE__;
}

const char *slow_animation_fix::GetLogTag()
{
	return "slow_animation_fix";
}

const char* slow_animation_fix::GetAuthor()
{
	return "Pisex";
}

const char* slow_animation_fix::GetDescription()
{
	return "Slow Animation Fix";
}

const char* slow_animation_fix::GetName()
{
	return "Slow Animation Fix";
}

const char* slow_animation_fix::GetURL()
{
	return "https://discord.gg/g798xERK5Y";
}
