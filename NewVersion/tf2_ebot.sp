#include <sourcemod>
#include <sdktools>
#include <sdkhooks>
#include <tf2>
#include <tf2_stocks>
#include <navmesh>

float DJTime[MAXPLAYERS + 1];
float NoDodge[MAXPLAYERS + 1];

public Plugin myinfo = 
{
	name = "[TF2] E-BOT",
	author = "EfeDursun125",
	description = "",
	version = "0.03",
	url = "https://steamcommunity.com/id/EfeDursun91/"
}

#pragma tabsize 0
#include <ebotai/utilities>
#include <ebotai/target>
#include <ebotai/process>
#include <ebotai/waypoints>
#include <ebotai/path>
#include <ebotai/think>
#include <ebotai/check>
#include <ebotai/look>
#include <ebotai/autoattack>

// Goal-Oriented Action Planning (GOAP)
#include <ebotai/goap/defaultai>
#include <ebotai/goap/gethealth>
#include <ebotai/goap/getammo>
#include <ebotai/goap/attack>
#include <ebotai/goap/idle>
#include <ebotai/goap/defend>
#include <ebotai/goap/spylurk>
#include <ebotai/goap/spyhunt>
#include <ebotai/goap/spysap>
#include <ebotai/goap/heal>
#include <ebotai/goap/engineeridle>
#include <ebotai/goap/engineerbuildsentry>
#include <ebotai/goap/engineerbuilddispenser>
#include <ebotai/goap/engineerbuildteleenter>
#include <ebotai/goap/engineerbuildteleexit>
#include <ebotai/goap/hide>

// trigger on plugin start
public void OnPluginStart()
{
	RegConsoleCmd("ebot_waypoint_on", WaypointOn);
	RegConsoleCmd("ebot_waypoint_off", WaypointOff);
	RegConsoleCmd("ebot_waypoint_add", WaypointCreate);
	RegConsoleCmd("ebot_waypoint_delete", WaypointDelete);
	RegConsoleCmd("ebot_waypoint_save", SaveWaypoint);
	RegConsoleCmd("ebot_waypoint_add_path", PathAdd);
	RegConsoleCmd("ebot_waypoint_select", Select);
	RegConsoleCmd("ebot_waypoint_delete_path", PathDelete);
	RegConsoleCmd("ebot_waypoint_add_radius", AddRadius, "after 128 it will be 0");
	RegConsoleCmd("ebot_waypoint_set_flag", SetFlag);
	RegConsoleCmd("ebot_waypoint_set_team", SetTeam);
	RegConsoleCmd("ebot_waypoint_set_area", SetArea);
	RegConsoleCmd("ebot_waypoint_set_aim_start", SetAim1);
	RegConsoleCmd("ebot_waypoint_set_aim_end", SetAim2);
	RegConsoleCmd("ebot_addbot", AddEBot);
	RegConsoleCmd("ebot_kickbot", KickEBot);
	CreateDirectory("addons/sourcemod/ebot", 3);
	HookEvent("player_hurt", BotHurt, EventHookMode_Post);
	HookEvent("player_spawn", BotSpawn, EventHookMode_Post);
	HookEvent("player_death", BotDeath, EventHookMode_Post);
	HookEvent("teamplay_point_startcapture", PointStartCapture, EventHookMode_Post);
	HookEvent("teamplay_point_unlocked", PointUnlocked, EventHookMode_Post);
	HookEvent("teamplay_point_captured", PointCaptured, EventHookMode_Post);
	HookEvent("teamplay_round_start", OnRoundStart, EventHookMode_Post);
	//HookEvent("teamplay_waiting_ends", PlayAnimation, EventHookMode_Post);
	EBotDebug = CreateConVar("ebot_debug", "0", "");
	EBotFPS = CreateConVar("ebot_run_fps", "0.05", "0.033 = 29 FPS | 0.05 = 20 FPS | 0.1 = 10 FPS");
	EBotMelee = CreateConVar("ebot_melee_range", "128", "");
	EBotSenseMin = CreateConVar("ebot_minimum_sense_chance", "10", "Minimum 10");
	EBotSenseMax = CreateConVar("ebot_maximum_sense_chance", "90", "Maximum 90");
	EBotDifficulty = CreateConVar("ebot_difficulty", "2", "0 = Beginner | 1 = Easy | 2 = Normal | 3 = Hard | 4 = Expert");
	EBotMedicFollowRange = CreateConVar("ebot_medic_follow_range", "150", "");
	EBotChangeClassChance = CreateConVar("ebot_change_class_chance", "35", "");
	EBotDeadChat = CreateConVar("ebot_dead_chat_chance", "30", "");
	m_eBotDodgeRangeMin = CreateConVar("ebot_minimum_dodge_range", "512", "the range when enemy closer than a value, bot will start dodging enemies");
	m_eBotDodgeRangeMax = CreateConVar("ebot_maximum_dodge_range", "1024", "the range when enemy closer than a value, bot will start dodging enemies");
	m_eBotDodgeRangeChance = CreateConVar("ebot_dodge_change_range_chance", "10", "the chance for change dodge range when attack process ends (1-100)");
	EBotQuota = CreateConVar("ebot_quota", "-1", "");

	sv_cheats = FindConVar("sv_cheats");
	cheat_flags = GetConVarFlags(sv_cheats);
	
	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsValidClient(i))
			continue;

		if (!IsFakeClient(i))
			continue;
		
		if (StrContains(currentMap, "mvm_" , false) != -1 && GetClientTeam(i) != 2)
			continue;
			
		isEBot[i] = true;
		SDKHook(i, SDKHook_OnTakeDamage, OnTakeDamage);
		m_difficulty[i] = -1;
	}
}

