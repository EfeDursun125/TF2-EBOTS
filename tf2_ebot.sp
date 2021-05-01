#include <sourcemod>
#include <sdktools>
#include <sdkhooks>
#include <tf2>
#include <tf2_stocks>
#include <PathFollower>
#pragma newdecls optional

bool AFKMode[MAXPLAYERS + 1];

bool g_bAutoJump[MAXPLAYERS + 1];
bool g_bPathFinding[MAXPLAYERS + 1];
bool UseTeleporter[MAXPLAYERS+1];
float RandomJumpTimer[MAXPLAYERS + 1];
float g_flLookPos[MAXPLAYERS + 1][3];

float EBotAimSpeed[MAXPLAYERS + 1];

float g_flVelocity0[MAXPLAYERS + 1];
float g_flVelocity1[MAXPLAYERS + 1];

float PlayerSpawn[MAXPLAYERS + 1][3];

float LastKnownEnemyPosition[MAXPLAYERS + 1][3];

bool IsLastKnownEnemyPositionClear[MAXPLAYERS + 1];

float PathAhead[MAXPLAYERS + 1][3];

float RedTeamSpawn[3];
float BluTeamSpawn[3];

int EBotSenseChance[MAXPLAYERS + 1];

Handle EBotQuota;
Handle EBotAFKMode;
Handle EBotDisableCheats;
Handle EBotPerformance;
Handle EBotNoArea;
Handle EBotUseVoiceline;
Handle EBotMinimumAimSpeed;
Handle EBotMaximumAimSpeed;
Handle EBotTeleporter;
Handle EBotDisableAI;
Handle EBotOnlyTeam;
Handle EBotAITeam;
Handle EBotSenseMax;
Handle EBotSenseMin;
Handle EBotTauntChance;
Handle EBotAimLag;

bool IsAttackDefendMap;

#include <ebotai/utilities>
#include <ebotai/target>
#include <ebotai/healthpack>
#include <ebotai/ammopack>
#include <ebotai/aim>
#include <ebotai/voice>
#include <ebotai/koth>
#include <ebotai/cp>
#include <ebotai/ctf>
#include <ebotai/plr>
#include <ebotai/pl>
#include <ebotai/dm>
#include <ebotai/movement>
#include <ebotai/base>
#include <ebotai/check>
#include <ebotai/look>
#include <ebotai/weapons>
#include <ebotai/attack>
#include <ebotai/unstuck>
#include <ebotai/sniper>
#include <ebotai/engineer>
#include <ebotai/spy>
#include <ebotai/pages>
#include <ebotai/slenderbase>
#include <ebotai/slenderai>

#define PLUGIN_VERSION  "0.1"

public Plugin myinfo = 
{
	name = "[TF2] E-Bots",
	author = "EfeDursun125",
	description = "Custom TF2 bots coded with sourcemod.",
	version = PLUGIN_VERSION,
	url = "https://steamcommunity.com/id/EfeDursun91/"
}

Handle BotKick;

public OnPluginStart()
{
	RegConsoleCmd("ebot_addbot", AddEBot);
	RegConsoleCmd("ebot_kickbot", KickEBot);
	RegConsoleCmd("sm_afk", Command_Afk);
	BotKick = CreateGlobalForward("Bot_OnBotKick", ET_Single, Param_CellByRef);
	HookEvent("player_spawn", BotSpawn, EventHookMode_Post);
	EBotQuota = CreateConVar("ebot_quota", "0", "");
	EBotAFKMode = CreateConVar("ebot_afk_mode", "1", "Controls the afk players.");
	EBotPerformance = CreateConVar("ebot_performance_mode", "0", "Downgrades the ai for increase the fps!");
	EBotNoArea = CreateConVar("ebot_noarea", "0", "Stops area checking for increase the fps!");
	EBotDisableCheats = CreateConVar("ebot_disable_sv_cheats", "1", "E-BOTs can't join if sv_cheats 0, disable cheats after bot joined.");
	EBotUseVoiceline = CreateConVar("ebot_use_voice_commands", "1", "");
	EBotMinimumAimSpeed = CreateConVar("ebot_minimum_aim_speed", "0.1", "");
	EBotMaximumAimSpeed = CreateConVar("ebot_maximum_aim_speed", "0.2", "");
	EBotTeleporter = CreateConVar("ebot_use_teleporter_chance", "60", "");
	EBotDisableAI = CreateConVar("ebot_disable_ai_bots", "0", "Disables ai bots, but afk bots still works");
	EBotOnlyTeam = CreateConVar("ebot_ai_only_one_team", "0", "AI for only one team, use ebot_ai_team");
	EBotAITeam = CreateConVar("ebot_ai_team", "3", "AI for only one team");
	EBotSenseMin = CreateConVar("ebot_minimum_sense_chance", "10", "Min 1, Max 100");
	EBotSenseMax = CreateConVar("ebot_maximum_sense_chance", "90", "Min 1, Max 100");
	EBotTauntChance = CreateConVar("ebot_taunt_chance", "25", "Min 1, Max 100");
	EBotAimLag = CreateConVar("ebot_aim_lag", "0.04", "");
	HookEvent("teamplay_round_start", RoundStarted);
}

float BotCheckTimer;

