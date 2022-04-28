#include <sourcemod>
#include <sdktools>
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
#include <ebotai/process>
#include <ebotai/utilities>
#include <ebotai/waypoints>
#include <ebotai/path>
#include <ebotai/target>
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
#include <ebotai/goap/heal>
#include <ebotai/goap/engineeridle>
#include <ebotai/goap/engineerbuildsentry>
#include <ebotai/goap/engineerbuilddispenser>

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
	CreateDirectory("addons/sourcemod/ebotWaypoints", 3);
	HookEvent("player_hurt", BotHurt, EventHookMode_Post);
	HookEvent("player_spawn", BotSpawn, EventHookMode_Post);
	HookEvent("teamplay_point_startcapture", PointStartCapture, EventHookMode_Post);
	HookEvent("teamplay_point_unlocked", PointUnlocked, EventHookMode_Post);
	HookEvent("teamplay_point_captured", PointCaptured, EventHookMode_Post);
	HookEvent("teamplay_round_start", OnRoundStart, EventHookMode_Post);
	EBotDebug = CreateConVar("ebot_debug", "0", "");
	EBotFPS = CreateConVar("ebot_run_fps", "0.05", "0.033 = 29 FPS | 0.05 = 20 FPS | 0.1 = 10 FPS | 0.2 = 5 FPS");
	EBotMelee = CreateConVar("ebot_melee_range", "128", "");
	EBotSenseMin = CreateConVar("ebot_minimum_sense_chance", "10", "Minimum 10");
	EBotSenseMax = CreateConVar("ebot_maximum_sense_chance", "90", "Maximum 90");
	EBotDifficulty = CreateConVar("ebot_difficulty", "2", "0 = Easiest | 1 = Easy | 2 = Normal | 3 = Hard | 4 = Expert");
	m_eBotMedicFollowRange = CreateConVar("ebot_medic_follow_range", "150", "");
	m_eBotDodgeRangeMin = CreateConVar("ebot_minimum_dodge_range", "512", "the range when enemy closer than a value, bot will start dodging enemies");
	m_eBotDodgeRangeMax = CreateConVar("ebot_maximum_dodge_range", "1024", "the range when enemy closer than a value, bot will start dodging enemies");
	m_eBotDodgeRangeChance = CreateConVar("ebot_dodge_change_range_chance", "10", "the chance for change dodge range when attack process ends (1-100)");
}

// trigger on map start
public void OnMapStart()
{
	GetCurrentMap(currentMap, sizeof(currentMap));
	SetConVarInt(FindConVar("nav_generate_fencetops"), 0);
	SetConVarInt(FindConVar("mp_waitingforplayers_cancel"), 1);
	InitGamedata();
	m_laserIndex = PrecacheModel("materials/sprites/laserbeam.vmt");
	m_beamIndex = PrecacheModel("materials/sprites/lgtning.vmt");
	ServerCommand("sv_tags ebot");
	AddServerTag("ebot");

	currentActiveArea = GetBluControlPointCount() + 1;
	healthpacks = 0;
	ammopacks = 0;
	pathdelayer = 0.0;
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

// trigger on client put in the server
public void OnClientPutInServer(int client)
{
	SetConVarInt(FindConVar("mp_waitingforplayers_cancel"), 1);

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
	m_fireTimer[client] = 0.0;
	m_wasdTimer[client] = 0.0;
	CurrentProcessTime[client] = 0.0;
	
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
		if (nearestIndex != -1 && GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(75))
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
	if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(75))
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
	if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(75))
	{
		int intflag = StringToInt(flag);
		if (intflag == _:WAYPOINT_DEFEND || intflag == _:WAYPOINT_SENTRY || intflag == _:WAYPOINT_DEMOMANCAMP || intflag == _:WAYPOINT_SNIPER)
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
	if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(75))
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
	if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(75))
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
	else if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(75))
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
	else if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(75))
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
	if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(75))
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
	if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(75))
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
	if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(75))
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

float hudtext = 0.0;
public void OnGameFrame()
{
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
        		ShowHudText(m_hostEntity, -1, "Current Bot\nGoal Index: %d\nGoal Position: %d %d %d\nGoal Entity: %d\nEnemy Near: %s\nFriend Near: %s\nEntity Near: %s\nSense: %d\nCurrent Process: %s\nRemembered Process: %s", 
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
				GetProcessName(RememberedProcess[bot]));
				hudtext = GetGameTime() + 0.5;
			}
		}
	}

	if (GetConVarFloat(EBotFPS) > 0.2)
		SetConVarFloat(EBotFPS, 0.2);
	
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
	if (!IsFakeClient(client))
		return Plugin_Continue;
	
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
		if (TF2_GetPlayerClass(client) == TFClass_Spy && IsWeaponSlotActive(client, 2))
		{
			int melee = GetPlayerWeaponSlot(client, TFWeaponSlot_Melee);
			if (IsValidEntity(melee) && HasEntProp(melee, Prop_Send, "m_bReadyToBackstab") && GetEntProp(melee, Prop_Send, "m_bReadyToBackstab"))
				buttons |= IN_ATTACK;
		}

		if (m_attackTimer[client] > GetGameTime())
		{
			// block attack button
			if ((TF2_GetPlayerClass(client) == TFClass_DemoMan || TF2_GetPlayerClass(client) == TFClass_Soldier) && IsWeaponSlotActive(client, 0) && CheckForward(client))
			{
				buttons &= ~IN_ATTACK;
				m_attackTimer[client] = 0.0;
			}
			else
				buttons |= IN_ATTACK;
		}
		
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
		
		// aim to position
		if (!m_isSlowThink[client])
			LookAtPosition(client, m_lookAt[client], angles);
		
		// move to position
		vel = m_moveVel[client];
	}
	
	return Plugin_Continue;
}