// trigger on map start
public void OnMapStart()
{
	SetConVarFlags(sv_cheats, cheat_flags);
	GetCurrentMap(currentMap, sizeof(currentMap));
	SetConVarInt(FindConVar("nav_generate_fencetops"), 0);
	SetConVarInt(FindConVar("mp_waitingforplayers_cancel"), 1);
	InitGamedata();
	ServerCommand("sv_tags ebot");
	AddServerTag("ebot");
	m_laserIndex = PrecacheModel("materials/sprites/laserbeam.vmt");
	m_beamIndex = PrecacheModel("materials/sprites/lgtning.vmt");

	currentActiveArea = GetBluControlPointCount() + 1;
	healthpacks = 0;
	ammopacks = 0;
	pathdelayer = 0.0;
	BotCheckTimer = 0.0;

	for (int x = 0; x <= GetMaxEntities(); x++)
	{
		if (IsValidHealthPack(x))
			healthpacks++;
		
		if (IsValidAmmoPack(x))
			ammopacks++;
	}
	
	if (StrContains(currentMap, "ctf_" , false) != -1)
	{
		int flag = -1;
		while ((flag = FindEntityByClassname(flag, "item_teamflag")) != INVALID_ENT_REFERENCE)
		{
			if (IsValidEntity(flag))
			{
				if (GetTeamNumber(flag) == 2)
					m_redFlagCapPoint = GetOrigin(flag);
				else if (GetTeamNumber(flag) == 3)
					m_bluFlagCapPoint = GetOrigin(flag);
			}
		}
	}

	InitializeWaypoints();
}

public void EBotDeathChat(int client)
{
	if (GetRandomInt(1, 100) > GetConVarInt(EBotDeadChat))
		return;

	char filepath[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, filepath, sizeof(filepath), "ebot/chat/ebot_deadchat.txt");
    File fp = OpenFile(filepath, "r");

	ArrayList RandomChat = new ArrayList(ByteCountToCells(65));
    if (fp != null)
    {
        while (!fp.EndOfFile())
		{
			char line[MAX_NAME_LENGTH];
			fp.ReadLine(line, sizeof(line));
			if (strlen(line) <= 2)
				continue;
			
			if (StrContains(line, "//") != -1)
				continue;
			
			TrimString(line);
			RandomChat.PushString(line);
		}

		fp.Close();

		char ChosenOne[MAX_NAME_LENGTH] = "";
		if (RandomChat.Length > 0)
		{
			RandomChat.GetString(GetRandomInt(0, RandomChat.Length - 1), ChosenOne, sizeof(ChosenOne));
			FakeClientCommand(client, "say %s", ChosenOne);
		}
    }
}