public OnMapStart()
{
	char currentMap[PLATFORM_MAX_PATH];
	GetCurrentMap(currentMap, sizeof(currentMap));
	
	BotCheckTimer = GetGameTime() + 2.0;
	
	if(StrContains(currentMap, "ctf_" , false) != -1)
	{
		int tmflag;
		while((tmflag = FindEntityByClassname(tmflag, "item_teamflag")) != INVALID_ENT_REFERENCE)
		{
			int iTeamNumObj = GetEntProp(tmflag, Prop_Send, "m_iTeamNum");
			if(IsValidEntity(tmflag))
			{
				if(iTeamNumObj == 2)
				{
					GetEntPropVector(tmflag, Prop_Send, "m_vecOrigin", g_flRedFlagCapPoint);
				}
				if(iTeamNumObj == 3)
				{
					GetEntPropVector(tmflag, Prop_Send, "m_vecOrigin", g_flBluFlagCapPoint);
				}
			}
		}
	}
	else if(StrContains(currentMap, "cp_" , false) != -1)
	{
		if(GetNearestBluControlPoint() != -1)
		{
			IsAttackDefendMap = false;
		}
		else
		{
			IsAttackDefendMap = true;
		}
	}
	
	ServerCommand("sv_tags ebot");
	AddServerTag("ebot"); // This is not working, i know but, i'm trying this
	
	InitGamedata();
}

public Action RoundStarted(Handle event , char[] name, bool dontBroadcast)
{
	char currentMap[PLATFORM_MAX_PATH];
	GetCurrentMap(currentMap, sizeof(currentMap));
	
	if(StrContains(currentMap, "ctf_2fort" , false) != -1)
	{
		int snipepos = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos, "teamnum", "2");
		int snipepos2 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos2, "teamnum", "2");
		int snipepos3 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos3, "teamnum", "2");
		
		float origin[3] = {233.0, 1020.0, 294.0};
		float origin2[3] = {-234.0, 1028.0, 294.0};
		float origin3[3] = {23.0, 881.0, 0.0};
		
		TeleportEntity(snipepos, origin, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos2, origin2, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos3, origin3, NULL_VECTOR, NULL_VECTOR);
		
		int snipepos4 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos4, "teamnum", "3");
		int snipepos5 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos5, "teamnum", "3");
		int snipepos6 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos6, "teamnum", "3");
		
		float origin4[3] = {-224.0, -1042.0, 306.0};
		float origin5[3] = {229.0, -1029.0, 305.0};
		float origin6[3] = {-27.0, -876.0, 298.0};
		
		TeleportEntity(snipepos4, origin4, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos5, origin5, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos6, origin6, NULL_VECTOR, NULL_VECTOR);
	}
	if(StrContains(currentMap, "tc_hydro" , false) != -1)
	{
		int snipepos = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos, "teamnum", "0");
		int snipepos2 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos2, "teamnum", "0");
		int snipepos3 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos3, "teamnum", "0");
		int snipepos4 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos4, "teamnum", "0");
		int snipepos5 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos5, "teamnum", "0");
		int snipepos6 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos6, "teamnum", "0");
		
		float origin[3] = {1605.0, 2383.0, 523.0};
		float origin2[3] = {2368.0, 1363.0, 523.0};
		float origin3[3] = {2375.0, 2633.0, 523.0};
		float origin4[3] = {2267.0, -1278.0, 363.0};
		float origin5[3] = {-2136.0, 858.0, 476.0};
		float origin6[3] = {-2133.0, 1427.0, 468.0};
		
		TeleportEntity(snipepos, origin, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos2, origin2, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos3, origin3, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos4, origin4, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos5, origin5, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos6, origin6, NULL_VECTOR, NULL_VECTOR);
	}
	if(StrContains(currentMap, "koth_harvest" , false) != -1)
	{
		int snipepos = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos, "teamnum", "2");
		int snipepos2 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos2, "teamnum", "2");
		int snipepos3 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos3, "teamnum", "2");
		int snipepos10 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos10, "teamnum", "2");
		int snipepos11 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos11, "teamnum", "2");
		int snipepos12 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos12, "teamnum", "2");
		
		float origin[3] = {922.0, -1043.0, 366.0};
		float origin2[3] = {-1424.0, -395.0, 331.0};
		float origin3[3] = {-930.0, -414.0, 331.0};
		float origin10[3] = {-743.0, -1093.0, 448.0};
		float origin11[3] = {-242.0, -1090.0, 45.0};
		float origin12[3] = {1262.0, 387.0, 300.0};
		
		TeleportEntity(snipepos, origin, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos2, origin2, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos3, origin3, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos10, origin10, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos11, origin11, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos12, origin12, NULL_VECTOR, NULL_VECTOR);
		
		int snipepos4 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos4, "teamnum", "3");
		int snipepos5 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos5, "teamnum", "3");
		int snipepos6 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos6, "teamnum", "3");
		int snipepos7 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos7, "teamnum", "3");
		int snipepos8 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos8, "teamnum", "3");
		int snipepos9 = CreateEntityByName("func_tfbot_hint");
		DispatchKeyValue(snipepos9, "teamnum", "3");
		
		float origin4[3] = {-892.0, 1032.0, 367.0};
		float origin5[3] = {1344.0, 296.0, 331.0};
		float origin6[3] = {198.0, 1085.0, 454.0};
		float origin7[3] = {744.0, 1093.0, 449.0};
		float origin8[3] = {892.0, 397.0, 331.0};
		float origin9[3] = {-1295.0, -392.0, 300.0};
		
		TeleportEntity(snipepos4, origin4, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos5, origin5, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos6, origin6, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos7, origin7, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos8, origin8, NULL_VECTOR, NULL_VECTOR);
		TeleportEntity(snipepos9, origin9, NULL_VECTOR, NULL_VECTOR);
	}
}

