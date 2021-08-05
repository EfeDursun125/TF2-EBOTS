#include <sourcemod>
#include <sdktools>
#include <sdkhooks>
#include <tf2>
#include <tf2_stocks>
#include <PathFollower>
#pragma newdecls optional

bool AFKMode[MAXPLAYERS + 1];

bool CanMove[MAXPLAYERS + 1];

bool g_bAutoJump[MAXPLAYERS + 1];
bool UseTeleporter[MAXPLAYERS+1];
float RandomJumpTimer[MAXPLAYERS + 1];
float g_flLookPos[MAXPLAYERS + 1][3];

float EBotAimSpeed[MAXPLAYERS + 1];

bool MoveTargetsBehind[MAXPLAYERS + 1];

float PlayerSpawn[MAXPLAYERS + 1][3];

float LastKnownEnemyPosition[MAXPLAYERS + 1][3];

bool IsLastKnownEnemyPositionClear[MAXPLAYERS + 1];

float PathAhead[MAXPLAYERS + 1][3];

float RedTeamSpawn[3];
float BluTeamSpawn[3];

int EBotSenseChance[MAXPLAYERS + 1];

Handle EBotQuota;
Handle EBotAFKMode;
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
Handle EBotNoTFBots;

bool IsAttackDefendMap;

float moveForward(float vel[3],float MaxSpeed)
{
	vel[0] = MaxSpeed;
	return vel;
}

float moveBackwards(float vel[3],float MaxSpeed)
{
	vel[0] = -MaxSpeed;
	return vel;
}

float moveRight(float vel[3],float MaxSpeed)
{
	vel[1] = MaxSpeed;
	return vel;
}

float moveLeft(float vel[3],float MaxSpeed)
{
	vel[1] = -MaxSpeed;
	return vel;
}

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
#include <ebotai/demoman>
#include <ebotai/pages>
#include <ebotai/slenderbase>
#include <ebotai/slenderai>