// trigger on client put in the server
public void OnClientPutInServer(int client)
{
	if (addBot && IsValidClient(client) && IsFakeClient(client))
	{
		addBot = false;
		isEBot[client] = true;
		SDKHook(client, SDKHook_OnTakeDamage, OnTakeDamage);
	}

	// delete path positions
	delete m_positions[client];
	m_positions[client] = new ArrayList(3);
	delete m_pathIndex[client];
	m_pathIndex[client] = new ArrayList();
	m_goalPosition[client] = NULL_VECTOR;
	m_goalEntity[client] = -1;
	m_nextStuckCheck[client] = GetGameTime() + 5.0;
	m_enemiesNearCount[client] = 0;
	m_friendsNearCount[client] = 0;
	m_hasEnemiesNear[client] = false;
	m_hasFriendsNear[client] = false;
	m_lowAmmo[client] = false;
	m_lowHealth[client] = false;
	m_pathDelay[client] = 0.0;
	DJTime[client] = 0.0;
	m_attack2Timer[client] = 0.0;
	m_attackTimer[client] = 0.0;
	m_thinkTimer[client] = 0.0;
	m_checkTimer[client] = 0.0;
	m_wasdTimer[client] = 0.0;
	m_knownSentry[client] = -1;
	CurrentProcessTime[client] = 0.0;
	m_itAimStart[client] = GetGameTime();
	
	if (GetConVarInt(EBotDifficulty) < 0 || GetConVarInt(EBotDifficulty) > 4)
		m_difficulty[client] = GetRandomInt(0, 4);
	else
		m_difficulty[client] = GetConVarInt(EBotDifficulty);
	
	// check host entity
	if (m_hasHostEntity == false)
		FindHostEntity();
}

public Action WaypointOn(int client, int args)
{
	showWaypoints = true;
	PrintHintTextToAll("Waypoints are now enabled");
	return Plugin_Handled;
}

public Action WaypointOff(int client, int args)
{
	showWaypoints = false;
	PrintHintTextToAll("Waypoints are now disabled");
	return Plugin_Handled;
}

public Action WaypointCreate(int client, int args)
{
	if (showWaypoints)
		WaypointAdd(VectorAsInt(GetOrigin(m_hostEntity)));
	else
	{
		showWaypoints = true;
		PrintHintTextToAll("Waypoints are now enabled");
	}
	return Plugin_Handled;
}

public Action WaypointDelete(int client, int args)
{
	if (showWaypoints)
	{
		if (nearestIndex != -1 && GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(64))
			DeleteWaypointIndex(nearestIndex);
		else
			PrintHintTextToAll("Move closer to waypoint");
	}
	else
	{
		showWaypoints = true;
		PrintHintTextToAll("Waypoints are now enabled");
	}
	return Plugin_Handled;
}

public Action SaveWaypoint(int client, int args)
{
	WaypointSave();
	return Plugin_Handled;
}

public Action AddRadius(int client, int args)
{
	if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(64))
	{
		m_paths[nearestIndex].radius += 16;
		if (m_paths[nearestIndex].radius > 128)
			m_paths[nearestIndex].radius = 0;
		PrintHintTextToAll("Current radius is %d", m_paths[nearestIndex].radius);
	}
	else
		PrintHintTextToAll("Move closer to waypoint");
	return Plugin_Handled;
}

public Action SetFlag(int client, int args)
{
	char flag[32];
    GetCmdArgString(flag, sizeof(flag));
	if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(64))
	{
		int intflag = StringToInt(flag);
		if (intflag == _:WAYPOINT_DEFEND || intflag == _:WAYPOINT_SENTRY || intflag == _:WAYPOINT_DEMOMANCAMP || intflag == _:WAYPOINT_SNIPER || intflag == _:WAYPOINT_TELEPORTERENTER || intflag == _:WAYPOINT_TELEPORTEREXIT)
		{
			float origin[3];
			GetAimOrigin(m_hostEntity, origin);
			m_paths[nearestIndex].campStart = VectorAsInt(origin);
			m_paths[nearestIndex].campEnd = VectorAsInt(origin);
		}
		m_paths[nearestIndex].flags = StringToInt(flag);
		PrintHintTextToAll("Waypoint Flag Added: %d", StringToInt(flag));
	}
	else
		PrintHintTextToAll("Move closer to waypoint");
	return Plugin_Handled;
}

public Action SetTeam(int client, int args)
{
	char team[32];
    GetCmdArgString(team, sizeof(team));
	if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(64))
	{
		m_paths[nearestIndex].team = StringToInt(team);
		PrintHintTextToAll("Waypoint Team Changed: %d", view_as<TFTeam>(StringToInt(team)));
	}
	else
		PrintHintTextToAll("Move closer to waypoint");
	return Plugin_Handled;
}

