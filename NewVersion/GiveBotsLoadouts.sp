#include <sourcemod>
#include <tf2_stocks>

#pragma semicolon 1
#pragma newdecls required
#define PLUGIN_VERSION "1.20"

int g_iResourceEntity;
bool g_bTouched[MAXPLAYERS + 1];
bool g_bSuddenDeathMode;
bool g_bMVM;
bool g_bMedi;
bool g_bLateLoad;
ConVar g_hCVTimer;
ConVar g_hCVEnabled;
ConVar g_hCVTeam;
ConVar g_hCVMVMSupport;
Handle g_hWeaponEquip;
Handle g_hWWeaponEquip;

int scoutLoadout[MAXPLAYERS + 1];
int soldierLoadout[MAXPLAYERS + 1];
int pyroLoadout[MAXPLAYERS + 1];
int engineerLoadout[MAXPLAYERS + 1];
int heavyLoadout[MAXPLAYERS + 1];
int demomanLoadout[MAXPLAYERS + 1];
int medicLoadout[MAXPLAYERS + 1];
int sniperLoadout[MAXPLAYERS + 1];
int spyLoadout[MAXPLAYERS + 1];

public Plugin myinfo = 
{
	name = "Give Bots Loadouts",
	author = "luki1412, EfeDursun125",
	description = "Gives TF2 bots non-stock loadouts",
	version = PLUGIN_VERSION,
	url = "https://forums.alliedmods.net/member.php?u=43109"
}

public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int err_max) 
{
	if (GetEngineVersion() != Engine_TF2) 
	{
		Format(error, err_max, "This plugin only works for Team Fortress 2.");
		return APLRes_Failure;
	}

	g_bLateLoad = late;
	return APLRes_Success;
}

public void OnPluginStart() 
{
	ConVar hCVversioncvar = CreateConVar("sm_gbl_version", PLUGIN_VERSION, "Give Bots Weapons version cvar", FCVAR_NOTIFY|FCVAR_DONTRECORD); 
	g_hCVEnabled = CreateConVar("sm_gbl_enabled", "1", "Enables/disables this plugin", FCVAR_NONE, true, 0.0, true, 1.0);
	g_hCVTimer = CreateConVar("sm_gbl_delay", "0.111", "Delay for giving weapons to bots", FCVAR_NONE, true, 0.1, true, 30.0);
	g_hCVTeam = CreateConVar("sm_gbl_team", "1", "Team to give weapons to: 1-both, 2-red, 3-blu", FCVAR_NONE, true, 1.0, true, 3.0);
	g_hCVMVMSupport = CreateConVar("sm_gbw_mvm", "1", "Enables/disables giving bots weapons when MVM mode is enabled", FCVAR_NONE, true, 0.0, true, 1.0);
	
	HookEvent("post_inventory_application", player_inv);
	HookEvent("teamplay_round_stalemate", EventSuddenDeath, EventHookMode_PostNoCopy);
	HookEvent("teamplay_round_start", EventRoundReset, EventHookMode_PostNoCopy);
	HookConVarChange(g_hCVEnabled, OnEnabledChanged);
	
	SetConVarString(hCVversioncvar, PLUGIN_VERSION);
	AutoExecConfig(true, "Give_Bots_Weapons");

	if (g_bLateLoad)
		OnMapStart();
	
	GameData hGameConfig = LoadGameConfigFile("give.bots.stuff");
	if (!hGameConfig)
		SetFailState("Failed to find give.bots.stuff.txt gamedata! Can't continue.");
	
	StartPrepSDKCall(SDKCall_Player);
	PrepSDKCall_SetFromConf(hGameConfig, SDKConf_Virtual, "WeaponEquip");
	PrepSDKCall_AddParameter(SDKType_CBaseEntity, SDKPass_Pointer);
	g_hWeaponEquip = EndPrepSDKCall();
	if (!g_hWeaponEquip)
		SetFailState("Failed to prepare the SDKCall for giving weapons. Try updating gamedata or restarting your server.");
	
	StartPrepSDKCall(SDKCall_Player);
	PrepSDKCall_SetFromConf(hGameConfig, SDKConf_Virtual, "EquipWearable");
	PrepSDKCall_AddParameter(SDKType_CBaseEntity, SDKPass_Pointer);
	g_hWWeaponEquip = EndPrepSDKCall();
	if (!g_hWWeaponEquip)
		SetFailState("Failed to prepare the SDKCall for giving weapons. Try updating gamedata or restarting your server.");

	if (hGameConfig)
		delete hGameConfig;

	// for the plugin reload
	int i;
	for (i = 0; i <= MaxClients; i++)
	{
		if (!IsPlayerHere(i))
			continue;

		if (!IsFakeClient(i))
			continue;

		SetupLoadouts(i);
	}
}

public void OnEnabledChanged(ConVar convar, const char[] oldValue, const char[] newValue)
{
	if (GetConVarBool(g_hCVEnabled))
	{
		HookEvent("post_inventory_application", player_inv);
		HookEvent("teamplay_round_stalemate", EventSuddenDeath, EventHookMode_PostNoCopy);
		HookEvent("teamplay_round_start", EventRoundReset, EventHookMode_PostNoCopy);
	}
	else
	{
		UnhookEvent("post_inventory_application", player_inv);
		UnhookEvent("teamplay_round_stalemate", EventSuddenDeath, EventHookMode_PostNoCopy);
		UnhookEvent("teamplay_round_start", EventRoundReset, EventHookMode_PostNoCopy);
	}
}

int g_seed;
public void OnMapStart()
{
	if (GameRules_GetProp("m_bPlayingMannVsMachine"))
		g_bMVM = true;
	else
		g_bMVM = false;

	if (GameRules_GetProp("m_bPlayingMedieval"))
		g_bMedi = true;
	else
		g_bMedi = false;

	g_iResourceEntity = GetPlayerResourceEntity();
	g_seed += GetURandomInt();
}

stock int frand()
{
	g_seed = (214013 * g_seed + 2531011);
	return (g_seed >> 16) & 32767;
}

stock int crandomint(const int min, const int max)
{
	return frand() % (max - min + 1) + min;
}

public void OnClientPutInServer(int client)
{
	SetupLoadouts(client);
}

stock void SetupLoadouts(const int client)
{
	scoutLoadout[client] = crandomint(1, 18);
	soldierLoadout[client] = crandomint(1, 16);
	pyroLoadout[client] = crandomint(1, 19);
	engineerLoadout[client] = crandomint(1, 23);
	heavyLoadout[client] = crandomint(1, 17);
	demomanLoadout[client] = crandomint(1, 16);
	medicLoadout[client] = crandomint(1, 22);
	sniperLoadout[client] = crandomint(1, 20);
	spyLoadout[client] = crandomint(1, 18);
}

public void OnClientDisconnect(int client)
{
	g_bTouched[client] = false;
}

public void player_inv(Handle event, const char[] name, bool dontBroadcast) 
{
	if (!GetConVarBool(g_hCVEnabled))
		return;
	
	if (!g_bSuddenDeathMode && (!g_bMVM || GetConVarBool(g_hCVMVMSupport)))
	{
		int client = GetClientOfUserId(GetEventInt(event, "userid"));
		if (!IsPlayerHere(client) || g_bTouched[client])
			return;
		
		g_bTouched[client] = true;
		switch (GetConVarInt(g_hCVTeam))
		{
			case 1:
				CreateTimer(GetConVarFloat(g_hCVTimer), Timer_GiveWeapons, client, TIMER_FLAG_NO_MAPCHANGE);
			case 2:
			{
				if (GetClientTeam(client) == 2)
					CreateTimer(GetConVarFloat(g_hCVTimer), Timer_GiveWeapons, client, TIMER_FLAG_NO_MAPCHANGE);
			}
			case 3:
			{
				if (GetClientTeam(client) == 3)
					CreateTimer(GetConVarFloat(g_hCVTimer), Timer_GiveWeapons, client, TIMER_FLAG_NO_MAPCHANGE);
			}
		}
	}
}