public Action Command_Afk(int client, int args)
{
	if(GetConVarInt(EBotAFKMode) != 1)
		return Plugin_Handled;
	
	if(args != 0 && args != 2)
	{
		ReplyToCommand(client, "[E-BOT] Usage: sm_afk <target> [0/1]");
		
		return Plugin_Handled;
	}
	
	if(args == 0) // For People
	{
		if(!AFKMode[client])
		{
			PrintToChat(client, "[E-BOT] AFK-BOT is enabled.");
			AFKMode[client] = true;
		}
		else
		{
			PrintToChat(client, "[E-BOT] AFK-BOT is disabled.");
			PrintCenterText(client, "Your AFK Mode is now disabled.");
			AFKMode[client] = false;
		}
		
		return Plugin_Handled;
	}
	
	else if(args == 2)
	{
		char arg1[PLATFORM_MAX_PATH];
		GetCmdArg(1, arg1, sizeof(arg1));
		char arg2[8];
		GetCmdArg(2, arg2, sizeof(arg2));
		
		int value = StringToInt(arg2);
		if(value != 0 && value != 1)
		{
			ReplyToCommand(client, "[E-BOT] Usage: sm_afk <target> [0/1]");
			return Plugin_Handled;
		}
		
		char target_name[MAX_TARGET_LENGTH];
		int target_list[MAXPLAYERS];
		int target_count;
		bool tn_is_ml;
		if((target_count = ProcessTargetString(arg1, client, target_list, MAXPLAYERS, 0, target_name, sizeof(target_name), tn_is_ml)) <= 0)
		{
			ReplyToTargetError(client, target_count);
			return Plugin_Handled;
		}
		
		for(int i=0; i<target_count; i++) if(IsValidClient(target_list[i]))
		{
			if(value == 0)
			{
				if(CheckCommandAccess(client, "sm_afk_access", ADMFLAG_ROOT))
				{
					PrintToChat(target_list[i], "[E-BOT] AFK-BOT is disabled.");
					PrintCenterText(target_list[i], "Your AFK Mode is now disabled.");
					AFKMode[target_list[i]] = false;
				}
			}
			else
			{
				if(CheckCommandAccess(client, "sm_afk_access", ADMFLAG_ROOT))
				{
					PrintToChat(target_list[i], "[E-BOT] AFK-BOT is enabled.");
					AFKMode[target_list[i]] = true;
				}
			}
		}
	}

	return Plugin_Handled;
}

static char FakeNames[64][] = {"System32", "Tom", "RogerThat", "Spybro", "botman", "YourGPUdeviceHasLost", "TeamForest", "Squ33z3", "quality", "bailoutnow", "ph34r", "1337sp34k", "soupMAN", "teke", "EngineerGaming", "Spyper", "benthas", "hmmmm", "LOLLOL", "pjkh", "sny", "demopan", "FREEDOOM!", "freeman", "alyx", "killer", "g-man", "xcellent", "nopera", "Moridin", "RankedTF2", "YoungGirl", "System64", "Creeper", "TheSameKiller", "F2PisAMAZING", "thinking?", "TheBest", "unicorn", "Pootis", "GoodFight", "bronze", "comeTOhere", "dead", "XxxBEASTxxX", "nope", "archive", "coach", "Steve", "notch", "forge", "herobrine", "liteXspy", "WEAREGREAT", "RickMay", "sniper", "AllyPlayer", "endernight", "fabric", "encore", "pocketPyro", "Pybro", "Kaboom", "PyroMaster"};

int RandomBotName;

public void OnGameFrame()
{
	if(BotCheckTimer < GetGameTime())
	{
		if(GetTotalPlayersCount() < GetConVarInt(EBotQuota))
		{
			AddEBotConsole();
		}
		else if(GetTotalPlayersCount() > GetConVarInt(EBotQuota))
		{
			KickEBotConsole();
		}
		else if((GetPlayersCountRed() + 1) < GetPlayersCountBlu())
		{
			KickEBotConsole();
		}
		else if((GetPlayersCountBlu() + 1) < GetPlayersCountRed())
		{
			KickEBotConsole();
		}
		else
		{
			if(GetConVarInt(EBotDisableCheats) == 1)
			{
				CreateTimer(0.1, ResetCheats);
			}
		}
		
		BotCheckTimer = GetGameTime() + 1.0;
	}
	else if(RandomBotName == -1)
	{
		RandomBotName = GetRandomInt(0, 63);
	}
	else
	{
		do
		{
			RandomBotName = GetRandomInt(0, 63);
		}
		while(NameAlreadyTakenByPlayer(FakeNames[RandomBotName]));
	}
}