public Action Select(int client, int args)
{
	if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(64))
	{
		savedIndex = nearestIndex;
		PrintHintTextToAll("Waypoint selected %d", savedIndex);
	}
	else
		PrintHintTextToAll("Move closer to waypoint");
	return Plugin_Handled;
}

public Action PathAdd(int client, int args)
{
	if (savedIndex == -1)
		PrintHintTextToAll("Select a waypoint first");
	else if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(64))
	{
		AddPath(savedIndex, nearestIndex, GetVectorDistance(VectorAsFloat(m_paths[savedIndex].origin), VectorAsFloat(m_paths[nearestIndex].origin)));
		savedIndex = -1;
	}
	else
		PrintHintTextToAll("Move closer to waypoint");
	return Plugin_Handled;
}

public Action PathDelete(int client, int args)
{
	if (savedIndex == -1)
		PrintHintTextToAll("Select a waypoint first");
	else if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(64))
	{
		DeletePath(savedIndex, nearestIndex);
		savedIndex = -1;
	}
	else
		PrintHintTextToAll("Move closer to waypoint");
	return Plugin_Handled;
}

public Action SetArea(int client, int args)
{
	char area[32];
    GetCmdArgString(area, sizeof(area));
	if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(64))
	{
		m_paths[nearestIndex].activeArea = StringToInt(area);
		PrintHintTextToAll("Waypoint Area Set: %d", StringToInt(area));
	}
	else
		PrintHintTextToAll("Move closer to waypoint");
	return Plugin_Handled;
}

public Action SetAim1(int client, int args)
{
	if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(64))
	{
		float origin[3];
		GetAimOrigin(m_hostEntity, origin);
		m_paths[nearestIndex].campStart = VectorAsInt(origin);
		PrintHintTextToAll("Waypoint Set Aim Start: %d %d %d", m_paths[nearestIndex].campStart[0], m_paths[nearestIndex].campStart[1], m_paths[nearestIndex].campStart[2]);
	}
	else
		PrintHintTextToAll("Move closer to waypoint");
	return Plugin_Handled;
}

public Action SetAim2(int client, int args)
{
	if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(64))
	{
		float origin[3];
		GetAimOrigin(m_hostEntity, origin);
		m_paths[nearestIndex].campEnd = VectorAsInt(origin);
		PrintHintTextToAll("Waypoint Set Aim End: %d %d %d", m_paths[nearestIndex].campEnd[0], m_paths[nearestIndex].campEnd[1], m_paths[nearestIndex].campEnd[2]);
	}
	else
		PrintHintTextToAll("Move closer to waypoint");
	return Plugin_Handled;
}

public Action AddEBot(int client, int args)
{
	AddEBotConsole();
}

public void AddEBotConsole()
{
	char filepath[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, filepath, sizeof(filepath), "ebot/ebot_names.txt");
    File fp = OpenFile(filepath, "r");

	ArrayList BotNames = new ArrayList(ByteCountToCells(65));
    if (fp != null)
    {
        while (!fp.EndOfFile())
		{
			char line[MAX_NAME_LENGTH];
			fp.ReadLine(line, sizeof(line));
			if (strlen(line) <= 2)
				continue;
			
			if (StrContains(line, "//") != -1)
				continue;
			
			TrimString(line);
			if (NameAlreadyTakenByPlayer(line))
				continue;
			
			BotNames.PushString(line);
		}

		fp.Close();
    }

	char ChosenName[MAX_NAME_LENGTH] = "";
	if (BotNames.Length > 0)
		BotNames.GetString(GetRandomInt(0, BotNames.Length - 1), ChosenName, sizeof(ChosenName));
	else
		Format(ChosenName, sizeof(ChosenName), "ebot %d", GetRandomInt(0, 9999));
	
	SetConVarFlags(sv_cheats, cheat_flags^(FCVAR_NOTIFY|FCVAR_REPLICATED));
	addBot = true;
	ServerCommand("sv_cheats 1; bot -name \"%s\" -class random; sv_cheats 0", ChosenName);
	SetConVarFlags(sv_cheats, cheat_flags);
}

public Action KickEBot(int client, int args)
{
	KickEBotConsole();
	return Plugin_Handled;
}