// trigger when bot spawns
public Action BotSpawn(Handle event, char[] name, bool dontBroadcast)
{
	int client = GetClientOfUserId(GetEventInt(event, "userid"));
	if (IsFakeClient(client))
	{
		delete m_positions[client];
		m_positions[client] = new ArrayList(3);
		m_nextStuckCheck[client] = GetGameTime() + 5.0;
		m_enemiesNearCount[client] = 0;
		m_friendsNearCount[client] = 0;
		m_hasEnemiesNear[client] = false;
		m_hasFriendsNear[client] = false;
		m_lowAmmo[client] = false;
		m_lowHealth[client] = false;
		m_knownSpy[client] = -1;
		m_goalIndex[client] = -1;
		m_goalEntity[client] = -1
		if (TF2_GetPlayerClass(client) == TFClass_Engineer)
			SetProcess(client, PRO_ENGINEERIDLE, 99999.0, "", true);
		else if (TF2_GetPlayerClass(client) == TFClass_Spy)
			SetProcess(client, PRO_SPYLURK, 99999.0, "", true);
		else
			SetProcess(client, PRO_DEFAULT, 99999.0, "", true);
		SelectObjective(client);
		DeletePathNodes(client);
		if (m_hasRouteWaypoints)
		{
			int index = -1;
			ArrayList RouteWaypoints = new ArrayList();
			for (int i = 0; i < m_waypointNumber; i++)
			{
				// blocked waypoint
   				if (m_paths[i].activeArea != 0 && m_paths[i].activeArea != currentActiveArea)
					continue;

				if (m_lastFailedWaypoint[client] == i)
					continue;

				if (m_paths[i].flags != _:WAYPOINT_ROUTE)
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
				AStarFindPath(-1, index, client);
		}
	}
}

// trigger when bot gets damaged
public Action BotHurt(Handle event, char[] name, bool dontBroadcast)
{
	int client = GetClientOfUserId(GetEventInt(event, "userid"));
	int target = GetClientOfUserId(GetEventInt(event, "attacker"));

	if (!m_hasEntitiesNear[client] && IsValidEntity(target))
	{
		m_lookAt[client] = GetCenter(target);
		m_nearestEntity[client] = target;
		m_hasEntitiesNear[client] = true;
	}
	
	if (IsFakeClient(client) && target != -1)
	{
		if (!m_hasEnemiesNear[client] || !IsPlayerAlive(m_nearestEnemy[client]))
		{
			m_lookAt[client] = GetEyePosition(target);
			m_pauseTime[client] = GetGameTime() + GetRandomFloat(2.5, 5.0);
			m_nearestEnemy[client] = target;
			m_hasEnemiesNear[client] = true;
		}
		if (TF2_GetPlayerClass(client) == TFClass_Spy && GetRandomInt(1, GetMaxHealth(client)) > GetClientHealth(client))
			m_buttons[client] |= IN_ATTACK2;
	}
	
	if (TF2_GetPlayerClass(client) == TFClass_Spy && target != -1)
	{
		if (ChanceOf(m_eBotSenseChance[target]) && IsFakeClient(target))
			m_knownSpy[target] = client;
		else
		{
			for (int search = 1; search <= MaxClients; search++)
			{
				if (IsValidClient(search) && IsPlayerAlive(search) && search != client && GetClientTeam(client) != GetClientTeam(search))
				{
					if (ChanceOf(m_eBotSenseChance[search]) && IsFakeClient(search))
						m_knownSpy[search] = client;
				}
			}
		}
	}
}

public Action OnRoundStart(Handle event, char[] name, bool dontBroadcast)
{
	currentActiveArea = GetBluControlPointCount() + 1;
	if (GetConVarInt(EBotDebug) == 1)
		PrintHintTextToAll("Active area: %d", currentActiveArea);
}

public Action PointStartCapture(Handle event, char[] name, bool dontBroadcast)
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

public Action PointUnlocked(Handle event, char[] name, bool dontBroadcast)
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

public Action PointCaptured(Handle event, char[] name, bool dontBroadcast)
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
}

public Action PrintHudTextOnStart(Handle timer)
{
    PrintHintTextToAll("%s", m_aboutTheWaypoint);
}