public Action OnPlayerRunCmd(int client, int& buttons, int& impulse, float vel[3], float angles[3])
{
	if(IsValidClient(client))
	{
		if((GetConVarInt(EBotDisableAI) != 1 && IsFakeClient(client)) || AFKMode[client])
		{
			if(IsPlayerAlive(client) && (GetConVarInt(EBotOnlyTeam) != 1 || GetClientTeam(client) == GetConVarInt(EBotAITeam)))
			{
				char currentMap[PLATFORM_MAX_PATH];
				GetCurrentMap(currentMap, sizeof(currentMap));
				
				if(g_flVelocity0[client] > 1)
				{
					vel[0] += g_flVelocity0[client];
				}
				
				if(g_flVelocity1[client] > 1)
				{
					vel[1] += g_flVelocity1[client];
				}
				
				if(IsSlowThink[client])
				{
					g_bPathFinding[client] = true;
					g_bSpyAlert[client] = true;
					g_bISeeSpy[client] = true;
					
					int nOldButtons = GetEntProp(client, Prop_Data, "m_nOldButtons");
					SetEntProp(client, Prop_Data, "m_nOldButtons", (nOldButtons &= ~(IN_JUMP|IN_DUCK)));
					
					if(TryUnStuck[client])
					{
						if(GetEntityFlags(client) & FL_ONGROUND)
						{
							buttons |= IN_JUMP;
						}
						
						buttons |= IN_DUCK;
					}
					
					if(EBotAimSpeed[client] == -1 || EBotAimSpeed[client] == 0)
					{
						EBotAimSpeed[client] = GetRandomFloat(GetConVarFloat(EBotMinimumAimSpeed), GetConVarFloat(EBotMaximumAimSpeed));
					}
					
					if(EBotSenseChance[client] == -1 || EBotSenseChance[client] == 0)
					{
						EBotSenseChance[client] = GetRandomInt(GetConVarInt(EBotSenseMin), GetConVarInt(EBotSenseMax));
					}
				}
				else
				{
					GetClientEyePosition(client, g_flClientEyePos[client]);
					GetClientAbsOrigin(client, g_flClientOrigin[client]);
					
					if(AttackTimer[client] > GetGameTime())
					{
						buttons |= IN_ATTACK;
					}
					
					if(Attack2Timer[client] > GetGameTime())
					{
						buttons |= IN_ATTACK2;
					}
				}
				
				if(ForcePressButton[client] != 0)
				{
					buttons |= ForcePressButton[client];
					
					ForcePressButton[client] = 0;
				}
				
				if(HasEnemiesNear[client])
				{
					if(TF2_GetPlayerClass(client) == TFClass_Sniper)
					{
						if(IsWeaponSlotActive(client, 0))
						{
							if(PrimaryID[client] == 56 || PrimaryID[client] == 1005 || PrimaryID[client] == 1092 || PrimaryID[client] == 1098)
							{
								if(FireTimer[client] < GetGameTime())
								{
									FireTimer[client] = GetGameTime() + GetRandomFloat(1.3, 2.0);
								}
								else if(FireTimer[client] > GetGameTime())
								{
									buttons |= IN_ATTACK;
								}
							}
							else if(IsSlowThink[client] && TF2_IsPlayerInCondition(client, TFCond_Zoomed))
							{
								buttons |= IN_ATTACK;
							}
							else if(!TF2_IsPlayerInCondition(client, TFCond_Zoomed))
							{
								if(TF2_HasTheFlag(client))
								{
									buttons |= IN_ATTACK;
								}
								else
								{
									buttons |= IN_ATTACK2;
								}
							}
							else if(!IsWeaponSlotActive(client, 0))
							{
								buttons |= IN_ATTACK;
							}
						}
						else
						{
							buttons |= IN_ATTACK;
						}
					}
					else if(TF2_GetPlayerClass(client) == TFClass_Soldier)
					{
						if(IsWeaponSlotActive(client, 0))
						{
							if(PrimaryID[client] == 730)
							{
								int ClipAmmo = GetEntProp(GetPlayerWeaponSlot(client, 0), Prop_Send, "m_iClip1");
								if(ClipAmmo < 3)
								{
									buttons |= IN_ATTACK;
								}
								else
								{
									buttons &= ~IN_ATTACK;
								}
							}
						}
					}
				}
				
				if(g_bAutoJump[client])
				{
					buttons |= IN_JUMP;
					
					g_bAutoJump[client] = false;
				}
				
				if(g_bJump[client])
				{
					if(GetEntityFlags(client) & FL_ONGROUND)
					{
						buttons |= IN_JUMP;
					}
					else
					{
						buttons |= IN_JUMP; // for double jump
						g_bJump[client] = false;
					}
				}
				
				if(GetEntProp(client, Prop_Send, "m_bJumping"))
				{
					buttons |= IN_DUCK;
				}
				
				if(g_bCrouch[client])
				{
					if(GetEntityFlags(client) & FL_ONGROUND)
					{
						buttons |= IN_DUCK;
					}
				}
				
				if(RandomJumpTimer[client] < GetGameTime())
				{
					if(!HasEnemiesNear[client] && ClientIsMoving(client) && StopTime[client] < GetGameTime())
					{
						buttons |= IN_JUMP;
					}
					
					RandomJumpTimer[client] = GetGameTime() + GetRandomFloat(5.0, 15.0);
				}
				
				if(StrContains(currentMap, "slender_" , false) != -1 || StrContains(currentMap, "sf2_" , false) != -1)
				{
					if(GetClientTeam(client) == 2)
					{
						if(PF_Exists(client))
						{
							TF2_MoveTo(client, g_flGoal[client], vel, angles);
						}
						
						SlenderBaseAI(client);
					}
					
					return Plugin_Continue;
				}
				
				if(GetVectorDistance(GetOrigin(client), TargetGoal[client]) > 23.0)
				{
					if(PF_Exists(client))
					{
						TF2_MoveTo(client, g_flGoal[client], vel, angles);
					}
				}
				
				BaseAI(client);
			}
			else
			{
				StopTime[client] = GetGameTime() - 2.0;
				Attack2Timer[client] = GetGameTime();
			}
		}
		else if(!AFKMode[client] && !IsFakeClient(client))
		{
			SetEntProp(client, Prop_Data, "m_bLagCompensation", true);
			SetEntProp(client, Prop_Data, "m_bPredictWeapons", true);
		}
	}
	
	return Plugin_Continue;
}