public void KickEBotConsole()
{
	addBot = false;
	int EBotTeam;
	if (GetTeamClientCount(2) > GetTeamClientCount(3))
		EBotTeam = 2;
	else if (GetTeamClientCount(2) < GetTeamClientCount(3))
		EBotTeam = 3;
	else
		EBotTeam = GetRandomInt(2, 3);
	
	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsValidClient(i))
			continue;
		
		if (!isEBot[i] && !IsFakeClient(i))
			continue;
		
		if (GetClientTeam(i) != EBotTeam)
			continue;
		
		KickClient(i);
		break;
	}
}

float hudtext = 0.0;
public void OnGameFrame()
{
	if (GetConVarInt(EBotQuota) > -1)
	{
		if (BotCheckTimer < GetGameTime())
		{
			if (GetTotalPlayersCount() < GetConVarInt(EBotQuota))
				AddEBotConsole();
			else if (GetTotalPlayersCount() > GetConVarInt(EBotQuota))
				KickEBotConsole();
			else if ((GetPlayersCountRed() + 1) < GetPlayersCountBlu())
				KickEBotConsole();
			else if ((GetPlayersCountBlu() + 1) < GetPlayersCountRed())
				KickEBotConsole();

			BotCheckTimer = GetGameTime() + 1.0;
		}
	}

	DrawWaypoints();

	if (GetConVarInt(EBotDebug) == 1)
	{
		if (!IsValidClient(m_hostEntity))
			FindHostEntity();
		
		if (hudtext < GetGameTime())
		{
			int bot = GetEntPropEnt(m_hostEntity, Prop_Send, "m_hObserverTarget");
			if (IsValidClient(bot) && IsFakeClient(bot))
			{
				SetHudTextParams(0.0, -1.0, 0.52, 255, 255, 255, 255, 2, 1.0, 0.0, 0.0);
        		ShowHudText(m_hostEntity, -1, "Goal Index: %d\nGoal Position: %d %d %d\nGoal Entity: %d\nEnemy Near: %s\nFriend Near: %s\nEntity Near: %s\nSense: %d\nCurrent Process: %s\nRemembered Process: %s\nDifficulty: %d", 
				m_goalIndex[bot], 
				RoundFloat(m_goalPosition[bot][0]), 
				RoundFloat(m_goalPosition[bot][1]), 
				RoundFloat(m_goalPosition[bot][2]), 
				m_goalEntity[bot], 
				m_hasEnemiesNear[bot] ? "Yes" : "No", 
				m_hasFriendsNear[bot] ? "Yes" : "No", 
				m_hasEntitiesNear[bot] ? "Yes" : "No", 
				m_eBotSenseChance[bot], 
				GetProcessName(CurrentProcess[bot]), 
				GetProcessName(RememberedProcess[bot]), 
				m_difficulty[bot]);
				hudtext = GetGameTime() + 0.5;
			}
		}
	}

	if (GetConVarFloat(EBotFPS) > 0.1)
		SetConVarFloat(EBotFPS, 0.1);
	
	if (GetConVarFloat(EBotFPS) < 0.033)
		SetConVarFloat(EBotFPS, 0.033);
}