#define PLUGIN_VERSION  "0.14"

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
	HookEvent("player_hurt", BotHurt, EventHookMode_Post);
	EBotQuota = CreateConVar("ebot_quota", "-1", "");
	EBotAFKMode = CreateConVar("ebot_afk_mode", "1", "Controls the afk players.");
	EBotPerformance = CreateConVar("ebot_performance_mode", "0", "Downgrades the ai for increase the fps!");
	EBotNoArea = CreateConVar("ebot_noarea", "1", "Stops area checking for increase the fps!");
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
	EBotNoTFBots = CreateConVar("ebot_no_tfbots", "1", "");
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
					GetEntPropVector(tmflag, Prop_Send, "m_vecOrigin", g_flRedFlagCapPoint);
				if(iTeamNumObj == 3)
					GetEntPropVector(tmflag, Prop_Send, "m_vecOrigin", g_flBluFlagCapPoint);
			}
		}
	}
	
	ServerCommand("sv_tags ebot");
	AddServerTag("ebot");
	
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
	if(GetConVarInt(EBotNoTFBots) == 1)
	{
		if(FindConVar("tf_bot_quota").IntValue > 0)
		{
			ServerCommand("ebot_quota %i", FindConVar("tf_bot_quota").IntValue);
			ServerCommand("tf_bot_quota 0");
		}
	}
	
	if(GetConVarInt(EBotQuota) < 0)
		return;
	
	if(BotCheckTimer < GetGameTime())
	{
		if(GetTotalPlayersCount() < GetConVarInt(EBotQuota))
			AddEBotConsole();
		else if(GetTotalPlayersCount() > GetConVarInt(EBotQuota))
			KickEBotConsole();
		else if((GetPlayersCountRed() + 1) < GetPlayersCountBlu())
			KickEBotConsole();
		else if((GetPlayersCountBlu() + 1) < GetPlayersCountRed())
			KickEBotConsole();
		
		BotCheckTimer = GetGameTime() + 1.0;
	}
	else if(RandomBotName == -1)
		RandomBotName = GetRandomInt(0, 63);
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
				
				if(IsSlowThink[client])
				{
					if(TryUnStuck[client] && CrouchTime[client] < GetGameTime())
					{
						int nOldButtons = GetEntProp(client, Prop_Data, "m_nOldButtons");
						SetEntProp(client, Prop_Data, "m_nOldButtons", (nOldButtons &= ~(IN_JUMP|IN_DUCK)));
						
						if(GetEntityFlags(client) & FL_ONGROUND && ClientIsMoving(client))
							buttons |= IN_JUMP;
						
						buttons |= IN_DUCK;
					}
					
					if(EBotAimSpeed[client] == -1 || EBotAimSpeed[client] == 0)
						EBotAimSpeed[client] = GetRandomFloat(GetConVarFloat(EBotMinimumAimSpeed), GetConVarFloat(EBotMaximumAimSpeed));
					
					if(EBotSenseChance[client] == -1 || EBotSenseChance[client] == 0)
						EBotSenseChance[client] = GetRandomInt(GetConVarInt(EBotSenseMin), GetConVarInt(EBotSenseMax));
				}
				
				GetClientEyePosition(client, g_flClientEyePos[client]);
				GetClientAbsOrigin(client, g_flClientOrigin[client]);
				
				if(AttackTimer[client] > GetGameTime())
					buttons |= IN_ATTACK;
				
				if(Attack2Timer[client] > GetGameTime())
					buttons |= IN_ATTACK2;
				
				if(ForcePressButton[client] != 0)
				{
					buttons |= ForcePressButton[client];
					
					ForcePressButton[client] = 0;
				}
				
				if(HasEnemiesNear[client] && IsValidClient(NearestEnemy[client]))
				{
					if(TF2_GetPlayerClass(client) == TFClass_Sniper)
					{
						if(IsWeaponSlotActive(client, 0))
						{
							if(PrimaryID[client] == 56 || PrimaryID[client] == 1005 || PrimaryID[client] == 1092 || PrimaryID[client] == 1098)
							{
								if(FireTimer[client] < GetGameTime())
									FireTimer[client] = GetGameTime() + GetRandomFloat(1.3, 2.0);
								else if(FireTimer[client] > GetGameTime())
									buttons |= IN_ATTACK;
							}
							else if(IsSlowThink[client] && TF2_IsPlayerInCondition(client, TFCond_Zoomed))
								buttons |= IN_ATTACK;
							else if(!TF2_IsPlayerInCondition(client, TFCond_Zoomed))
							{
								if(TF2_HasTheFlag(client))
									buttons |= IN_ATTACK;
								else
									buttons |= IN_ATTACK2;
							}
							else if(!IsWeaponSlotActive(client, 0))
								buttons |= IN_ATTACK;
						}
						else
							buttons |= IN_ATTACK;
					}
					else if(TF2_GetPlayerClass(client) == TFClass_Soldier)
					{
						if(IsWeaponSlotActive(client, 0))
						{
							if(PrimaryID[client] == 730)
							{
								int ClipAmmo = GetEntProp(GetPlayerWeaponSlot(client, 0), Prop_Send, "m_iClip1");
								if(ClipAmmo < 3)
									buttons |= IN_ATTACK;
								else
									buttons &= ~IN_ATTACK;
							}
						}
					}
					
					if(MoveTargetsBehind[client])
					{
						float flBotAng[3], flTargetAng[3];
						GetClientEyeAngles(client, flBotAng);
						GetClientEyeAngles(NearestEnemy[client], flTargetAng);
						int iAngleDiff = AngleDifference(flBotAng[1], flTargetAng[1]);
						
						if(GetClientAimTarget(NearestEnemy[client]) == client)
							vel = moveBackwards(vel, 300.0);
						else
							vel = moveForward(vel, 300.0);
						
						if(iAngleDiff > 90)
							vel = moveRight(vel, 300.0);
						else if(iAngleDiff < -90)
							vel = moveLeft(vel, 300.0);
						else
							vel = moveBackwards(vel, 300.0);
					}
				}
				
				if(CrouchTime[client] > GetGameTime())
				{
					if(GetEntityFlags(client) & FL_ONGROUND)
						buttons |= IN_DUCK;
				}
				else
				{
					int nOldButtons = GetEntProp(client, Prop_Data, "m_nOldButtons");
					SetEntProp(client, Prop_Data, "m_nOldButtons", (nOldButtons &= ~(IN_JUMP|IN_DUCK)));
					
					if(g_bAutoJump[client])
					{
						buttons |= IN_JUMP;
						
						g_bAutoJump[client] = false;
					}
					
					if(g_bJump[client])
					{
						if(GetEntityFlags(client) & FL_ONGROUND)
							buttons |= IN_JUMP;
						else
						{
							buttons |= IN_JUMP; // for double jump
							g_bJump[client] = false;
						}
					}
					
					if(GetEntProp(client, Prop_Send, "m_bJumping"))
						buttons |= IN_DUCK;
					
					if(RandomJumpTimer[client] < GetGameTime())
					{
						if(!HasEnemiesNear[client] && StopTime[client] < GetGameTime())
							buttons |= IN_JUMP;
						
						RandomJumpTimer[client] = GetGameTime() + GetRandomFloat(5.0, 15.0);
					}
				}
				
				if(StrContains(currentMap, "slender_" , false) != -1 || StrContains(currentMap, "sf2_" , false) != -1)
				{
					if(GetClientTeam(client) == 2)
					{
						if(CanMove[client])
							TF2_MoveTo(client, g_flGoal[client], vel, angles);
						
						SlenderBaseAI(client);
					}
					
					return Plugin_Continue;
				}
				
				BaseAI(client);
				
				if(TryUnStuck[client])
				{
					vel = moveForward(vel, 400.0);
					
					return Plugin_Continue;
				}
				
				if(CanMove[client])
					TF2_MoveTo(client, g_flGoal[client], vel, angles);
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
	
	SetCommandFlags("bot", ~FCVAR_CHEAT);
	
	if(StrContains(currentMap, "mvm_" , false) != -1)
		ServerCommand("bot -name %s -team red -class random", FakeNames[RandomBotName]);
	else if(StrContains(currentMap, "tfdb_" , false) != -1)
	{
		if(GetTeamsCount(2) > GetTeamsCount(3))
			ServerCommand("bot -name %s -team blue -class pyro", FakeNames[RandomBotName]);
		else
			ServerCommand("bot -name %s -team red -class pyro", FakeNames[RandomBotName]);
	}
	else
	{
		if(GetTeamsCount(2) > GetTeamsCount(3))
			ServerCommand("bot -name %s -team blue -class random", FakeNames[RandomBotName]);
		else
			ServerCommand("bot -name %s -team red -class random", FakeNames[RandomBotName]);
	}
	
	SetCommandFlags("bot", FCVAR_CHEAT);
	
	return Plugin_Handled;
}