public Action AddEBot(int client, int args)
{
	char currentMap[PLATFORM_MAX_PATH];
	GetCurrentMap(currentMap, sizeof(currentMap));
	
	SetConVarFlags(FindConVar("sv_cheats"), FCVAR_NONE);
	SetConVarInt(FindConVar("sv_cheats"), 1);
	if(StrContains(currentMap, "mvm_" , false) != -1)
	{
		ServerCommand("bot -name %s -team red -class random", FakeNames[RandomBotName]);
	}
	else if(StrContains(currentMap, "tfdb_" , false) != -1)
	{
		if(GetTeamsCount(2) > GetTeamsCount(3))
		{
			ServerCommand("bot -name %s -team blue -class pyro", FakeNames[RandomBotName]);
		}
		else
		{
			ServerCommand("bot -name %s -team red -class pyro", FakeNames[RandomBotName]);
		}
	}
	else
	{
		if(GetTeamsCount(2) > GetTeamsCount(3))
		{
			ServerCommand("bot -name %s -team blue -class random", FakeNames[RandomBotName]);
		}
		else
		{
			ServerCommand("bot -name %s -team red -class random", FakeNames[RandomBotName]);
		}
	}
	
	if(GetConVarInt(EBotDisableCheats) == 1)
	{
		CreateTimer(0.1, ResetCheats);
	}
	
	return Plugin_Handled;
}

AddEBotConsole()
{
	char currentMap[PLATFORM_MAX_PATH];
	GetCurrentMap(currentMap, sizeof(currentMap));
	
	SetConVarFlags(FindConVar("sv_cheats"), FCVAR_NONE);
	SetConVarInt(FindConVar("sv_cheats"), 1);
	
	if(StrContains(currentMap, "mvm_" , false) != -1)
	{
		ServerCommand("bot -name %s -team red -class random", FakeNames[RandomBotName]);
	}
	else if(StrContains(currentMap, "tfdb_" , false) != -1)
	{
		if(GetTeamsCount(2) > GetTeamsCount(3))
		{
			ServerCommand("bot -name %s -team blue -class pyro", FakeNames[RandomBotName]);
		}
		else
		{
			ServerCommand("bot -name %s -team red -class pyro", FakeNames[RandomBotName]);
		}
	}
	else
	{
		if(GetTeamsCount(2) > GetTeamsCount(3))
		{
			ServerCommand("bot -name %s -team blue -class random", FakeNames[RandomBotName]);
		}
		else
		{
			ServerCommand("bot -name %s -team red -class random", FakeNames[RandomBotName]);
		}
	}
}

public Action KickEBot(int client, int args)
{
	int BotTeam;
	
	if(GetTeamClientCount(2) > GetTeamClientCount(3))
	{
		BotTeam = 2;
	} 
	else if(GetTeamClientCount(2) < GetTeamClientCount(3))
	{
		BotTeam = 3;
	}
	else
	{
		BotTeam = GetRandomInt(2, 3);
	}
	
	Handle AllBots = CreateArray();
	for(int i = 1; i <= MaxClients; i++)
	{
		if(IsClientConnected(i) && !IsClientSourceTV(i) && !IsClientReplay(i) && IsFakeClient(i) && GetClientTeam(i) == BotTeam)
		{
			PushArrayCell(AllBots, i);
		}
	}
	
	if(GetArraySize(AllBots) == 0)
	{
		CloseHandle(AllBots);
		return;
	}
	
	int Bot = GetArrayCell(AllBots, GetRandomInt(0, GetArraySize(AllBots) - 1));
	CloseHandle(AllBots);
	
	Call_StartForward(BotKick);
	Call_PushCellRef(Bot);
	Call_Finish();
	
	SetConVarFlags(FindConVar("sv_cheats"), FCVAR_NONE);
	SetConVarInt(FindConVar("sv_cheats"), 1);
	
	ServerCommand("bot_kick \"%N\"", Bot);
	
	if(GetConVarInt(EBotDisableCheats) == 1)
	{
		CreateTimer(0.1, ResetCheats);
	}
}

KickEBotConsole()
{
	int BotTeam;
	
	if(GetTeamClientCount(2) > GetTeamClientCount(3))
	{
		BotTeam = 2;
	} 
	else if(GetTeamClientCount(2) < GetTeamClientCount(3))
	{
		BotTeam = 3;
	}
	else
	{
		BotTeam = GetRandomInt(2, 3);
	}
	
	Handle AllBots = CreateArray();
	for(int i = 1; i <= MaxClients; i++)
	{
		if(IsClientConnected(i) && !IsClientSourceTV(i) && !IsClientReplay(i) && IsFakeClient(i) && GetClientTeam(i) == BotTeam)
		{
			PushArrayCell(AllBots, i);
		}
	}
	
	if(GetArraySize(AllBots) == 0)
	{
		CloseHandle(AllBots);
		return;
	}
	
	int Bot = GetArrayCell(AllBots, GetRandomInt(0, GetArraySize(AllBots) - 1));
	CloseHandle(AllBots);
	
	Call_StartForward(BotKick);
	Call_PushCellRef(Bot);
	Call_Finish();
	
	SetConVarFlags(FindConVar("sv_cheats"), FCVAR_NONE);
	SetConVarInt(FindConVar("sv_cheats"), 1);
	
	ServerCommand("bot_kick \"%N\"", Bot);
}