// repeat for all clients
public Action OnPlayerRunCmd(int client, &buttons, &impulse, float vel[3], float angles[3])
{
	// is client valid? if not, we can't do anything
	if (!IsValidClient(client))
		return Plugin_Continue;
		
	// is client is bot? if not, we can't do anything
	if (!IsEBot(client))
		return Plugin_Continue;
	
	// auto backstab
	if (TF2_GetPlayerClass(client) == TFClass_Spy && IsWeaponSlotActive(client, 2))
	{
		int melee = GetPlayerWeaponSlot(client, TFWeaponSlot_Melee);
		if (IsValidEntity(melee) && HasEntProp(melee, Prop_Send, "m_bReadyToBackstab") && GetEntProp(melee, Prop_Send, "m_bReadyToBackstab"))
			buttons |= IN_ATTACK;
	}

	// trigger scout double jump
	if (DJTime[client] <= GetGameTime())
	{
		if (m_positions[client] != null && m_positions[client].Length > 0)
		{
			float flGoPos[3];
			m_positions[client].GetArray(m_targetNode[client], flGoPos);
			MoveTo(client, flGoPos, !m_hasWaypoints);
		}
		buttons |= IN_JUMP;
		DJTime[client] = GetGameTime() + 99999;
	}
	
	// check if client pressing any buttons and set them
	if (m_buttons[client] != 0)
	{
		buttons |= m_buttons[client];
		m_buttons[client] = 0;
	}

	// is client alive? if not, we can't do anything
	if (IsPlayerAlive(client))
	{
		if (m_attackTimer[client] > GetGameTime())
			buttons |= IN_ATTACK;
		
		if (m_attack2Timer[client] > GetGameTime())
			buttons |= IN_ATTACK2;
			
		if (GetEntProp(client, Prop_Send, "m_bJumping"))
			buttons |= IN_DUCK;

		if (m_thinkTimer[client] < GetGameTime())
		{
			if (CurrentProcess[client] != PRO_ATTACK)
			{
				m_moveVel[client][0] = 0.0;
				m_moveVel[client][1] = 0.0;
				m_moveVel[client][2] = 0.0;
			}
			
			CheckSlowThink(client);
			ThinkAI(client);
			m_thinkTimer[client] = GetGameTime() + GetConVarFloat(EBotFPS);
		}
		else
		{
			// aim to position
			LookAtPosition(client, m_lookAt[client]);
			int nOldButtons = GetEntProp(client, Prop_Data, "m_nOldButtons");
			SetEntProp(client, Prop_Data, "m_nOldButtons", (nOldButtons &= ~(IN_JUMP|IN_DUCK)));
		}
		
		// move to position
		vel = m_moveVel[client];
	}
	
	return Plugin_Continue;
}

// trigger when bot spawns
public Action BotSpawn(Handle event, char[] name, bool dontBroadcast)
{
	int client = GetClientOfUserId(GetEventInt(event, "userid"));
	if (IsEBot(client))
	{
		int changeclass = GetRandomInt(1, 100);
		if (changeclass <= GetConVarInt(EBotChangeClassChance))
		{
			TF2_SetPlayerClass(client, view_as<TFClassType>(GetRandomInt(1, 9)));
			if (IsValidClient(client) && IsPlayerAlive(client))
				TF2_RespawnPlayer(client);
		}
		
		delete m_positions[client];
		m_positions[client] = new ArrayList(3);
		m_enemiesNearCount[client] = 0;
		m_friendsNearCount[client] = 0;
		m_hasEnemiesNear[client] = false;
		m_hasFriendsNear[client] = false;
		m_lowAmmo[client] = false;
		m_lowHealth[client] = false;
		m_knownSpy[client] = -1;
		m_goalIndex[client] = -1;
		m_goalEntity[client] = -1;
		m_lastFailedWaypoint[client] = -1;
		m_itAimStart[client] = GetGameTime();
		DeletePathNodes(client);

		if (TF2_GetPlayerClass(client) == TFClass_Engineer)
		{
			m_teleEnterTriggerBlock[client] = false;
			m_teleExitTriggerBlock[client] = false;
			SetProcess(client, PRO_ENGINEERIDLE, 99999.0, "", true);
		}
		else if (TF2_GetPlayerClass(client) == TFClass_Spy)
			SetProcess(client, PRO_SPYLURK, 99999.0, "", true);
		else
			SetProcess(client, PRO_DEFAULT, 99999.0, "", true);
		
		if (m_hasRouteWaypoints)
		{
			int index = -1;
			ArrayList RouteWaypoints = new ArrayList();
			for (int i = 0; i < m_waypointNumber; i++)
			{
				if (m_paths[i].flags != _:WAYPOINT_ROUTE)
					continue;
				
				// blocked waypoint
   				if (m_paths[i].activeArea != 0 && m_paths[i].activeArea != currentActiveArea)
					continue;

				if (m_lastFailedWaypoint[client] == i)
					continue;
				
    			// not for our team
				int cteam = GetClientTeam(client);
 				if (cteam == 3 && m_paths[i].team == 2)
    				continue;
				
   				if (cteam == 2 && m_paths[i].team == 3)
        			continue;

				RouteWaypoints.Push(i);
			}

			if (RouteWaypoints.Length > 0)
				index = RouteWaypoints.Get(GetRandomInt(0, RouteWaypoints.Length - 1));
			delete RouteWaypoints;

			if (index != -1)
			{
				AStarFindPath(-1, index, client, VectorAsFloat(m_paths[index].origin));
				m_nextStuckCheck[client] = GetGameTime() + 20.0;
			}
		}

		m_nextStuckCheck[client] = GetGameTime() + 20.0;
	}
}

