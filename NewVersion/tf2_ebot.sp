#include <sourcemod>
#include <sdktools>
#include <tf2>
#include <tf2_stocks>
#include <navmesh>

public Plugin myinfo = 
{
	name = "[TF2] E-BOT",
	author = "EfeDursun125",
	description = "",
	version = "0.01",
	url = "https://steamcommunity.com/id/EfeDursun91/"
}

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
#include <ebotai/goap/spylurk>
#include <ebotai/goap/spyhunt>

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
	RegConsoleCmd("ebot_waypoint_add_flags", AddFlags);
	RegConsoleCmd("ebot_waypoint_delete_flags", DeleteFlags);
	CreateDirectory("addons/sourcemod/ebotWaypoints", 3);
	HookEvent("player_hurt", BotHurt, EventHookMode_Post);
	HookEvent("player_spawn", BotSpawn, EventHookMode_Post);
	EBotDebug = CreateConVar("ebot_debug", "0", "");
	EBotFPS = CreateConVar("ebot_run_fps", "0.05", "0.033 = 29 FPS | 0.05 = 20 FPS | 0.1 = 10 FPS | 0.2 = 5 FPS");
	EBotMelee = CreateConVar("ebot_melee_range", "128", "");
	EBotSenseMin = CreateConVar("ebot_minimum_sense_chance", "10", "Minimum 10");
	EBotSenseMax = CreateConVar("ebot_maximum_sense_chance", "90", "Maximum 90");
	m_eBotDodgeRangeMin = CreateConVar("ebot_minimum_dodge_range", "768", "the range when enemy closer than a value, bot will start dodging enemies");
	m_eBotDodgeRangeMax = CreateConVar("ebot_maximum_dodge_range", "1536", "the range when enemy closer than a value, bot will start dodging enemies");
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

	healthpacks = 0;
	ammopacks = 0;
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
	// delete path positions
	delete m_positions[client];
	m_positions[client] = new ArrayList(3);
	delete m_pathIndex[client];
	m_pathIndex[client] = new ArrayList();
	m_goalPosition[client] = NULL_VECTOR;
	m_goalEntity[client] = -1;
	m_nextStuckCheck[client] = GetGameTime() + 5.0;
	m_nextPathUpdate[client] = GetGameTime() + GetRandomFloat(0.5, 1.0);
	m_enemiesNearCount[client] = 0;
	m_friendsNearCount[client] = 0;
	m_hasEnemiesNear[client] = false;
	m_hasFriendsNear[client] = false;
	m_lowAmmo[client] = false;
	m_lowHealth[client] = false;
	
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

public Action AddFlags(int client, int args)
{
	if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(64))
	{
		m_paths[nearestIndex].flags |= args;
		PrintHintTextToAll("Waypoint Flag Added: %d", view_as<WaypointFlag>(args));
	}
	else
		PrintHintTextToAll("Move closer to waypoint");
	return Plugin_Handled;
}

public Action DeleteFlags(int client, int args)
{
	if (GetVectorDistance(GetOrigin(m_hostEntity), VectorAsFloat(m_paths[nearestIndex].origin), true) <= Squared(64))
	{
		m_paths[nearestIndex].flags |= args;
		PrintHintTextToAll("Waypoint Flag Deleted: %d", view_as<WaypointFlag>(args));
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

public void OnGameFrame()
{
	DrawWaypoints();

	if (GetConVarInt(EBotDebug) == 1)
	{
		if (!IsValidClient(m_hostEntity))
			FindHostEntity();
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
		m_nextPathUpdate[client] = GetGameTime() + GetRandomFloat(0.5, 1.0);
		m_enemiesNearCount[client] = 0;
		m_friendsNearCount[client] = 0;
		m_hasEnemiesNear[client] = false;
		m_hasFriendsNear[client] = false;
		m_lowAmmo[client] = false;
		m_lowHealth[client] = false;
	}
}

// trigger when bot gets damaged
public Action BotHurt(Handle event, char[] name, bool dontBroadcast)
{
	int client = GetClientOfUserId(GetEventInt(event, "userid"));
	int target = GetClientOfUserId(GetEventInt(event, "attacker"));
	
	if (IsFakeClient(client) && IsValidClient(target))
	{
		m_lookAt[client] = GetEyePosition(target);
		m_pauseTime[client] = GetGameTime() + GetRandomFloat(2.5, 5.0);
		if (TF2_GetPlayerClass(client) == TFClass_Spy && GetRandomInt(1, GetMaxHealth(client)) > GetClientHealth(client))
			m_buttons[client] |= IN_ATTACK2;
	}
	
	if (TF2_GetPlayerClass(client) == TFClass_Spy && IsValidClient(target))
	{
		if (ChanceOf(m_eBotSenseChance[target]) && IsFakeClient(target))
			m_knownSpy[target] = client;
		else
		{
			for (int search = 1; search <= MaxClients; search++)
			{
				if (IsValidClient(search) && IsClientInGame(search) && IsPlayerAlive(search) && search != client && GetClientTeam(client) != GetClientTeam(search))
				{
					if (ChanceOf(m_eBotSenseChance[search]) && IsFakeClient(search))
						m_knownSpy[search] = client;
				}
			}
		}
	}
}