public Action BotSpawn(Handle event, char[] name, bool dontBroadcast)
{
	int client = GetClientOfUserId(GetEventInt(event, "userid"));
	
	StopTime[client] = GetGameTime() - 20.0;
	
	if(IsValidClient(client) && (IsFakeClient(client) || AFKMode[client]))
	{
		HasEnemiesNear[client] = false;
		g_iKothAction[client] = GetRandomInt(1,2);
		PLRAction[client] = GetRandomInt(1,2);
		HasMonstersNear[client] = false;
		CanAttack[client] = true;
		DefendFlag[client] = true;
		Attack2Timer[client] = GetGameTime();
		AttackTimer[client] = GetGameTime();
		PlayerSpawn[client] = GetOrigin(client);
		g_bPickRandomSentrySpot[client] = true;
		HasABuildPosition[client] = false;
		TargetEnemy[client] = -1;
		
		if(ChanceOf(GetConVarInt(EBotTeleporter)))
		{
			UseTeleporter[client] = true;
		}
		else
		{
			UseTeleporter[client] = false;
		}
	}
	else if(IsValidClient(client))
	{
		if(GetClientTeam(client) == 2)
		{
			RedTeamSpawn = GetOrigin(client);
		}
		else if(GetClientTeam(client) == 3)
		{
			BluTeamSpawn = GetOrigin(client);
		}
	}
}

stock int GetHealth(int client)
{
	return GetEntProp(client, Prop_Send, "m_iHealth");
}

float g_flFindPathTimer[MAXPLAYERS+1];
stock void TF2_FindPath(int client, float flTargetVector[3])
{
	if(g_flFindPathTimer[client] < GetGameTime())
	{
		if (!PF_Exists(client))
		{
			SetEntProp(client, Prop_Data, "m_bLagCompensation", false);
			
			SetEntProp(client, Prop_Data, "m_bPredictWeapons", false);
			
			PF_Create(client, 24.0, 64.0, 1000.0, 0.6, MASK_PLAYERSOLID, 300.0, 1.0, 1.25, 1.25);
			
			PF_EnableCallback(client, PFCB_Approach, Approach);
			
			PF_EnableCallback(client, PFCB_ClimbUpToLedge, PF_ClibmUpToLedge);
			
			PF_EnableCallback(client, PFCB_IsEntityTraversable, IsEntityTraversable);
			
			PF_EnableCallback(client, PFCB_GetPathCost, PathCost);
			
			return;
		}
		
		if(!PF_IsPathToVectorPossible(client, flTargetVector))
			return;
		
		PF_SetGoalVector(client, flTargetVector);
		
		TargetGoal[client][0] = flTargetVector[0];
		TargetGoal[client][1] = flTargetVector[1];
		TargetGoal[client][2] = flTargetVector[2];
		
		if(GetVectorDistance(GetOrigin(client), flTargetVector) > 25.0)
		{
			PF_StartPathing(client);
		}
		else
		{
			PF_StopPathing(client);
		}
		
		g_flFindPathTimer[client] = GetGameTime() + 1.0;
	}
}

float AimLagDelay[MAXPLAYERS + 1];

stock void TF2_LookAtPos(int client, float flGoal[3], float flAimSpeed = 0.05)
{
	if(AimLagDelay[client] < GetGameTime())
	{
		float flPos[3];
		GetClientEyePosition(client, flPos);
		
		float flAng[3];
		GetClientEyeAngles(client, flAng);
		
		// get normalised direction from target to client
		float desired_dir[3];
		MakeVectorFromPoints(flPos, flGoal, desired_dir);
		GetVectorAngles(desired_dir, desired_dir);
		
		// ease the current direction to the target direction
		flAng[0] += AngleNormalize(desired_dir[0] - flAng[0]) * (flAimSpeed + (GetConVarFloat(EBotAimLag) * 6));
		flAng[1] += AngleNormalize(desired_dir[1] - flAng[1]) * (flAimSpeed + (GetConVarFloat(EBotAimLag) * 6));
		flAng[2] += AngleNormalize(desired_dir[2] - flAng[2]) * (flAimSpeed + (GetConVarFloat(EBotAimLag) * 6));
		
		//TeleportEntity(client, NULL_VECTOR, flAng, NULL_VECTOR);
		SnapEyeAngles(client, flAng);
		
		AimLagDelay[client] = GetGameTime() + GetConVarFloat(EBotAimLag);
	}
}

stock float AngleNormalize(float angle)
{
	angle = fmodf(angle, 360.0);
	if (angle > 180) 
	{
		angle -= 360;
	}
	if (angle < -180)
	{
		angle += 360;
	}
	
	return angle;
}

stock float fmodf(float number, float denom)
{
	return number - RoundToFloor(number / denom) * denom;
}

stock bool ClientViews(Viewer, Target, float fMaxDistance=0.0, float fThreshold=0.70) // By Damizean
{
    // Retrieve view and target eyes position
    float fViewPos[3];   GetClientEyePosition(Viewer, fViewPos);
    float fViewAng[3];   GetClientEyeAngles(Viewer, fViewAng);
    float fViewDir[3];
    float fTargetPos[3]; GetClientEyePosition(Target, fTargetPos);
    float fTargetDir[3];
    float fDistance[3];
	
    // Calculate view direction
    fViewAng[0] = fViewAng[2] = 0.0;
    GetAngleVectors(fViewAng, fViewDir, NULL_VECTOR, NULL_VECTOR);
    
    // Calculate distance to viewer to see if it can be seen.
    fDistance[0] = fTargetPos[0]-fViewPos[0];
    fDistance[1] = fTargetPos[1]-fViewPos[1];
    fDistance[2] = 0.0;
    if (fMaxDistance != 0.0)
    {
        if (((fDistance[0]*fDistance[0])+(fDistance[1]*fDistance[1])) >= (fMaxDistance*fMaxDistance))
            return false;
    }
    
    // Check dot product. If it's negative, that means the viewer is facing
    // backwards to the target.
    NormalizeVector(fDistance, fTargetDir);
    if (GetVectorDotProduct(fViewDir, fTargetDir) < fThreshold) return false;
    
    return true;
}