// trigger when bot spawns
public Action BotDeath(Handle event, char[] name, bool dontBroadcast)
{
	int victim = GetClientOfUserId(GetEventInt(event, "userid"));
	int client = GetClientOfUserId(GetEventInt(event, "attacker"));
	
	if (IsEBot(client))
	{
		if (GetRandomInt(1, 3) == 1 && !m_lowHealth[client] && !m_hasEntitiesNear[client])
		{
			FindFriendsAndEnemiens(client);
			if (!m_hasEnemiesNear[client] && (!m_hasFriendsNear[client] || TF2_GetPlayerClass(client) == TFClass_Sniper))
				FakeClientCommandThrottled(client, "taunt");
		}
	}

	if (IsEBot(victim))
		EBotDeathChat(victim);
}

// trigger when bot gets damaged
public Action BotHurt(Handle event, char[] name, bool dontBroadcast)
{
	int client = GetClientOfUserId(GetEventInt(event, "userid"));
	int target = GetClientOfUserId(GetEventInt(event, "attacker"));

	if (!m_hasEntitiesNear[client] && IsValidEntity(target))
	{
		m_pauseTime[client] = GetGameTime() + GetRandomFloat(2.5, 5.0);
		m_lookAt[client] = GetCenter(target);
		m_nearestEntity[client] = target;
		m_hasEntitiesNear[client] = true;
	}
	
	if (IsEBot(client) && IsValidClient(target))
	{
		if (!m_hasEnemiesNear[client] || !IsPlayerAlive(m_nearestEnemy[client]))
		{
			m_lookAt[client] = GetEyePosition(target);
			m_pauseTime[client] = GetGameTime() + GetRandomFloat(2.5, 5.0);
			m_nearestEnemy[client] = target;
			m_hasEnemiesNear[client] = true;
		}
		
		if (TF2_GetPlayerClass(client) == TFClass_Spy)
		{
			if (GetRandomInt(1, GetMaxHealth(client)) > GetClientHealth(client))
				m_buttons[client] |= IN_ATTACK2;
			
			if (ChanceOf(m_eBotSenseChance[target]) && IsEBot(target))
				m_knownSpy[target] = client;
			else
			{
				for (int search = 1; search <= MaxClients; search++)
				{
					if (IsValidClient(search) && IsPlayerAlive(search) && search != client && GetClientTeam(client) != GetClientTeam(search))
					{
						if (ChanceOf(m_eBotSenseChance[search]) && IsEBot(search))
							m_knownSpy[search] = client;
					}
				}
			}
		}
	}
}

// on bot take damage
public Action OnTakeDamage(victim, &attacker, &inflictor, &Float:damage, &damagetype)
{
	if (IsFakeClient(victim) && victim == attacker)
	{
		damage = 0.0;
		return Plugin_Handled;
	}

	return Plugin_Continue;
}

public Action OnRoundStart(Handle event, char[] name, bool dontBroadcast)
{
	if (StrContains(currentMap, "arena_" , false) != -1)
	{
		currentActiveArea = 1;

		if (GetConVarInt(EBotDebug) == 1)
			PrintHintTextToAll("Active area: %d", currentActiveArea);
	}
	else
	{
		currentActiveArea = GetBluControlPointCount() + 1;

		if (GetConVarInt(EBotDebug) == 1)
			PrintHintTextToAll("Active area: %d", currentActiveArea);
		
		/*GameRules_SetProp("m_nRoundsPlayed", 0);
		GameRules_SetProp("m_nMatchGroupType", 7);
		RequestFrame(Frame_RestartTime);*/
	}

	for (int client = 1; client <= MaxClients; client++)
	{
		if (!IsValidClient(client))
			continue;
		
		if (!IsEBot(client))
			continue;
		
		if (!IsPlayerAlive(client))
			continue;

		if (m_hasRouteWaypoints)
		{
			int index = -1;
			ArrayList RouteWaypoints = new ArrayList();
			for (int i = 0; i < m_waypointNumber; i++)
			{
				if (m_paths[i].flags != _:WAYPOINT_ROUTE)
					continue;
				
				// blocked waypoint
   				if (m_paths[i].activeArea != 0 && m_paths[i].activeArea != currentActiveArea)
					continue;

				if (m_lastFailedWaypoint[client] == i)
					continue;
				
    			// not for our team
				int cteam = GetClientTeam(client);
 				if (cteam == 3 && m_paths[i].team == 2)
    				continue;
				
   				if (cteam == 2 && m_paths[i].team == 3)
        			continue;

				RouteWaypoints.Push(i);
			}

			if (RouteWaypoints.Length > 0)
				index = RouteWaypoints.Get(GetRandomInt(0, RouteWaypoints.Length - 1));
			delete RouteWaypoints;

			if (index != -1)
			{
				AStarFindPath(-1, index, client, VectorAsFloat(m_paths[index].origin));
				m_nextStuckCheck[client] = GetGameTime() + 20.0;
			}
		}
	}

	return Plugin_Handled;
}