public Action Timer_GiveWeapons(Handle timer, any data)
{
	int client = data;
	if (!IsPlayerHere(client))
		return Plugin_Continue;

	g_bTouched[client] = false;
	switch (GetConVarInt(g_hCVTeam))
	{
		case 2:
		{
			if (GetClientTeam(client) != 2)
				return Plugin_Continue;
		}
		case 3:
		{
			if (GetClientTeam(client) != 3)
				return Plugin_Continue;
		}
	}

	if (!g_bSuddenDeathMode && (!g_bMVM || GetConVarBool(g_hCVMVMSupport)))
	{
		if (!g_bMedi)
		{
			switch (TF2_GetPlayerClass(client))
			{
				case TFClass_Scout:
				{
					switch (scoutLoadout[client])
					{
						case 1:
						{
							CreateWeapon(client, "tf_weapon_soda_popper", 0, 448);
							CreateWeapon(client, "tf_weapon_handgun_scout_secondary", 1, 449);
							CreateWeapon(client, "tf_weapon_bat", 2, 317);
							CreateHat(client, 261);
							CreateHat(client, 343);
							CreateHat(client, 165);
						}
						case 2:
						{
							CreateWeapon(client, "tf_weapon_scattergun", 0, 1078);
							CreateWeapon(client, "tf_weapon_lunchbox_drink", 1, 1145);
							CreateWeapon(client, "tf_weapon_bat", 2, 349);
							CreateHat(client, 30540);
							CreateHat(client, 30428);
							CreateHat(client, 30552);
						}
						case 3:
						{
							CreateWeapon(client, "tf_weapon_scattergun", 0, 1103);
							CreateWeapon(client, "tf_weapon_jar_milk", 1, 1121);
							CreateWeapon(client, "tf_weapon_bat", 2, 450);
							CreateHat(client, 30576);
						}
						case 4:
						{
							CreateWeapon(client, "tf_weapon_jar_milk", 1, 222);
						}
						case 5:
						{
							CreateWeapon(client, "tf_weapon_pep_brawler_blaster", 0, 772);
							CreateWeapon(client, "tf_weapon_handgun_scout_secondary", 1, 773);
							CreateWeapon(client, "tf_weapon_bat", 2, 317);
							CreateHat(client, 780);
							CreateHat(client, 781);
						}
						case 6:
						{
							CreateWeapon(client, "tf_weapon_pep_brawler_blaster", 0, 772);
							CreateWeapon(client, "tf_weapon_cleaver", 1, 812);
							CreateWeapon(client, "tf_weapon_bat_fish", 2, 221);
							CreateHat(client, 617);
							CreateHat(client, 30076);
							CreateHat(client, 1016);
						}
						case 7:
						{
							CreateWeapon(client, "tf_weapon_handgun_scout_primary", 0, 220);
							CreateWeapon(client, "tf_weapon_jar_milk", 1, 222);
							CreateWeapon(client, "tf_weapon_bat_fish", 2, 221);
							CreateHat(client, 219);
						}
						case 8:
						{
							CreateWeapon(client, "tf_weapon_scattergun", 0, 1103);
							CreateWeapon(client, "tf_weapon_handgun_scout_secondary", 1, 773);
							CreateHat(client, 165);
						}
						case 9:
						{
							CreateWeapon(client, "tf_weapon_pep_brawler_blaster", 0, 772);
							CreateWeapon(client, "tf_weapon_lunchbox_drink", 1, 46);
							CreateWeapon(client, "tf_weapon_bat", 2, 660);
							CreateHat(client, 984);
							CreateHat(client, 30134);
							CreateHat(client, 987);
						}
						case 10:
						{
							CreateWeapon(client, "tf_weapon_scattergun", 0, 45);
							CreateHat(client, 166);
						}
						case 11:
						{
							CreateWeapon(client, "tf_weapon_scattergun", 0, 799);
							CreateWeapon(client, "tf_weapon_jar_milk", 1, 222);
							CreateWeapon(client, "tf_weapon_bat_giftwrap", 2, 648);
							CreateHat(client, 30479);
							CreateHat(client, 30060);
						}
						case 12:
						{
							CreateHat(client, 126);
							CreateHat(client, 30397);
							CreateHat(client, 30426);
						}
						case 13:
						{
							CreateWeapon(client, "tf_weapon_scattergun", 0, 45);
							CreateWeapon(client, "tf_weapon_lunchbox_drink", 1, 46);
							CreateWeapon(client, "tf_weapon_bat_wood", 2, 44);
							CreateHat(client, 940);
							CreateHat(client, 583);
							CreateHat(client, 744);
						}
						case 14:
						{
							CreateWeapon(client, "tf_weapon_scattergun", 0, 45);
							CreateWeapon(client, "tf_weapon_lunchbox_drink", 1, 46);
							CreateWeapon(client, "tf_weapon_bat_wood", 2, 44);
							CreateHat(client, 940);
							CreateHat(client, 583);
							CreateHat(client, 744);
						}
						case 15:
						{
							CreateWeapon(client, "tf_weapon_handgun_scout_primary", 0, 220);
							CreateWeapon(client, "tf_weapon_handgun_scout_secondary", 1, 773);
							CreateWeapon(client, "tf_weapon_bat_fish", 2, 221);
						}
						case 16:
						{
							CreateHat(client, 106);
						}
						case 17:
						{
							CreateWeapon(client, "tf_weapon_scattergun", 0, 45);
							CreateHat(client, 30357);
						}
						case 18:
						{
							CreateHat(client, 261);
						}
					}
				}
				case TFClass_Sniper:
				{
					switch (sniperLoadout[client])
					{
						case 1:
						{
							CreateHat(client, 261);
						}
						case 2:
						{
							CreateHat(client, 302);
							CreateHat(client, 743);
							CreateHat(client, 583);
							CreateWeapon(client, "tf_weapon_sniperrifle_classic", 0, 1098);
							CreateWeapon(client, "tf_weapon_jar", 1, 58);
							CreateWeapon(client, "tf_weapon_club", 2, 401);
						}
						case 3:
						{
							CreateHat(client, 261);
							CreateHat(client, 343);
							CreateHat(client, 165);
						}
						case 4:
						{
							CreateWeapon(client, "tf_weapon_sniperrifle", 0, 851);
							CreateWeapon(client, "tf_wearable", 1, 231, true);
							CreateWeapon(client, "tf_weapon_club", 2, 171);
							CreateHat(client, 600);
							CreateHat(client, 30550);
							CreateHat(client, 646);
						}
						case 5:
						{
							CreateWeapon(client, "tf_weapon_sniperrifle", 0, 526);
							CreateWeapon(client, "tf_wearable", 1, 642, true);
							CreateHat(client, 766);
							CreateHat(client, 30546);
						}
						case 6:
						{
							CreateWeapon(client, "tf_weapon_sniperrifle", 0, 664);
							CreateWeapon(client, "tf_wearable", 1, 642, true);
						}
						case 7:
						{
							CreateWeapon(client, "tf_weapon_sniperrifle", 0, 752);
							CreateWeapon(client, "tf_weapon_smg", 1, 751);
							CreateHat(client, 779);
						}
						case 8:
						{
							CreateWeapon(client, "tf_weapon_compound_bow", 0, 1092);
							CreateWeapon(client, "tf_weapon_smg", 1, 751);
							CreateWeapon(client, "tf_weapon_club", 2, 171);
							CreateHat(client, 30373);
							CreateHat(client, 314);
							CreateHat(client, 1094);
						}
						case 9:
						{
							CreateWeapon(client, "tf_weapon_sniperrifle", 0, 230);
							CreateWeapon(client, "tf_wearable", 1, 231, true);
							CreateWeapon(client, "tf_weapon_club", 2, 232);
							CreateHat(client, 229);
						}
						case 10:
						{
							CreateWeapon(client, "tf_weapon_sniperrifle", 0, 402);
							CreateWeapon(client, "tf_wearable_razorback", 1, 57, true);
							CreateWeapon(client, "tf_weapon_club", 2, 232);
							CreateHat(client, 165);
						}
						case 11:
						{
							CreateWeapon(client, "tf_weapon_sniperrifle_classic", 0, 1098);
							CreateWeapon(client, "tf_weapon_smg", 1, 751);
							CreateWeapon(client, "tf_weapon_club", 2, 401);
							CreateHat(client, 671);
							CreateHat(client, 30306);
							CreateHat(client, 500);
						}
						case 12:
						{
							CreateWeapon(client, "tf_weapon_sniperrifle", 0, 851);
							CreateHat(client, 166);
						}
						case 13:
						{
							CreateWeapon(client, "tf_weapon_sniperrifle", 0, 752);
							CreateWeapon(client, "tf_weapon_jar", 1, 58);
							CreateWeapon(client, "tf_weapon_club", 2, 401);
							CreateHat(client, 762);
						}
						case 14:
						{
							CreateWeapon(client, "tf_weapon_sniperrifle", 0, 230);
							CreateWeapon(client, "tf_weapon_jar", 1, 58);
							CreateWeapon(client, "tf_weapon_club", 2, 232);
							CreateHat(client, 261);
							CreateHat(client, 583);
							CreateHat(client, 744);
						}
						case 15:
						{
							CreateWeapon(client, "tf_weapon_sniperrifle", 0, 851);
							CreateWeapon(client, "tf_wearable_razorback", 1, 57, true);
							CreateWeapon(client, "tf_weapon_club", 2, 401);
							CreateHat(client, 30373);
							CreateHat(client, 30650);
							CreateHat(client, 626);
						}
						case 16:
						{
							CreateWeapon(client, "tf_weapon_sniperrifle", 0, 230);
							CreateWeapon(client, "tf_wearable", 1, 231, true);
							CreateWeapon(client, "tf_weapon_club", 2, 171);
							CreateHat(client, 518);
							CreateHat(client, 30599);
							CreateHat(client, 30599);
						}
						case 17:
						{
							CreateWeapon(client, "tf_weapon_sniperrifle", 0, 15000);
							CreateWeapon(client, "tf_wearable_razorback", 1, 57, true);
							CreateWeapon(client, "tf_weapon_club", 2, 232);
						}
						case 18:
						{
							CreateHat(client, 600);
						}
						case 19:
						{
							CreateWeapon(client, "tf_weapon_sniperrifle", 0, 851);
							CreateHat(client, 30397);
							CreateHat(client, 263);
							CreateHat(client, 30170);
						}
						case 20:
						{
							CreateWeapon(client, "tf_weapon_sniperrifle_decap", 0, 402);
						}
					}
				}
				case TFClass_Soldier:
				{
					switch (soldierLoadout[client])
					{
						case 1:
						{
							CreateHat(client, 261);
						}
						case 2:
						{
							CreateHat(client, 940);
							CreateHat(client, 743);
							CreateHat(client, 583);
							CreateWeapon(client, "tf_weapon_rocketlauncher_directhit", 0, 127);
							CreateWeapon(client, "tf_weapon_buff_item", 1, 129);
							CreateWeapon(client, "tf_weapon_shovel", 2, 128);
						}
						case 3:
						{
							CreateWeapon(client, "tf_weapon_rocketlauncher", 0, 513);
							CreateWeapon(client, "tf_weapon_shotgun_soldier", 1, 1153);
							CreateHat(client, 261);
							CreateHat(client, 343);
							CreateHat(client, 165);
						}
						case 4:
						{
							CreateWeapon(client, "tf_weapon_rocketlauncher", 0, 730);
							CreateWeapon(client, "tf_wearable", 1, 133, true);
							CreateWeapon(client, "tf_weapon_shovel", 2, 416);
							CreateHat(client, 852);
							CreateHat(client, 54);
							CreateHat(client, 701);
						}
						case 5:
						{
							CreateWeapon(client, "tf_weapon_particle_cannon", 0, 441);
							CreateWeapon(client, "tf_weapon_buff_item", 1, 226);
							CreateWeapon(client, "tf_weapon_shovel", 2, 775);
							CreateHat(client, 340);
							CreateHat(client, 30126);
						}
						case 6:
						{
							CreateWeapon(client, "tf_weapon_rocketlauncher_airstrike", 0, 1104);
							CreateWeapon(client, "tf_weapon_parachute", 1, 1101, true);
						}
						case 7:
						{
							CreateWeapon(client, "tf_weapon_rocketlauncher", 0, 800);
							CreateWeapon(client, "tf_wearable", 1, 444, true);
							CreateWeapon(client, "tf_weapon_shovel", 2, 447);
							CreateHat(client, 445);
							CreateHat(client, 446);
						}
						case 8:
						{
							CreateWeapon(client, "tf_weapon_rocketlauncher", 0, 228);
							CreateWeapon(client, "tf_weapon_buff_item", 1, 354);
							CreateWeapon(client, "tf_weapon_shovel", 2, 128);
							CreateHat(client, 378);
							CreateHat(client, 30388);
							CreateHat(client, 30115);
						}
						case 9:
						{
							CreateWeapon(client, "tf_weapon_rocketlauncher", 0, 228);
							CreateWeapon(client, "tf_weapon_buff_item", 1, 226);
							CreateHat(client, 227);
						}
						case 10:
						{
							CreateWeapon(client, "tf_weapon_particle_cannon", 0, 441);
							CreateWeapon(client, "tf_weapon_buff_item", 1, 354);
							CreateWeapon(client, "tf_weapon_katana", 2, 357);
							CreateHat(client, 165);
						}
						case 12:
						{
							CreateWeapon(client, "tf_weapon_rocketlauncher", 0, 513);
							CreateWeapon(client, "tf_weapon_parachute", 1, 1101, true);
							CreateHat(client, 54);
						}
						case 13:
						{
							CreateWeapon(client, "tf_weapon_rocketlauncher", 0, 414);
							CreateWeapon(client, "tf_weapon_shotgun_soldier", 1, 415);
							CreateWeapon(client, "tf_weapon_katana", 2, 357);
							CreateHat(client, 152);
							CreateHat(client, 30126);
						}
						case 14:
						{
							CreateWeapon(client, "tf_weapon_rocketlauncher_directhit", 0, 127);
							CreateWeapon(client, "tf_wearable", 1, 133, true);
							CreateWeapon(client, "tf_weapon_shovel", 2, 416);
							CreateHat(client, 126);
						}
						case 15:
						{
							CreateWeapon(client, "tf_weapon_rocketlauncher", 0, 228);
							CreateWeapon(client, "tf_weapon_buff_item", 1, 354);
							CreateWeapon(client, "tf_weapon_katana", 2, 357);
						}
						case 16:
						{
							CreateWeapon(client, "tf_weapon_rocketlauncher", 0, 513);
							CreateHat(client, 391);
						}
					}
				}
				case TFClass_DemoMan:
				{
					switch (demomanLoadout[client])
					{
						case 1:
						{
							CreateHat(client, 261);
						}
						case 2:
						{
							CreateHat(client, 116);
							CreateHat(client, 743);
							CreateHat(client, 583);
							CreateWeapon(client, "tf_weapon_pipebomblauncher", 1, 130);
						}
						case 3:
						{
							CreateHat(client, 940);
							CreateHat(client, 743);
							CreateHat(client, 583);
							CreateWeapon(client, "tf_wearable_demoshield", 1, 131, true);
							CreateWeapon(client, "tf_weapon_sword", 2, 132);
						}
						case 4:
						{
							CreateWeapon(client, "tf_weapon_grenadelauncher", 0, 1151);
							CreateWeapon(client, "tf_weapon_pipebomblauncher", 1, 130);
							CreateWeapon(client, "tf_weapon_bottle", 2, 609);
							CreateHat(client, 261);
							CreateHat(client, 343);
							CreateHat(client, 165);
						}
						case 5:
						{
							CreateWeapon(client, "tf_weapon_grenadelauncher", 0, 308);
							CreateWeapon(client, "tf_weapon_pipebomblauncher", 1, 661);
							CreateWeapon(client, "tf_weapon_stickbomb", 2, 307);
							CreateHat(client, 776);
							CreateHat(client, 390);
							CreateHat(client, 845);
						}
						case 6:
						{
							CreateWeapon(client, "tf_wearable", 0, 405, true);
							CreateWeapon(client, "tf_wearable_demoshield", 1, 406, true);
							CreateWeapon(client, "tf_weapon_katana", 2, 357);
							CreateHat(client, 359);
						}
						case 7:
						{
							CreateWeapon(client, "tf_weapon_grenadelauncher", 0, 1151);
							CreateWeapon(client, "tf_wearable_demoshield", 1, 1099, true);
							CreateWeapon(client, "tf_weapon_stickbomb", 2, 307);
						}
						case 8:
						{
							CreateWeapon(client, "tf_wearable", 0, 405, true);
							CreateWeapon(client, "tf_weapon_pipebomblauncher", 1, 806);
							CreateWeapon(client, "tf_weapon_shovel", 2, 154);
							CreateHat(client, 30429);
							CreateHat(client, 30430);
							CreateHat(client, 30431);
						}
						case 9:
						{
							CreateWeapon(client, "tf_wearable", 0, 405, true);
							CreateWeapon(client, "tf_wearable_demoshield", 1, 406, true);
							CreateWeapon(client, "tf_weapon_sword", 2, 1082);
							CreateHat(client, 342);
							CreateHat(client, 874);
						}
						case 10:
						{
							CreateWeapon(client, "tf_weapon_grenadelauncher", 0, 308);
							CreateWeapon(client, "tf_weapon_stickbomb", 2, 307);
							CreateHat(client, 306);
						}
						case 11:
						{
							CreateWeapon(client, "tf_weapon_grenadelauncher", 0, 308);
							CreateWeapon(client, "tf_weapon_pipebomblauncher", 1, 130);
							CreateWeapon(client, "tf_weapon_stickbomb", 2, 307);
							CreateHat(client, 165);
						}
						case 12:
						{
							CreateWeapon(client, "tf_wearable", 0, 608, true);
							CreateWeapon(client, "tf_weapon_pipebomblauncher", 1, 1150);
							CreateWeapon(client, "tf_weapon_katana", 2, 357);
							CreateHat(client, 30180);
							CreateHat(client, 30110);
							CreateHat(client, 610);
						}
						case 13:
						{
							CreateWeapon(client, "tf_weapon_cannon", 0, 996);
							CreateHat(client, 166);
						}
						case 14:
						{
							CreateWeapon(client, "tf_weapon_grenadelauncher", 0, 1151);
							CreateWeapon(client, "tf_weapon_pipebomblauncher", 1, 130);
							CreateWeapon(client, "tf_weapon_katana", 2, 357);
							CreateHat(client, 845);
							CreateHat(client, 830);
						}
						case 15:
						{
							CreateWeapon(client, "tf_weapon_cannon", 0, 996);
							CreateWeapon(client, "tf_wearable_demoshield", 1, 406, true);
							CreateWeapon(client, "tf_weapon_stickbomb", 2, 307);
						}
						case 16:
						{
							CreateWeapon(client, "tf_weapon_cannon", 0, 996);
							CreateHat(client, 30357);
						}
					}
				}
				case TFClass_Medic:
				{
					switch (medicLoadout[client])
					{
						case 1:
						{
							CreateHat(client, 261);
						}
						case 2:
						{
							CreateHat(client, 756);
							CreateHat(client, 743);
							CreateHat(client, 583);
							CreateWeapon(client, "tf_weapon_medigun", 1, 35);
							CreateWeapon(client, "tf_weapon_syringegun_medic", 0, 36);
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 37);
						}
						case 3:
						{
							CreateWeapon(client, "tf_weapon_crossbow", 0, 305);
							CreateWeapon(client, "tf_weapon_medigun", 1, 411);
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 304);
							CreateHat(client, 261);
							CreateHat(client, 343);
							CreateHat(client, 165);
						}
						case 4:
						{
							CreateWeapon(client, "tf_weapon_crossbow", 0, 1079);
							CreateWeapon(client, "tf_weapon_medigun", 1, 796);
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 413);
							CreateHat(client, 30311);
							CreateHat(client, 30312);
							CreateHat(client, 30410);
						}
						case 5:
						{
							CreateWeapon(client, "tf_weapon_crossbow", 0, 305);
							CreateWeapon(client, "tf_weapon_medigun", 1, 35);
							CreateHat(client, 828);
							CreateHat(client, 378);
						}
						case 6:
						{
							CreateWeapon(client, "tf_weapon_medigun", 1, 411);
						}
						case 7:
						{
							CreateWeapon(client, "tf_weapon_crossbow", 0, 305);
							CreateWeapon(client, "tf_weapon_medigun", 1, 796);
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 304);
							CreateHat(client, 388);
							CreateHat(client, 657);
						}
						case 8:
						{
							CreateWeapon(client, "tf_weapon_medigun", 1, 663);
							CreateWeapon(client, "tf_weapon_crossbow", 0, 1079);
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 304);
							CreateHat(client, 143);
							CreateHat(client, 101);
							CreateHat(client, 30361);
						}
						case 9:
						{
							CreateWeapon(client, "tf_weapon_medigun", 1, 411);
							CreateWeapon(client, "tf_weapon_syringegun_medic", 0, 412);
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 413);
							CreateHat(client, 30490);
							CreateHat(client, 303);
						}
						case 10:
						{
							CreateWeapon(client, "tf_weapon_crossbow", 0, 305);
							CreateWeapon(client, "tf_weapon_medigun", 1, 998);
							CreateHat(client, 165);
						}
						case 11:
						{
							CreateHat(client, 5622);
						}
						case 12:
						{
							CreateWeapon(client, "tf_weapon_medigun", 1, 35);
							CreateWeapon(client, "tf_weapon_syringegun_medic", 0, 412);
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 37);
							CreateHat(client, 432);
							CreateHat(client, 343);
							CreateHat(client, 538);
						}
						case 13:
						{
							CreateWeapon(client, "tf_weapon_crossbow", 0, 305);
							CreateWeapon(client, "tf_weapon_medigun", 1, 998);
							CreateHat(client, 166);
						}
						case 14:
						{
							CreateWeapon(client, "tf_weapon_crossbow", 0, 305);
							CreateWeapon(client, "tf_weapon_medigun", 1, 805);
							CreateHat(client, 165);
							CreateHat(client, 30119);
							CreateHat(client, 987);
						}
						case 15:
						{
							CreateWeapon(client, "tf_weapon_syringegun_medic", 0, 36);
							CreateWeapon(client, "tf_weapon_medigun", 1, 35);
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 37);
							CreateHat(client, 30066);
							CreateHat(client, 143);
							CreateHat(client, 738);
						}
						case 16:
						{
							CreateWeapon(client, "tf_weapon_medigun", 1, 663);
							CreateWeapon(client, "tf_weapon_crossbow", 0, 1079);
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 304);
							CreateHat(client, 378);
							CreateHat(client, 878);
							CreateHat(client, 522);
						}
						case 17:
						{
							CreateHat(client, 261);
							CreateHat(client, 583);
							CreateHat(client, 744);
						}
						case 18:
						{
							CreateWeapon(client, "tf_weapon_crossbow", 0, 1079);
							CreateWeapon(client, "tf_weapon_medigun", 1, 663);
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 304);
							CreateHat(client, 778);
							CreateHat(client, 30419);
							CreateHat(client, 978);
						}
						case 19:
						{
							CreateWeapon(client, "tf_weapon_syringegun_medic", 0, 36);
							CreateWeapon(client, "tf_weapon_medigun", 1, 35);
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 173);
							CreateHat(client, 867);
							CreateHat(client, 868);
						}
						case 20:
						{
							CreateWeapon(client, "tf_weapon_syringegun_medic", 0, 36);
							CreateWeapon(client, "tf_weapon_medigun", 1, 411);
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 37);
						}
						case 21:
						{
							CreateWeapon(client, "tf_weapon_syringegun_medic", 0, 36);
							CreateWeapon(client, "tf_weapon_medigun", 1, 411);
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 37);
							CreateHat(client, 101);
							CreateHat(client, 143);
						}
						case 22:
						{
							CreateWeapon(client, "tf_weapon_crossbow", 0, 305);
							CreateWeapon(client, "tf_weapon_medigun", 1, 998);
							CreateHat(client, 30357);
						}
					}
				}
				case TFClass_Heavy:
				{
					switch (heavyLoadout[client])
					{
						case 1:
						{
							CreateHat(client, 261);
							CreateWeapon(client, "tf_weapon_fists", 2, 239);
						}
						case 2:
						{
							CreateHat(client, 940);
							CreateHat(client, 743);
							CreateHat(client, 583);
							CreateWeapon(client, "tf_weapon_minigun", 0, 41);
							CreateWeapon(client, "tf_weapon_fists", 2, 43);
						}
						case 3:
						{
							CreateWeapon(client, "tf_weapon_minigun", 0, 424);
							CreateWeapon(client, "tf_weapon_shotgun_hwg", 1, 425);
							CreateWeapon(client, "tf_weapon_fists", 2, 331);
							CreateHat(client, 261);
							CreateHat(client, 343);
							CreateHat(client, 165);
						}
						case 4:
						{
							CreateWeapon(client, "tf_weapon_minigun", 0, 802);
							CreateWeapon(client, "tf_weapon_fists", 2, 587);
							CreateHat(client, 145);
							CreateHat(client, 30319);
							CreateHat(client, 777);
						}
						case 5:
						{
							CreateWeapon(client, "tf_weapon_minigun", 0, 298);
							CreateHat(client, 30545);
						}
						case 6:
						{
							CreateWeapon(client, "tf_weapon_minigun", 0, 811);
							CreateWeapon(client, "tf_weapon_fists", 2, 5);
						}
						case 7:
						{
							CreateWeapon(client, "tf_weapon_minigun", 0, 424);
							CreateWeapon(client, "tf_weapon_shotgun_hwg", 1, 425);
							CreateWeapon(client, "tf_weapon_fists", 2, 426);
							CreateHat(client, 427);
							CreateHat(client, 30085);
						}
						case 8:
						{
							CreateWeapon(client, "tf_weapon_minigun", 0, 654);
							CreateWeapon(client, "tf_weapon_shotgun_hwg", 1, 1153);
							CreateWeapon(client, "tf_weapon_fists", 2, 239);
							CreateHat(client, 96);
							CreateHat(client, 30342);
							CreateHat(client, 30343);
						}
						case 9:
						{
							CreateWeapon(client, "tf_weapon_minigun", 0, 312);
							CreateWeapon(client, "tf_weapon_fists", 2, 310);
							CreateHat(client, 309);
						}
						case 10:
						{
							CreateWeapon(client, "tf_weapon_minigun", 0, 312);
							CreateWeapon(client, "tf_weapon_shotgun_hwg", 1, 425);
							CreateWeapon(client, "tf_weapon_fists", 2, 331);
							CreateHat(client, 165);
						}
						case 11:
						{
							CreateWeapon(client, "tf_weapon_minigun", 0, 958);
							CreateWeapon(client, "tf_weapon_shotgun_hwg", 1, 1153);
							CreateWeapon(client, "tf_weapon_fists", 2, 239);
							CreateHat(client, 162);
							CreateHat(client, 143);
							CreateHat(client, 30302);
						}
						case 12:
						{
							CreateWeapon(client, "tf_weapon_minigun", 0, 811);
							CreateHat(client, 166);
						}
						case 13:
						{
							CreateWeapon(client, "tf_weapon_minigun", 0, 802);
							CreateWeapon(client, "tf_weapon_shotgun_hwg", 1, 15047);
							CreateWeapon(client, "tf_weapon_fists", 2, 587);
							CreateHat(client, 97);
							CreateHat(client, 30302);
						}
						case 14:
						{
							CreateWeapon(client, "tf_weapon_minigun", 0, 654);
							CreateWeapon(client, "tf_weapon_shotgun_hwg", 1, 425);
							CreateWeapon(client, "tf_weapon_fists", 2, 331);
							CreateHat(client, 30085);
							CreateHat(client, 601);
							CreateHat(client, 30178);
						}
						case 15:
						{
							CreateWeapon(client, "tf_weapon_minigun", 0, 424);
							CreateWeapon(client, "tf_weapon_shotgun_hwg", 1, 1153);
							CreateWeapon(client, "tf_weapon_fists", 2, 310);
						}
						case 16:
						{
							CreateHat(client, 246);
							CreateHat(client, 757);
						}
						case 17:
						{
							CreateWeapon(client, "tf_weapon_minigun", 0, 811);
							CreateHat(client, 30357);
						}
					}
				}
				case TFClass_Pyro:
				{
					switch (pyroLoadout[client])
					{
						case 1:
						{
							CreateHat(client, 261);
						}
						case 2:
						{
							CreateHat(client, 940);
							CreateHat(client, 743);
							CreateHat(client, 583);
							CreateWeapon(client, "tf_weapon_flamethrower", 0, 40);
							CreateWeapon(client, "tf_weapon_flaregun", 1, 39);
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 38);
						}
						case 3:
						{
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 153);
							CreateWeapon(client, "tf_weapon_shotgun_pyro", 1, 1153);
							CreateHat(client, 261);
							CreateHat(client, 343);
							CreateHat(client, 165);
						}
						case 4:
						{
							CreateWeapon(client, "tf_weapon_flamethrower", 0, 1146);
							CreateWeapon(client, "tf_weapon_jar_gas", 1, 1180);
							CreateWeapon(client, "tf_weapon_slap", 2, 1181);
							CreateHat(client, 105);
							CreateHat(client, 387);
							CreateHat(client, 632);
						}
						case 5:
						{
							CreateWeapon(client, "tf_weapon_flamethrower", 0, 15017);
							CreateWeapon(client, "tf_weapon_flaregun", 1, 351);
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 38);
							CreateHat(client, 394);
						}
						case 6:
						{
							CreateWeapon(client, "tf_weapon_flamethrower", 0, 741);
							CreateWeapon(client, "tf_weapon_shotgun_pyro", 1, 415);
						}
						case 7:
						{
							CreateWeapon(client, "tf_weapon_flamethrower", 0, 594);
							CreateWeapon(client, "tf_weapon_flaregun_revenge", 1, 595);
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 593);
							CreateHat(client, 597);
							CreateHat(client, 596);
						}
						case 8:
						{
							CreateWeapon(client, "tf_weapon_rocketlauncher_fireball", 0, 1178);
							CreateWeapon(client, "tf_weapon_flaregun", 1, 1081);
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 214);
							CreateHat(client, 435);
							CreateHat(client, 632);
							CreateHat(client, 30169);
						}
						case 9:
						{
							CreateWeapon(client, "tf_weapon_flamethrower", 0, 215);
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 214);
							CreateHat(client, 213);
						}
						case 10:
						{
							CreateWeapon(client, "tf_weapon_flaregun", 1, 39);
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 214);
							CreateHat(client, 165);
						}
						case 11:
						{
							CreateWeapon(client, "tf_weapon_flamethrower", 0, 1146);
							CreateWeapon(client, "tf_weapon_shotgun_pyro", 1, 415);
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 348);
							CreateHat(client, 213);
							CreateHat(client, 30400);
							CreateHat(client, 570);
						}
						case 12:
						{
							CreateWeapon(client, "tf_weapon_flamethrower", 0, 659);
							CreateWeapon(client, "tf_weapon_jar_gas", 1, 1180);
							CreateHat(client, 166);
						}
						case 13:
						{
							CreateWeapon(client, "tf_weapon_flamethrower", 0, 215);
							CreateWeapon(client, "tf_weapon_shotgun_pyro", 1, 1153);
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 153);
							CreateHat(client, 570);
							CreateHat(client, 937);
							CreateHat(client, 30309);
						}
						case 14:
						{
							CreateWeapon(client, "tf_weapon_flamethrower", 0, 594);
							CreateWeapon(client, "tf_weapon_flaregun", 1, 740);
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 593);
							CreateHat(client, 570);
							CreateHat(client, 30305);
							CreateHat(client, 30168);
						}
						case 15:
						{
							CreateWeapon(client, "tf_weapon_flamethrower", 0, 594);
							CreateWeapon(client, "tf_weapon_jar_gas", 1, 1180);
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 214);
							CreateHat(client, 30089);
							CreateHat(client, 570);
							CreateHat(client, 627);
						}
						case 16:
						{
							CreateHat(client, 105);
							CreateHat(client, 30169);
							CreateHat(client, 387);
						}
						case 17:
						{
							CreateWeapon(client, "tf_weapon_flamethrower", 0, 215);
							CreateWeapon(client, "tf_weapon_shotgun_pyro", 1, 415);
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 326);
						}
						case 18:
						{
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 38);
							CreateHat(client, 213);
						}
						case 19:
						{
							CreateWeapon(client, "tf_weapon_flamethrower", 0, 659);
							CreateWeapon(client, "tf_weapon_jar_gas", 1, 1180);
							CreateHat(client, 30357);
						}
					}
				}
				case TFClass_Spy:
				{
					switch (spyLoadout[client])
					{
						case 1:
						{
							CreateHat(client, 261);
						}
						case 2:
						{
							CreateHat(client, 668);
							CreateHat(client, 743);
							CreateHat(client, 583);
							CreateWeapon(client, "tf_weapon_revolver", 0, 61);
							CreateWeapon(client, "tf_weapon_invis", 4, 59);
						}
						case 3:
						{
							CreateWeapon(client, "tf_weapon_revolver", 0, 224);
							CreateWeapon(client, "tf_weapon_knife", 2, 649);
							CreateWeapon(client, "tf_weapon_invis", 4, 297);
							CreateHat(client, 261);
							CreateHat(client, 343);
							CreateHat(client, 165);
						}
						case 4:
						{
							CreateWeapon(client, "tf_weapon_revolver", 0, 224);
							CreateWeapon(client, "tf_weapon_knife", 2, 461);
							CreateHat(client, 437);
							CreateHat(client, 879);
						}
						case 5:
						{
							CreateWeapon(client, "tf_weapon_revolver", 0, 61);
							CreateWeapon(client, "tf_weapon_knife", 2, 727);
							CreateHat(client, 459);
							CreateHat(client, 30353);
						}
						case 6:
						{
							CreateWeapon(client, "tf_weapon_revolver", 0, 161);
							CreateWeapon(client, "tf_weapon_knife", 2, 574);
						}
						case 7:
						{
							CreateWeapon(client, "tf_weapon_revolver", 0, 224);
							CreateWeapon(client, "tf_weapon_knife", 2, 225);
							CreateWeapon(client, "tf_weapon_invis", 4, 60);
							CreateHat(client, 223);
						}
						case 8:
						{
							CreateWeapon(client, "tf_weapon_revolver", 0, 61);
							CreateWeapon(client, "tf_weapon_knife", 2, 665);
							CreateWeapon(client, "tf_weapon_sapper", 1, 1102);
							CreateWeapon(client, "tf_weapon_invis", 4, 947);
							CreateHat(client, 30405);
							CreateHat(client, 30404);
							CreateHat(client, 30467);
						}
						case 9:
						{
							CreateWeapon(client, "tf_weapon_revolver", 0, 460);
							CreateWeapon(client, "tf_weapon_knife", 2, 461);
							CreateWeapon(client, "tf_weapon_sapper", 1, 810);
							CreateWeapon(client, "tf_weapon_invis", 4, 297);
							CreateHat(client, 459);
							CreateHat(client, 462);
						}
						case 10:
						{
							CreateWeapon(client, "tf_weapon_revolver", 0, 224);
							CreateWeapon(client, "tf_weapon_knife", 2, 727);
							CreateHat(client, 165);
						}
						case 11:
						{
							CreateWeapon(client, "tf_weapon_revolver", 0, 61);
							CreateWeapon(client, "tf_weapon_sapper", 1, 1102);
							CreateWeapon(client, "tf_weapon_knife", 2, 225);
							CreateWeapon(client, "tf_weapon_invis", 4, 59);
							CreateHat(client, 30397);
							CreateHat(client, 30414);
							CreateHat(client, 30546);
						}
						case 12:
						{
							CreateWeapon(client, "tf_weapon_revolver", 0, 224);
							CreateWeapon(client, "tf_weapon_knife", 2, 638);
							CreateHat(client, 166);
						}
						case 13:
						{
							CreateWeapon(client, "tf_weapon_revolver", 0, 15149);
							CreateWeapon(client, "tf_weapon_sapper", 1, 810);
							CreateWeapon(client, "tf_weapon_knife", 2, 15118);
							CreateWeapon(client, "tf_weapon_invis", 4, 60);
							CreateHat(client, 180);
						}
						case 14:
						{
							CreateWeapon(client, "tf_weapon_revolver", 0, 525);
							CreateWeapon(client, "tf_weapon_sapper", 1, 1102);
							CreateWeapon(client, "tf_weapon_knife", 2, 356);
							CreateWeapon(client, "tf_weapon_invis", 4, 947);
							CreateWeapon(client, "tf_weapon_pda_spy", 3, 27);
							CreateHat(client, 30177);
							CreateHat(client, 30389);
							CreateHat(client, 30397);
						}
						case 15:
						{
							CreateWeapon(client, "tf_weapon_revolver", 0, 525);
							CreateWeapon(client, "tf_weapon_sapper", 1, 1102);
							CreateWeapon(client, "tf_weapon_knife", 2, 356);
							CreateWeapon(client, "tf_weapon_invis", 4, 59);
							CreateWeapon(client, "tf_weapon_pda_spy", 3, 27);
							CreateHat(client, 397);
							CreateHat(client, 337);
							CreateHat(client, 30189);
						}
						case 16:
						{
							CreateWeapon(client, "tf_weapon_revolver", 0, 224);
							CreateWeapon(client, "tf_weapon_knife", 2, 461);
							CreateWeapon(client, "tf_weapon_invis", 4, 59);
						}
						case 17:
						{
							CreateWeapon(client, "tf_weapon_revolver", 0, 224);
							CreateWeapon(client, "tf_weapon_knife", 2, 638);
							CreateHat(client, 30357);
						}
						case 18:
						{
							CreateWeapon(client, "tf_weapon_knife", 2, 225);
						}
					}
				}
				case TFClass_Engineer:
				{
					switch (engineerLoadout[client])
					{
						case 1:
						{
							CreateHat(client, 261);
						}
						case 2:
						{
							CreateHat(client, 941);
							CreateHat(client, 743);
							CreateHat(client, 583);
							CreateWeapon(client, "tf_weapon_sentry_revenge", 0, 141);
							CreateWeapon(client, "tf_weapon_laser_pointer", 1, 140);
						}
						case 3:
						{
							CreateWeapon(client, "tf_weapon_shotgun_primary", 0, 527);
							CreateWeapon(client, "tf_weapon_mechanical_arm", 1, 528);
							CreateHat(client, 261);
							CreateHat(client, 343);
							CreateHat(client, 165);
						}
						case 4:
						{
							CreateWeapon(client, "tf_weapon_sentry_revenge", 0, 1004);
							CreateWeapon(client, "tf_weapon_laser_pointer", 1, 1086);
							CreateWeapon(client, "tf_weapon_wrench", 2, 662);
							CreateHat(client, 420);
							CreateHat(client, 496);
							CreateHat(client, 30306);
						}
						case 5:
						{
							CreateWeapon(client, "tf_weapon_shotgun_primary", 0, 15085);
							CreateWeapon(client, "tf_weapon_wrench", 2, 329);
							CreateHat(client, 48);
						}
						case 6:
						{
							CreateWeapon(client, "tf_weapon_sentry_revenge", 0, 141);
							CreateWeapon(client, "tf_weapon_wrench", 2, 155);
						}
						case 7:
						{
							CreateWeapon(client, "tf_weapon_shotgun_primary", 0, 15109);
							CreateWeapon(client, "tf_weapon_wrench", 2, 329);
							CreateHat(client, 30402);
							CreateHat(client, 30403);
						}
						case 8:
						{
							CreateWeapon(client, "tf_weapon_shotgun_primary", 0, 15152);
							CreateWeapon(client, "tf_weapon_laser_pointer", 1, 1086);
							CreateWeapon(client, "tf_weapon_wrench", 2, 169);
							CreateHat(client, 162);
							CreateHat(client, 143);
							CreateHat(client, 30414);
						}
						case 9:
						{
							CreateWeapon(client, "tf_weapon_drg_pomson", 0, 588);
							CreateWeapon(client, "tf_weapon_wrench", 2, 589);
							CreateHat(client, 590);
							CreateHat(client, 591);
						}
						case 10:
						{
							CreateWeapon(client, "tf_weapon_sentry_revenge", 0, 141);
							CreateWeapon(client, "tf_weapon_wrench", 2, 155);
							CreateHat(client, 165);
						}
						case 11:
						{
							CreateHat(client, 30510);
						}
						case 12:
						{
							CreateHat(client, 116);
						}
						case 13:
						{
							CreateWeapon(client, "tf_weapon_sentry_revenge", 0, 141);
							CreateWeapon(client, "tf_weapon_wrench", 2, 155);
							CreateHat(client, 166);
						}
						case 14:
						{
							CreateWeapon(client, "tf_weapon_shotgun_primary", 0, 1153);
							CreateWeapon(client, "tf_weapon_pistol", 1, 294);
							CreateWeapon(client, "tf_weapon_wrench", 2, 589);
							CreateHat(client, 165);
							CreateHat(client, 30119);
							CreateHat(client, 987);
						}
						case 15:
						{
							CreateWeapon(client, "tf_weapon_shotgun_primary", 0, 1141);
							CreateWeapon(client, "tf_weapon_pistol", 1, 30666);
							CreateWeapon(client, "tf_weapon_wrench", 2, 329);
							CreateHat(client, 30066);
							CreateHat(client, 143);
							CreateHat(client, 733);
						}
						case 16:
						{
							CreateWeapon(client, "tf_weapon_sentry_revenge", 0, 1004);
							CreateWeapon(client, "tf_weapon_mechanical_arm", 1, 528);
							CreateWeapon(client, "tf_weapon_wrench", 2, 169);
							CreateHat(client, 94);
							CreateHat(client, 5621);
							CreateHat(client, 755);
						}
						case 17:
						{
							CreateWeapon(client, "tf_weapon_laser_pointer", 1, 140);
							CreateHat(client, 261);
							CreateHat(client, 583);
							CreateHat(client, 744);
						}
						case 18:
						{
							CreateWeapon(client, "tf_weapon_sentry_revenge", 0, 1004);
							CreateWeapon(client, "tf_weapon_laser_pointer", 1, 1086);
							CreateWeapon(client, "tf_weapon_wrench", 2, 169);
							CreateHat(client, 755);
							CreateHat(client, 94);
							CreateHat(client, 30412);
						}
						case 19:
						{
							CreateWeapon(client, "tf_weapon_shotgun_building_rescue", 0, 997);
							CreateWeapon(client, "tf_weapon_pistol", 1, 30666);
							CreateWeapon(client, "tf_weapon_wrench", 2, 329);
							CreateHat(client, 30408);
							CreateHat(client, 30341);
							CreateHat(client, 30407);
						}
						case 20:
						{
							CreateWeapon(client, "tf_weapon_sentry_revenge", 0, 141);
							CreateWeapon(client, "tf_weapon_mechanical_arm", 1, 528);
							CreateWeapon(client, "tf_weapon_wrench", 2, 329);
						}
						case 21:
						{
							CreateHat(client, 162);
						}
						case 22:
						{
							CreateWeapon(client, "tf_weapon_sentry_revenge", 0, 141);
							CreateWeapon(client, "tf_weapon_wrench", 2, 155);
							CreateHat(client, 30357);
						}
						case 23:
						{
							CreateWeapon(client, "tf_weapon_wrench", 2, 169);
						}
					}
				}
			}

			EquipWeaponSlot(client, 0);
		}
		else
		{
			switch (TF2_GetPlayerClass(client))
			{
				case TFClass_Scout:
				{
					int rnd = crandomint(1, 12);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_bat_wood", 2, 44);
						case 2:
							CreateWeapon(client, "tf_weapon_bat_fish", 2, 572);
						case 3:
							CreateWeapon(client, "tf_weapon_bat", 2, 317);
						case 4:
							CreateWeapon(client, "tf_weapon_bat", 2, 349);
						case 5:
							CreateWeapon(client, "tf_weapon_bat", 2, 355);
						case 6:
							CreateWeapon(client, "tf_weapon_bat_giftwrap", 2, 648);
						case 7:
							CreateWeapon(client, "tf_weapon_bat", 2, 450);
						case 8:
							CreateWeapon(client, "tf_weapon_bat_fish", 2, 221);
						case 9:
							CreateWeapon(client, "tf_weapon_bat", 2, 1013);
						case 10:
							CreateWeapon(client, "tf_weapon_bat", 2, 939);
						case 11:
							CreateWeapon(client, "tf_weapon_bat", 2, 880);
						case 12:
							CreateWeapon(client, "tf_weapon_bat", 2, 474);
					}
				}
				case TFClass_Sniper:
				{
					int rnd = crandomint(1, 3);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_wearable", 1, 57, true);
						case 2:
							CreateWeapon(client, "tf_wearable", 1, 231, true);
						case 3:
							CreateWeapon(client, "tf_wearable", 1, 642, true);
					}

					rnd = crandomint(1, 7);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_club", 2, 171);
						case 2:
							CreateWeapon(client, "tf_weapon_club", 2, 232);
						case 3:
							CreateWeapon(client, "tf_weapon_club", 2, 401);
						case 4:
							CreateWeapon(client, "tf_weapon_club", 2, 1013);
						case 5:
							CreateWeapon(client, "tf_weapon_club", 2, 939);
						case 6:
							CreateWeapon(client, "tf_weapon_club", 2, 880);
						case 7:
							CreateWeapon(client, "tf_weapon_club", 2, 474);
					}
				}
				case TFClass_Soldier:
				{
					int rnd = crandomint(1, 2);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_wearable", 1, 133, true);
						case 2:
							CreateWeapon(client, "tf_wearable", 1, 444, true);
					}

					rnd = crandomint(1, 9);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_shovel", 2, 128);
						case 2:
							CreateWeapon(client, "tf_weapon_shovel", 2, 154);
						case 3:
							CreateWeapon(client, "tf_weapon_shovel", 2, 447);
						case 4:
							CreateWeapon(client, "tf_weapon_shovel", 2, 775);
						case 5:
							CreateWeapon(client, "tf_weapon_katana", 2, 357);
						case 6:
							CreateWeapon(client, "tf_weapon_shovel", 2, 1013);
						case 7:
							CreateWeapon(client, "tf_weapon_shovel", 2, 939);
						case 8:
							CreateWeapon(client, "tf_weapon_shovel", 2, 880);
						case 9:
							CreateWeapon(client, "tf_weapon_shovel", 2, 474);
					}
				}
				case TFClass_DemoMan:
				{
					int rnd = crandomint(1, 2);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_wearable", 0, 405, true);
						case 2:
							CreateWeapon(client, "tf_wearable", 0, 608, true);
					}

					rnd = crandomint(1, 3);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_wearable_demoshield", 1, 131, true);
						case 2:
							CreateWeapon(client, "tf_wearable_demoshield", 1, 406, true);
						case 3:
							CreateWeapon(client, "tf_wearable_demoshield", 1, 1099, true);
					}
				
					rnd = crandomint(1, 13);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_sword", 2, 132);
						case 2:
							CreateWeapon(client, "tf_weapon_shovel", 2, 154);
						case 3:
							CreateWeapon(client, "tf_weapon_sword", 2, 172);
						case 4:
							CreateWeapon(client, "tf_weapon_stickbomb", 2, 307);
						case 5:
							CreateWeapon(client, "tf_weapon_sword", 2, 327);
						case 6:
							CreateWeapon(client, "tf_weapon_katana", 2, 357);
						case 7:
							CreateWeapon(client, "tf_weapon_sword", 2, 482);
						case 8:
							CreateWeapon(client, "tf_weapon_bottle", 2, 609);
						case 9:
							CreateWeapon(client, "tf_weapon_bottle", 2, 1013);
						case 10:
							CreateWeapon(client, "tf_weapon_bottle", 2, 939);
						case 11:
							CreateWeapon(client, "tf_weapon_bottle", 2, 880);
						case 12:
							CreateWeapon(client, "tf_weapon_bottle", 2, 474);
						case 13:
							CreateWeapon(client, "tf_weapon_sword", 2, 404);
					}
				}
				case TFClass_Medic:
				{
					int rnd = crandomint(1, 8);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 37);
						case 2:
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 173);
						case 3:
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 304);
						case 4:
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 413);
						case 5:
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 1013);
						case 6:
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 939);
						case 7:
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 880);
						case 8:
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 474);
					}
				}
				case TFClass_Heavy:
				{
					int rnd = crandomint(1, 11);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_fists", 2, 43);
						case 2:
							CreateWeapon(client, "tf_weapon_fists", 2, 239);
						case 3:
							CreateWeapon(client, "tf_weapon_fists", 2, 310);
						case 4:
							CreateWeapon(client, "tf_weapon_fists", 2, 331);
						case 5:
							CreateWeapon(client, "tf_weapon_fists", 2, 426);
						case 6:
							CreateWeapon(client, "tf_weapon_fists", 2, 656);
						case 7:
							CreateWeapon(client, "tf_weapon_fists", 2, 587);
						case 8:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 1013);
						case 9:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 939);
						case 10:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 880);
						case 11:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 474);
					}
				}
				case TFClass_Pyro:
				{
					int rnd = crandomint(1, 15);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 38);
						case 2:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 153);
						case 3:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 214);
						case 4:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 326);
						case 5:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 348);
						case 6:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 593);
						case 7:
							CreateWeapon(client, "tf_weapon_breakable_sign", 2, 813);
						case 8:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 457);
						case 9:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 739);
						case 10:
							CreateWeapon(client, "tf_weapon_slap", 2, 1181);
						case 11:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 466);
						case 12:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 1013);
						case 13:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 939);
						case 14:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 880);
						case 15:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 474);
					}
				}
				case TFClass_Spy:
				{
					int rnd = crandomint(1, 6);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_knife", 2, 356);
						case 2:
							CreateWeapon(client, "tf_weapon_knife", 2, 461);
						case 3:
							CreateWeapon(client, "tf_weapon_knife", 2, 649);
						case 4:
							CreateWeapon(client, "tf_weapon_knife", 2, 638);
						case 5:
							CreateWeapon(client, "tf_weapon_knife", 2, 225);
						case 6:
							CreateWeapon(client, "tf_weapon_knife", 2, 574);
					}

					rnd = 1;
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_invis", 4, 947);
					}
				}
				case TFClass_Engineer:
				{
					int rnd = crandomint(1, 4);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_wrench", 2, 155);
						case 2:
							CreateWeapon(client, "tf_weapon_wrench", 2, 329);
						case 3:
							CreateWeapon(client, "tf_weapon_wrench", 2, 589);
						case 4:
							CreateWeapon(client, "tf_weapon_robot_arm", 2, 142);
					}
				}
			}
		}
	}

	return Plugin_Continue;
}