stock bool ClientViewsOrigin(Viewer, float fTargetPos[3], float fMaxDistance=0.0, float fThreshold=0.70) // Stock Link : https://forums.alliedmods.net/showpost.php?p=973411&postcount=4 | By Damizean
{
    // Retrieve view and target eyes position
    float fViewPos[3];   GetClientEyePosition(Viewer, fViewPos);
    float fViewAng[3];   GetClientEyeAngles(Viewer, fViewAng);
    float fViewDir[3];
    float fTargetDir[3];
    float fDistance[3];
	
    // Calculate view direction
    fViewAng[0] = fViewAng[2] = 0.0;
    GetAngleVectors(fViewAng, fViewDir, NULL_VECTOR, NULL_VECTOR);
    
    // Calculate distance to viewer to see if it can be seen.
    fDistance[0] = fTargetPos[0]-fViewPos[0];
    fDistance[1] = fTargetPos[1]-fViewPos[1];
    fDistance[2] = 0.0;
    if (fMaxDistance != 0.0)
    {
        if (((fDistance[0]*fDistance[0])+(fDistance[1]*fDistance[1])) >= (fMaxDistance*fMaxDistance))
            return false;
    }
    
    // Check dot product. If it's negative, that means the viewer is facing
    // backwards to the target.
    NormalizeVector(fDistance, fTargetDir);
    if (GetVectorDotProduct(fViewDir, fTargetDir) < fThreshold) return false;
    
    return true;
}

public bool ClientViewsFilter(Entity, Mask, any:Junk)
{
    if (Entity >= 1 && Entity <= MaxClients) return false;
    return true;
}