public Action PointStartCapture(Handle event, char[] name, bool dontBroadcast)
{
	if (StrContains(currentMap, "arena_" , false) != -1)
	{
		currentActiveArea = 2;

		if (GetConVarInt(EBotDebug) == 1)
			PrintHintTextToAll("Active area: %d", currentActiveArea);
	}
	else
	{
		int currentArea = GetEventInt(event, "cp");
		currentActiveArea = currentArea + 1;

		if (currentActiveArea >= 7)
			currentActiveArea = 1;

		if (currentActiveArea < 1)
			currentActiveArea = 1;

		if (GetConVarInt(EBotDebug) == 1)
			PrintHintTextToAll("Active area: %d", currentActiveArea);
	}
}

public Action PointUnlocked(Handle event, char[] name, bool dontBroadcast)
{
	if (StrContains(currentMap, "arena_" , false) != -1)
	{
		currentActiveArea = 2;

		if (GetConVarInt(EBotDebug) == 1)
			PrintHintTextToAll("Active area: %d", currentActiveArea);
	}
	else
	{
		int currentArea = GetEventInt(event, "cp");
		currentActiveArea = currentArea + 1;

		if (currentActiveArea >= 7)
			currentActiveArea = 1;

		if (currentActiveArea < 1)
			currentActiveArea = 1;

		if (GetConVarInt(EBotDebug) == 1)
			PrintHintTextToAll("Active area: %d", currentActiveArea);
	}
}

public Action PointCaptured(Handle event, char[] name, bool dontBroadcast)
{
	if (StrContains(currentMap, "arena_" , false) != -1)
	{
		currentActiveArea = 2;

		if (GetConVarInt(EBotDebug) == 1)
			PrintHintTextToAll("Active area: %d", currentActiveArea);
	}
	else
	{
		int currentArea = GetEventInt(event, "cp");
		int team = GetEventInt(event, "team");

		if (team == 3)
			currentActiveArea = currentArea + 2;
		else
			currentActiveArea = currentArea - 2;

		if (currentActiveArea >= 7)
			currentActiveArea = 1;

		if (currentActiveArea < 1)
			currentActiveArea = 1;

		if (GetConVarInt(EBotDebug) == 1)
			PrintHintTextToAll("Active area: %d", currentActiveArea);

		for (int search = 1; search <= MaxClients; search++)
		{
			if (!IsValidClient(search))
				continue;

			DeletePathNodes(search);
		}
	}
}

public Action PrintHudTextOnStart(Handle timer)
{
    PrintHintTextToAll("%s", m_aboutTheWaypoint);
}

public Action PlayAnimation(Handle event, char[] name, bool dontBroadcast)
{
	GameRules_SetProp("m_nRoundsPlayed", 0);
	GameRules_SetProp("m_nMatchGroupType", 7);
	RequestFrame(Frame_RestartTime);
	return Plugin_Handled;
}

public Action Timer_RestartTime(Handle timer)
{
	Event event = CreateEvent("restart_timer_time");
	event.SetInt("time", gI_RestartTimerIteration);
	event.Fire();

	if (gI_RestartTimerIteration-- == 10)
	{
		gH_RestartTimer = CreateTimer(1.0, Timer_RestartTime, 0, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
		return Plugin_Continue;
	}

	if (gI_RestartTimerIteration > 0)
		return Plugin_Continue;

	gH_RestartTimer = null;
	return Plugin_Stop;
}

void Frame_RestartTime()
{
	gI_RestartTimerIteration = 10;
	delete gH_RestartTimer;
	Timer_RestartTime(null);
}