AddEBotConsole()
{
	char currentMap[PLATFORM_MAX_PATH];
	GetCurrentMap(currentMap, sizeof(currentMap));
	
	SetCommandFlags("bot", ~FCVAR_CHEAT);
	
	if(StrContains(currentMap, "mvm_" , false) != -1)
		ServerCommand("bot -name %s -team red -class random", FakeNames[RandomBotName]);
	else if(StrContains(currentMap, "tfdb_" , false) != -1)
	{
		if(GetTeamsCount(2) > GetTeamsCount(3))
			ServerCommand("bot -name %s -team blue -class pyro", FakeNames[RandomBotName]);
		else
			ServerCommand("bot -name %s -team red -class pyro", FakeNames[RandomBotName]);
	}
	else
	{
		if(GetTeamsCount(2) > GetTeamsCount(3))
			ServerCommand("bot -name %s -team blue -class random", FakeNames[RandomBotName]);
		else
			ServerCommand("bot -name %s -team red -class random", FakeNames[RandomBotName]);
	}
	
	SetCommandFlags("bot", FCVAR_CHEAT);
}

public Action KickEBot(int client, int args)
{
	int BotTeam;
	
	if(GetTeamClientCount(2) > GetTeamClientCount(3))
		BotTeam = 2;
	else if(GetTeamClientCount(2) < GetTeamClientCount(3))
		BotTeam = 3;
	else
		BotTeam = GetRandomInt(2, 3);
	
	Handle AllBots = CreateArray();
	for(int i = 1; i <= MaxClients; i++)
	{
		if(IsClientConnected(i) && !IsClientSourceTV(i) && !IsClientReplay(i) && IsFakeClient(i) && GetClientTeam(i) == BotTeam)
			PushArrayCell(AllBots, i);
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
	
	SetCommandFlags("bot", ~FCVAR_CHEAT);
	
	ServerCommand("bot_kick \"%N\"", Bot);
	
	SetCommandFlags("bot", FCVAR_CHEAT);
}

KickEBotConsole()
{
	int BotTeam;
	
	if(GetTeamClientCount(2) > GetTeamClientCount(3))
		BotTeam = 2;
	else if(GetTeamClientCount(2) < GetTeamClientCount(3))
		BotTeam = 3;
	else
		BotTeam = GetRandomInt(2, 3);
	
	Handle AllBots = CreateArray();
	for(int i = 1; i <= MaxClients; i++)
	{
		if(IsClientConnected(i) && !IsClientSourceTV(i) && !IsClientReplay(i) && IsFakeClient(i) && GetClientTeam(i) == BotTeam)
			PushArrayCell(AllBots, i);
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
	
	SetCommandFlags("bot", ~FCVAR_CHEAT);
	
	ServerCommand("bot_kick \"%N\"", Bot);
	
	SetCommandFlags("bot", FCVAR_CHEAT);
}

public Action BotSpawn(Handle event, char[] name, bool dontBroadcast)
{
	int client = GetClientOfUserId(GetEventInt(event, "userid"));
	
	StopTime[client] = GetGameTime() - 20.0;
	
	if(IsValidClient(client) && (IsFakeClient(client) || AFKMode[client]))
	{
		HasEnemiesNear[client] = false;
		g_iKothAction[client] = GetRandomInt(1,2);
		HasMonstersNear[client] = false;
		CanAttack[client] = true;
		DefendMode[client] = true;
		Attack2Timer[client] = GetGameTime();
		AttackTimer[client] = GetGameTime();
		PlayerSpawn[client] = GetOrigin(client);
		g_bPickRandomSentrySpot[client] = true;
		HasABuildPosition[client] = false;
		TargetEnemy[client] = -1;
		
		if(WantsBuildSentryGun[client] || !IsValidEntity(SentryGun[client]))
		{
			int hint = -1;
			while((hint = FindEntityByClassname(hint, "bot_hint_sentrygun")) != -1)
			{
				if (client == GetOwnerEntity(hint))
					SetEntPropEnt(hint, Prop_Send, "m_hOwnerEntity", INVALID_ENT_REFERENCE);
			}
		}
		
		if(ChanceOf(GetConVarInt(EBotTeleporter)))
			UseTeleporter[client] = true;
		else
			UseTeleporter[client] = false;
	}
	else if(IsValidClient(client))
	{
		if(GetClientTeam(client) == 2)
			RedTeamSpawn = GetOrigin(client);
		else if(GetClientTeam(client) == 3)
			BluTeamSpawn = GetOrigin(client);
	}
}

public Action BotHurt(Handle event, char[] name, bool dontBroadcast)
{
	int client = GetClientOfUserId(GetEventInt(event, "userid"));
	int target = GetClientOfUserId(GetEventInt(event, "attacker"));
	
	if(IsFakeClient(client) || AFKMode[client])
	{
		if(IsValidClient(client) && IsValidClient(target))
		{
			g_flLookPos[client] = GetEyePosition(target);
			
			g_flLookTimer[client] = GetGameTime() + GetRandomFloat(2.5, 5.0);
		}
	}
	
	if(GetConVarInt(EBotPerformance) != 1 && TF2_GetPlayerClass(client) == TFClass_Spy)
	{
		if(ChanceOf(EBotSenseChance[target]) && (IsFakeClient(target) || AFKMode[target]))
		{
			TargetSpyClient[target] = client;
			
			g_flSpyForgotTime[target] = GetGameTime() + GetRandomFloat(4.0, 12.0);
		}
		else
		{
			for(int search = 1; search <= MaxClients; search++)
			{
				if(IsValidClient(search) && IsClientInGame(search) && IsPlayerAlive(search) && search != client && GetClientTeam(client) != GetClientTeam(search))
				{
					if(ChanceOf(EBotSenseChance[search]) && (IsFakeClient(search) || AFKMode[search]))
					{
						TargetSpyClient[search] = client;
						
						g_flSpyForgotTime[search] = GetGameTime() + GetRandomFloat(2.0, 4.0); // just a warrning about spy.
					}
				}
			}
		}
	}
	else if(!IsFakeClient(client) && TF2_GetPlayerClass(client) == TFClass_Spy) // if performance mode enabled, only search clients for human spy
	{
		if(ChanceOf(EBotSenseChance[target]) && (IsFakeClient(target) || AFKMode[target]))
		{
			TargetSpyClient[target] = client;
			
			g_flSpyForgotTime[target] = GetGameTime() + GetRandomFloat(4.0, 12.0);
		}
		else
		{
			for(int search = 1; search <= MaxClients; search++)
			{
				if(IsValidClient(search) && IsClientInGame(search) && IsPlayerAlive(search) && search != client && GetClientTeam(client) != GetClientTeam(search))
				{
					if(ChanceOf(EBotSenseChance[search]) && (IsFakeClient(search) || AFKMode[search]))
					{
						TargetSpyClient[search] = client;
						
						g_flSpyForgotTime[search] = GetGameTime() + GetRandomFloat(2.0, 4.0); // just a warrning about spy.
					}
				}
			}
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
	float random = GetRandomFloat(0.5, 1.0);
	
	if(g_flFindPathTimer[client] < GetGameTime())
	{
		if (!PF_Exists(client))
		{
			SetEntProp(client, Prop_Data, "m_bLagCompensation", false);
			
			SetEntProp(client, Prop_Data, "m_bPredictWeapons", false);
			
			PF_Create(client, 36.0, 72.0, 1000.0, 0.6, MASK_PLAYERSOLID, 300.0, random, 1.25, 1.25);
			
			PF_EnableCallback(client, PFCB_Approach, Approach);
			
			PF_EnableCallback(client, PFCB_ClimbUpToLedge, PF_ClibmUpToLedge);
			
			return;
		}
		
		TargetGoal[client][0] = flTargetVector[0];
		TargetGoal[client][1] = flTargetVector[1];
		TargetGoal[client][2] = flTargetVector[2];
		
		PF_SetGoalVector(client, flTargetVector);
		
		if(GetVectorDistance(GetOrigin(client), flTargetVector) > 30.0)
		{
			PF_StartPathing(client);
			CanMove[client] = true;
		}
		else
		{
			PF_StopPathing(client);
			CanMove[client] = false;
		}
		
		g_flFindPathTimer[client] = GetGameTime() + random;
	}
}

float AimLagDelay[MAXPLAYERS + 1];

float m_itAimStart[MAXPLAYERS + 1];

stock void TF2_LookAtPos(int client, float flGoal[3], float flAimSpeed = 0.05)
{
	float eye_to_target[3];
	SubtractVectors(flGoal, GetEyePosition(client), eye_to_target);
	
	float eye_ang[3];
	GetClientEyeAngles(client, eye_ang);
	
	NormalizeVector(eye_to_target, eye_to_target);
	
	float ang_to_target[3];
	GetVectorAngles(eye_to_target, ang_to_target);
	
	float eye_vec[3];
	GetAngleVectors(eye_ang, eye_vec, NULL_VECTOR, NULL_VECTOR);
	
	float cos_error = GetVectorDotProduct(eye_to_target, eye_vec);
	
	float max_angvel = 1024.0;
	
	if (cos_error > 0.7)
		max_angvel *= Sine((3.14 / 2.0) * (1.0 + ((-49.0 / 15.0) * (cos_error - 0.7))));
	
	if(m_itAimStart[client] != -1 && (GetGameTime() - m_itAimStart[client] < 0.25))
		max_angvel *= 4.0 * (GetGameTime() - m_itAimStart[client]);
	
	if (cos_error > 0.98 && !HasEnemiesNear[client])
		m_itAimStart[client] = GetGameTime();
	
	float new_eye_angle[3];
	new_eye_angle[0] = ApproachAngle(ang_to_target[0], eye_ang[0], (max_angvel * GetGameFrameTime()) * GetRandomFloat(0.4, 0.6));
	new_eye_angle[1] = ApproachAngle(ang_to_target[1], eye_ang[1], (max_angvel * GetGameFrameTime()));
	new_eye_angle[2] = 0.0;
	
	SubtractVectors(new_eye_angle, NULL_VECTOR, new_eye_angle);
	new_eye_angle[0] = AngleNormalize(new_eye_angle[0]) + EBotAimSpeed[client];
	new_eye_angle[1] = AngleNormalize(new_eye_angle[1]) + EBotAimSpeed[client];
	new_eye_angle[2] = 0.0;
	
	SnapEyeAngles(client, new_eye_angle);
}

stock float Max(float one, float two)
{
	if(one > two)
		return one;
	else if(two > one)
		return two;
		
	return two;
}

stock void TF2_LookAtPos2(int client, float flGoal[3], float flAimSpeed = 0.05)
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
		
		//TeleportEntity(client, NULL_VECTOR, flAng, NULL_VECTOR);
		SnapEyeAngles(client, flAng);
		
		AimLagDelay[client] = GetGameTime() + GetConVarFloat(EBotAimLag);
	}
}

stock float AngleNormalize(float angle)
{
	angle = angle - 360.0 * RoundToFloor(angle / 360.0);
	while (angle > 180.0) angle -= 360.0;
	while (angle < -180.0) angle += 360.0;
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
	
	SetCommandFlags("bot", ~FCVAR_CHEAT);
	
	if(StrContains(currentMap, "mvm_" , false) != -1)
		ServerCommand("bot -name %s -team red -class random", FakeNames[RandomName]);
	else if(StrContains(currentMap, "tfdb_" , false) != -1)
	{
		if(GetTeamsCount(2) > GetTeamsCount(3))
			ServerCommand("bot -name %s -team blue -class pyro", FakeNames[RandomName]);
		else
			ServerCommand("bot -name %s -team red -class pyro", FakeNames[RandomName]);
	}
	else
	{
		if(GetTeamsCount(2) > GetTeamsCount(3))
			ServerCommand("bot -name %s -team blue -class random", FakeNames[RandomName]);
		else
			ServerCommand("bot -name %s -team red -class random", FakeNames[RandomName]);
	}
	
	SetCommandFlags("bot", FCVAR_CHEAT);
}

stock bool NameAlreadyTakenByPlayer(const char[] name)
{
	char buffer[MAX_NAME_LENGTH];
	for(int i = 1; i <= MaxClients; i++)
	{
		if(!IsValidClient(i))
			continue;
		
		GetClientName(i, buffer, sizeof(buffer));
		
		if(StrEqual(name, buffer))
			return true;
	}
	
	return false;
}

public Action ReloadPlugin(Handle timer)
{
	ServerCommand("sm plugins reload tf2_ebot");
}