public void EventSuddenDeath(Handle event, const char[] name, bool dontBroadcast)
{
	g_bSuddenDeathMode = true;
}

public void EventRoundReset(Handle event, const char[] name, bool dontBroadcast)
{
	g_bSuddenDeathMode = false;
}

stock bool CreateWeapon(const int client, const char[] classname, const int slot, const int itemindex, const bool wearable = false)
{
	int weapon = CreateEntityByName(classname);
	if (!IsValidEntity(weapon))
	{
		LogError("Failed to create a valid entity with class name [%s]! Skipping.", classname);
		return false;
	}
	
	char entclass[64];
	GetEntityNetClass(weapon, entclass, sizeof(entclass));
	SetEntProp(weapon, Prop_Send, "m_iItemDefinitionIndex", itemindex);
	SetEntData(weapon, FindSendPropInfo(entclass, "m_iEntityQuality"), 6);
	SetEntData(weapon, FindSendPropInfo(entclass, "m_hOwnerEntity"), client);
	SetEntProp(weapon, Prop_Send, "m_iEntityLevel", crandomint(1, 99));
	SetEntProp(weapon, Prop_Send, "m_bInitialized", 1);
	if (!DispatchSpawn(weapon)) 
	{
		LogError("The created weapon entity [Class name: %s, Item index: %i, Index: %i], failed to spawn! Skipping.", classname, itemindex, weapon);
		AcceptEntityInput(weapon, "Kill");
		return false;
	}

	switch (itemindex)
	{
		case 810:
		{
			SetEntProp(weapon, Prop_Send, "m_iObjectType", 3);
			SetEntProp(weapon, Prop_Data, "m_iSubType", 3);
			SetEntProp(weapon, Prop_Send, "m_aBuildableObjectTypes", 0, _, 0);
			SetEntProp(weapon, Prop_Send, "m_aBuildableObjectTypes", 0, _, 1);
			SetEntProp(weapon, Prop_Send, "m_aBuildableObjectTypes", 0, _, 2);
			SetEntProp(weapon, Prop_Send, "m_aBuildableObjectTypes", 1, _, 3);
		}
		case 998:
			SetEntProp(weapon, Prop_Send, "m_nChargeResistType", crandomint(0, 2));
		case 1178:
			SetEntData(client, FindSendPropInfo("CTFPlayer", "m_iAmmo") + (GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType", 1) * 4), 40, 4);
		case 39, 351, 740:
			SetEntData(client, FindSendPropInfo("CTFPlayer", "m_iAmmo") + (GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType", 1) * 4), 16, 4);
	}

	if (slot > -1)
		TF2_RemoveWeaponSlot(client, slot);

	if (!wearable) 
		SDKCall(g_hWeaponEquip, client, weapon);
	else 
		SDKCall(g_hWWeaponEquip, client, weapon);

	if (slot > -1 && !wearable && GetPlayerWeaponSlot(client, slot) != weapon)
	{
		LogError("The created weapon entity [Class name: %s, Item index: %i, Index: %i], failed to equip! This is probably caused by invalid gamedata.", classname, itemindex, weapon);
		AcceptEntityInput(weapon, "Kill");
		return false;
	}

	return true;
}

stock bool CreateHat(const int client, const int itemindex, const int quality = 6)
{
	int hat = CreateEntityByName("tf_wearable");
	if (!IsValidEntity(hat))
	{
		LogError("Failed to create a valid entity with class name [tf_wearable]! Skipping.");
		return false;
	}
	
	char entclass[64];
	GetEntityNetClass(hat, entclass, sizeof(entclass));
	SetEntProp(hat, Prop_Send, "m_iItemDefinitionIndex", itemindex);
	SetEntData(hat, FindSendPropInfo(entclass, "m_hOwnerEntity"), client);
	SetEntData(hat, FindSendPropInfo(entclass, "m_iEntityQuality"), quality);
	SetEntProp(hat, Prop_Send, "m_iEntityLevel", crandomint(1, 100));
	SetEntProp(hat, Prop_Send, "m_bInitialized", 1);
	if (!DispatchSpawn(hat)) 
	{
		LogError("The created cosmetic entity [Class name: tf_wearable, Item index: %i, Index: %i], failed to spawn! Skipping.", itemindex, hat);
		AcceptEntityInput(hat, "Kill");
		return false;
	}

	SDKCall(g_hWWeaponEquip, client, hat);
	return true;
}

public Action TimerHealth(Handle timer, any client)
{
	int hp = GetPlayerMaxHp(client);
	if (hp > 0)
		SetEntityHealth(client, hp);
	
	return Plugin_Continue;
}

stock void EquipWeaponSlot(const int client, const int slot)
{
	int iWeapon = GetPlayerWeaponSlot(client, slot);
	if (IsValidEntity(iWeapon))
		EquipWeapon(client, iWeapon);
}

stock void EquipWeapon(int client, int weapon)
{
	char class[80];
	GetEntityClassname(weapon, class, sizeof(class));
	Format(class, sizeof(class), "use %s", class);
	FakeClientCommandThrottled(client, class);
}

float g_flNextCommand[MAXPLAYERS + 1];
stock bool FakeClientCommandThrottled(const int client, const char[] command)
{
	if (g_flNextCommand[client] > GetGameTime())
		return false;
	
	FakeClientCommand(client, command);
	g_flNextCommand[client] = GetGameTime() + 0.4;
	return true;
}

stock int GetPlayerMaxHp(const int client)
{
	if (!IsClientInGame(client) || g_iResourceEntity == -1)
		return -1;

	return GetEntProp(g_iResourceEntity, Prop_Send, "m_iMaxHealth", _, client);
}

stock bool IsPlayerHere(const int client)
{
	return (client && IsClientInGame(client) && IsFakeClient(client) && !IsClientReplay(client) && !IsClientSourceTV(client));
}