stock void EquipWeaponSlot(int client, int slot)
{
	int iWeapon = GetPlayerWeaponSlot(client, slot);
	if(IsValidEntity(iWeapon))
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
stock bool FakeClientCommandThrottled(int client, const char[] command)
{
	if(g_flNextCommand[client] > GetGameTime())
		return false;
	
	FakeClientCommand(client, command);
	
	g_flNextCommand[client] = GetGameTime() + 0.4;
	
	return true;
}

public bool TraceEntityFilterPlayer(int entity, int contentsMask) 
{
    return entity > MaxClients;
}

stock bool IsWeaponSlotActive(int client, int slot)
{
    return GetPlayerWeaponSlot(client, slot) == GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
}

stock bool IsPointVisible(float start[3], float end[3])
{
	Handle hTrace = TR_TraceRayFilterEx(start, end, MASK_SHOT|CONTENTS_GRATE, RayType_EndPoint, ClientViewsFilter);
	if(TR_DidHit(hTrace))
	{
		CloseHandle(hTrace);
		
		return false;
	}
	
	CloseHandle(hTrace);
	
	return true;
}

stock bool IsPointVisible2(float start[3], float end[3])
{
	TR_TraceRayFilter(start, end, MASK_SHOT|CONTENTS_GRATE, RayType_EndPoint, TraceEntityFilterStuff);
	return TR_GetFraction() >= 0.9;
}

stock bool IsPointVisibleTank(float start[3], float end[3])
{
	TR_TraceRayFilter(start, end, MASK_SHOT|CONTENTS_GRATE, RayType_EndPoint, TraceEntityFilterStuffTank);
	return TR_GetFraction() >= 0.9;
}

public bool TraceEntityFilterStuffTank(int entity, int mask)
{
	new maxentities = GetMaxEntities();
	return entity > maxentities;
}

public bool TraceEntityFilterStuff(int entity, int mask)
{
	return entity > MaxClients;
}

public bool PF_ClibmUpToLedge(int bot_entidx, const float dst[3], const float dir[3])
{
	g_bAutoJump[bot_entidx] = true;
}

public void Approach(int bot_entidx, const float dst[3])
{
	float AimPosition[3];
	float AimPositionNavmesh[3];
	
	g_flGoal[bot_entidx] = dst;
	
	char currentMap[PLATFORM_MAX_PATH];
	GetCurrentMap(currentMap, sizeof(currentMap));
	
	if(!HasEnemiesNear[bot_entidx] && TF2_HasTheFlag(bot_entidx) && PF_GetFutureSegment(bot_entidx, 1, AimPosition))
	{
		NavArea area = TheNavMesh.GetNearestNavArea_Vec(AimPosition, true, 5000.0, false, false, GetClientTeam(bot_entidx));
		if(area != NavArea_Null)
		{
			area.GetCenter(AimPositionNavmesh);
		}
		
		AimPositionNavmesh[2] += (g_flClientEyePos[bot_entidx][2] - g_flClientOrigin[bot_entidx][2]);
		
		TF2_LookAtPos(bot_entidx, AimPositionNavmesh, EBotAimSpeed[bot_entidx]);
	}
	else if((StrContains(currentMap, "slender_" , false) != -1 || StrContains(currentMap, "sf2_" , false) != -1) && PF_GetFutureSegment(bot_entidx, 1, AimPosition) && EnableLook[bot_entidx])
	{
		float BestAimPosition[3];
		
		BestAimPosition[0] = AimPosition[0];
		BestAimPosition[1] = AimPosition[1];
		BestAimPosition[2] += (g_flClientEyePos[bot_entidx][2] - g_flClientOrigin[bot_entidx][2]);
		
		TF2_LookAtPos(bot_entidx, BestAimPosition, EBotAimSpeed[bot_entidx]);
	}
	else if(PF_GetFutureSegment(bot_entidx, 2, AimPosition))
	{
		PathAhead[bot_entidx][0] = AimPosition[0];
		PathAhead[bot_entidx][1] = AimPosition[1];
		PathAhead[bot_entidx][2] = g_flClientEyePos[bot_entidx][2];
	}
}

public AutoAddBot()
{
	char currentMap[PLATFORM_MAX_PATH];
	GetCurrentMap(currentMap, sizeof(currentMap));
	new RandomName = GetRandomInt(0, 63);
	SetConVarFlags(FindConVar("sv_cheats"), FCVAR_NONE);
	SetConVarInt(FindConVar("sv_cheats"), 1);
	if(StrContains(currentMap, "mvm_" , false) != -1)
	{
		ServerCommand("bot -name %s -team red -class random", FakeNames[RandomName]);
	}
	else if(StrContains(currentMap, "tfdb_" , false) != -1)
	{
		if(GetTeamsCount(2) > GetTeamsCount(3))
		{
			ServerCommand("bot -name %s -team blue -class pyro", FakeNames[RandomName]);
		}
		else
		{
			ServerCommand("bot -name %s -team red -class pyro", FakeNames[RandomName]);
		}
	}
	else
	{
		if(GetTeamsCount(2) > GetTeamsCount(3))
		{
			ServerCommand("bot -name %s -team blue -class random", FakeNames[RandomName]);
		}
		else
		{
			ServerCommand("bot -name %s -team red -class random", FakeNames[RandomName]);
		}
	}
	
	CreateTimer(0.1, ResetCheats);
}

public bool IsEntityTraversable(int bot_entidx, int other_entidx, TraverseWhenType when)
{
	char ClassName[32];
	GetEdictClassname(other_entidx, ClassName, 32);
	
	if(HasEntProp(other_entidx, Prop_Send, "m_hBuilder") && GetEntPropEnt(other_entidx, Prop_Send, "m_hBuilder") == bot_entidx)
	{
		return false;
	}
	
	if(StrContains(ClassName, "monster_generic", false) != -1 || StrContains(ClassName, "monster", false) != -1 || StrContains(ClassName, "npc", false) != -1)
	{
		return false;
	}
	
	if(IsWeaponSlotActive(bot_entidx, 2) && IsValidClient(other_entidx) && GetTeamNumber(bot_entidx) == GetTeamNumber(other_entidx))
	{
		return false;
	}
	
	if(IsValidClient(other_entidx) && GetTeamNumber(bot_entidx) == GetTeamNumber(other_entidx))
	{
		return true;
	}
	
	if(IsValidClient(other_entidx) && GetTeamNumber(bot_entidx) != GetTeamNumber(other_entidx))
	{
		return false;
	}
	
	return true;
}

public float PathCost(int bot_entidx, NavArea area, NavArea from_area, float length)
{
	float multiplier = 1.0;
	float dist;
	
	if(TF2_GetPlayerClass(bot_entidx) == TFClass_Spy || TF2_GetPlayerClass(bot_entidx) == TFClass_Sniper || TF2_GetPlayerClass(bot_entidx) == TFClass_Medic || TF2_GetPlayerClass(bot_entidx) == TFClass_Engineer || HidingSpotIsReady[bot_entidx] || g_bHealthIsLow[bot_entidx] || g_bAmmoIsLow[bot_entidx]) 
	{
		float Center[3];
		area.GetCenter(Center);
		
		if(IsPointVisible(LastKnownEnemyPosition[bot_entidx], Center))
		{
			dist *= 5.0;
		}
		
		if((TF2_GetClientTeam(bot_entidx) == TFTeam_Red  && HasTFAttributes(area, BLUE_SENTRY)) || (TF2_GetClientTeam(bot_entidx) == TFTeam_Blue && HasTFAttributes(area, RED_SENTRY))) 
		{
			dist *= 5.0;
		}
		
		dist *= area.GetPlayerCount(GetEnemyTeam(bot_entidx));
	}
	else
	{
		if (length > 0.0) 
		{
			dist = length;
		} 
		else 
		{
			float center[3];          area.GetCenter(center);
			float fromCenter[3]; from_area.GetCenter(fromCenter);
		
			float subtracted[3]; SubtractVectors(center, fromCenter, subtracted);
		
			dist = GetVectorLength(subtracted);
		}
		
		if(area.ComputeAdjacentConnectionHeightChange(from_area) >= 18.0)
		{
			dist *= 2.0;
		}
		
		if (!GetEntProp(bot_entidx, Prop_Send, "m_bIsMiniBoss")) 
		{
			int seed = RoundToFloor(GetGameTime() * 0.1) + 1;
			seed *= area.GetID();
			seed *= bot_entidx;
			
			multiplier += (Cosine(float(seed)) + 1.0) * 10.0;
		}
	}
	
	float cost = dist * multiplier;
	
	return from_area.GetCostSoFar() + cost;
}

bool NameAlreadyTakenByPlayer(const char[] name)
{
	char buffer[MAX_NAME_LENGTH];
	for(int i = 1; i <= MaxClients; i++)
	{
		if(!IsValidClient(i))
		{
			continue;
		}
		GetClientName(i, buffer, sizeof(buffer));
		if(StrEqual(name, buffer))
		{
			return true;
		}
	}
	
	return false;
}

public Action ResetCheats(Handle timer)
{
	SetConVarInt(FindConVar("sv_cheats"), 0);
	SetConVarFlags(FindConVar("sv_cheats"), FCVAR_NOTIFY|FCVAR_REPLICATED);
}