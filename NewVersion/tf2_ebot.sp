#include <sourcemod>
#include <sdktools>
#include <sdkhooks>
#include <tf2>
#include <tf2_stocks>
#include <navmesh>
#include <tf2items>

float autoupdate = 0.0;

float DJTime[MAXPLAYERS + 1];
float NoDodge[MAXPLAYERS + 1];

public Plugin myinfo = 
{
	name = "[TF2] E-BOT",
	author = "EfeDursun125",
	description = "",
	version = "0.05",
	url = "https://steamcommunity.com/id/EfeDursun91/"
}

#pragma tabsize 0
#include <ebotai/gamemode>
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
	EBotDebug = CreateConVar("ebot_debug", "0", "", FCVAR_NONE);
	EBotFPS = CreateConVar("ebot_run_fps", "0.1", "0.0333 = 30 FPS | 0.05 = 20 FPS | 0.1 = 10 FPS | 0.2 = 5 FPS", FCVAR_NONE);
	EBotMelee = CreateConVar("ebot_melee_range", "128", "", FCVAR_NONE);
	EBotSenseMin = CreateConVar("ebot_minimum_sense_chance", "10", "Minimum 10", FCVAR_NONE);
	EBotSenseMax = CreateConVar("ebot_maximum_sense_chance", "90", "Maximum 90", FCVAR_NONE);
	EBotDifficulty = CreateConVar("ebot_difficulty", "2", "0 = Beginner | 1 = Easy | 2 = Normal | 3 = Hard | 4 = Expert", FCVAR_NONE);
	EBotMedicFollowRange = CreateConVar("ebot_medic_follow_range", "150", "", FCVAR_NONE);
	EBotChangeClass = CreateConVar("ebot_change_class", "1", "", FCVAR_NONE);
	EBotChangeClassChance = CreateConVar("ebot_change_class_chance", "35", "", FCVAR_NONE);
	EBotDeadChat = CreateConVar("ebot_dead_chat_chance", "30", "", FCVAR_NONE);
	m_eBotDodgeRangeMin = CreateConVar("ebot_minimum_dodge_range", "512", "the range when enemy closer than a value, bot will start dodging enemies", FCVAR_NONE);
	m_eBotDodgeRangeMax = CreateConVar("ebot_maximum_dodge_range", "2048", "the range when enemy closer than a value, bot will start dodging enemies", FCVAR_NONE);
	m_eBotDodgeRangeChance = CreateConVar("ebot_dodge_change_range_chance", "10", "the chance for change dodge range when attack process ends (1-100)", FCVAR_NONE);
	EBotQuota = CreateConVar("ebot_quota", "-1", "", FCVAR_NONE);
	EBotAutoWaypoint = CreateConVar("ebot_waypoint_auto", "0", "", FCVAR_NONE);
	EBotRadius = CreateConVar("ebot_waypoint_default_radius", "0", "", FCVAR_NONE);
	EBotDistance = CreateConVar("ebot_waypoint_auto_distance", "250.0", "", FCVAR_NONE);
	EBotForceHuntEnemy = CreateConVar("ebot_force_hunt_enemy", "0", "", FCVAR_NONE);
	EBotAllowAttackButtons = CreateConVar("ebot_allow_attack_buttons", "1", "", FCVAR_NONE);

	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsValidClient(i))
			continue;

		if (!IsFakeClient(i))
			continue;
		
		if (GameRules_GetProp("m_bPlayingMannVsMachine") && GetClientTeam(i) != 2)
			continue;
		
		SDKHook(i, SDKHook_OnTakeDamage, OnTakeDamage);
		m_difficulty[i] = -1;
	}
}

// trigger on map start
public void OnMapStart()
{
	if (!GameRules_GetProp("m_bPlayingMannVsMachine"))
		SetConVarInt(FindConVar("nb_player_stop"), 1);
	
	GetCurrentMap(currentMap, sizeof(currentMap));
	AutoLoadGamemode();

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
	autoupdate = 0.0;

	for (int x = 0; x <= GetMaxEntities(); x++)
	{
		if (IsValidHealthPack(x))
			healthpacks++;
		
		if (IsValidAmmoPack(x))
			ammopacks++;
	}
	
	if (isCTF)
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
	if (IsValidClient(client))
		SDKHook(client, SDKHook_OnTakeDamage, OnTakeDamage);

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
	m_enterFail[client] = false;
	m_exitFail[client] = false;
	CurrentProcessTime[client] = 0.0;
	CurrentProcess[client] = PRO_DEFAULT;
	m_damageTime[client] = 0.0;
	
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
		WaypointAdd(GetOrigin(m_hostEntity));
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
		if (nearestIndex != -1 && GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) <= Squared(64))
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
	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) <= Squared(64))
	{
		m_paths[nearestIndex].radius += 16;
		if (m_paths[nearestIndex].radius > 128)
			m_paths[nearestIndex].radius = 0.0;

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
	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) <= Squared(64))
	{
		int intflag = StringToInt(flag);
		if (intflag == WAYPOINT_DEFEND || intflag == WAYPOINT_SENTRY || intflag == WAYPOINT_DEMOMANCAMP || intflag == WAYPOINT_SNIPER || intflag == WAYPOINT_TELEPORTERENTER || intflag == WAYPOINT_TELEPORTEREXIT)
		{
			float origin[3];
			GetAimOrigin(m_hostEntity, origin);
			m_paths[nearestIndex].campStart = origin;
			m_paths[nearestIndex].campEnd = origin;
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
	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) <= Squared(64))
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
	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) <= Squared(64))
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
	else if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) <= Squared(64))
	{
		AddPath(savedIndex, nearestIndex, GetVectorDistance(m_paths[savedIndex].origin, m_paths[nearestIndex].origin));
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
	else if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) <= Squared(64))
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
	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) <= Squared(64))
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
	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) <= Squared(64))
	{
		float origin[3];
		GetAimOrigin(m_hostEntity, origin);
		m_paths[nearestIndex].campStart = origin;
		PrintHintTextToAll("Waypoint Set Aim Start: %d %d %d", m_paths[nearestIndex].campStart[0], m_paths[nearestIndex].campStart[1], m_paths[nearestIndex].campStart[2]);
	}
	else
		PrintHintTextToAll("Move closer to waypoint");
	
	return Plugin_Handled;
}

public Action SetAim2(int client, int args)
{
	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) <= Squared(64))
	{
		float origin[3];
		GetAimOrigin(m_hostEntity, origin);
		m_paths[nearestIndex].campEnd = origin;
		PrintHintTextToAll("Waypoint Set Aim End: %d %d %d", m_paths[nearestIndex].campEnd[0], m_paths[nearestIndex].campEnd[1], m_paths[nearestIndex].campEnd[2]);
	}
	else
		PrintHintTextToAll("Move closer to waypoint");
	
	return Plugin_Handled;
}

public Action AddEBot(int client, int args)
{
	AddEBotConsole();
	return Plugin_Handled;
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

    SetConVarInt(FindConVar("sv_cheats"), 1, false, false);
	ServerCommand("bot -name \"%s\" -class random", ChosenName);
}

public Action KickEBot(int client, int args)
{
	KickEBotConsole();
	return Plugin_Handled;
}

public void KickEBotConsole()
{
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
		
		if (!IsEBot(i))
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
		int wanted = GetConVarInt(EBotQuota);

		if (BotCheckTimer < GetGameTime())
		{
			int total = GetTotalPlayersCount();
			int red = GetPlayersCountRed();
			int blu = GetPlayersCountBlu();

			if (total < wanted)
				AddEBotConsole();
			else if (total > wanted)
				KickEBotConsole();
			else if ((red + 1) < blu)
				KickEBotConsole();
			else if ((blu + 1) < red)
				KickEBotConsole();
			
			BotCheckTimer = GetGameTime() + 1.0;
		}
	}

	if (autoupdate < GetGameTime())
	{
		int newArea = GetBluControlPointCount() + 1;
		if (newArea != currentActiveArea)
		{
			currentActiveArea = newArea;

			if (GetConVarInt(EBotDebug) == 1)
				PrintHintTextToAll("Active area: %d", currentActiveArea);

			for (int search = 1; search <= MaxClients; search++)
			{
				if (!IsValidClient(search))
					continue;

				SelectObjective(search);
				DeletePathNodes(search);
			}
		}

		autoupdate = GetGameTime() + GetRandomFloat(3.0, 5.0);
	}

	DrawWaypoints();
	AutoWaypoint();
	UpdateWaypoints();

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

	if (GetConVarFloat(EBotFPS) > 0.2)
		SetConVarFloat(EBotFPS, 0.2);
	
	if (GetConVarFloat(EBotFPS) < 0.0333)
		SetConVarFloat(EBotFPS, 0.0333);
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
		if (GetConVarBool(EBotAllowAttackButtons))
		{
			if (m_attackTimer[client] > GetGameTime())
				buttons |= IN_ATTACK;
		
			if (m_attack2Timer[client] > GetGameTime())
				buttons |= IN_ATTACK2;
		}
			
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
			m_thinkTimer[client] = GetGameTime() + (GetConVarFloat(EBotFPS) + GetRandomFloat(0.001, 0.01));
		}
		else
		{
			LookAtPosition(client, m_lookAt[client], angles, m_tackEntity[client]);

			int nOldButtons = GetEntProp(client, Prop_Data, "m_nOldButtons");
			if (nOldButtons & (IN_JUMP|IN_DUCK))
				SetEntProp(client, Prop_Data, "m_nOldButtons", (nOldButtons &= ~(IN_JUMP|IN_DUCK)));
			
			if (CurrentProcess[client] == PRO_BUILDDISPENSER && m_isSlowThink[client] && GetRandomInt(1, 3) == 1)
				m_lookAt[client][1] = GetRandomFloat(1.0, 359.0);
		}

		if (TF2_IsPlayerInCondition(client, TFCond_Charging))
			buttons &= ~IN_ATTACK;

		vel = m_moveVel[client];

		if (m_paths[m_currentIndex[client]].flags == WAYPOINT_DEMOCHARGE && TF2_GetPlayerClass(client) == TFClass_DemoMan)
		{
			m_currentWaypointIndex[client]--;
			m_targetNode[client]--;
			m_currentIndex[client] = m_pathIndex[client].Get(m_currentWaypointIndex[client]);
			m_nextStuckCheck[client] = GetGameTime() + 5.0;
			float vec[3];
			float camangle[3];
			float wayorigin[3];
			wayorigin = m_paths[m_currentIndex[client]].origin;
			wayorigin[2] += GetHeight(client);
			MakeVectorFromPoints(wayorigin, GetEyePosition(client), vec);
			GetVectorAngles(vec, camangle);
			camangle[0] *= -1.0;
			camangle[1] += 180.0;
			ClampAngle(camangle);
			m_lookAt[client] = wayorigin;
			m_pauseTime[client] = GetGameTime() + 3.0;
			SetEntPropFloat(client, Prop_Send, "m_flChargeMeter", 100.0);
			TeleportEntity(client, NULL_VECTOR, camangle, vec);
			buttons |= IN_ATTACK2;
		}
	}
	
	return Plugin_Continue;
}

// trigger when bot spawns
public Action BotSpawn(Handle event, char[] name, bool dontBroadcast)
{
	int client = GetClientOfUserId(GetEventInt(event, "userid"));
	if (IsValidClient(client) && IsEBot(client))
	{
		if (GetConVarBool(EBotChangeClass))
		{
			int changeclass = GetRandomInt(1, 100);
			if (changeclass <= GetConVarInt(EBotChangeClassChance))
			{
				if (GameRules_GetProp("m_bPlayingMedieval"))
				{
					int randomclass = GetRandomInt(1, 5);
					if (randomclass == 1)
						TF2_SetPlayerClass(client, TFClass_DemoMan);
					else if (randomclass == 2)
						TF2_SetPlayerClass(client, TFClass_Spy);
					else if (randomclass == 3)
						TF2_SetPlayerClass(client, TFClass_Sniper);
					else if (randomclass == 4)
					{
						if (GetClientTeam(client) == 3)
						{
							if (currentActiveArea == 1)
								TF2_SetPlayerClass(client, TFClass_Sniper);
							else 
								TF2_SetPlayerClass(client, TFClass_DemoMan);
						}
						else
						{
							randomclass = GetRandomInt(1, 2);
							if (randomclass == 1)
								TF2_SetPlayerClass(client, TFClass_Sniper);
							else
								TF2_SetPlayerClass(client, TFClass_Spy);
						}
					}
					else
					{
						randomclass = GetRandomInt(1, 2);
						if (randomclass == 1)
							TF2_SetPlayerClass(client, TFClass_Medic);
						else
						{
							randomclass = GetRandomInt(1, 3);
							if (randomclass == 1)
								TF2_SetPlayerClass(client, TFClass_Heavy);
							else if (randomclass == 2)
								TF2_SetPlayerClass(client, TFClass_Pyro);
							else
							{
								randomclass = GetRandomInt(1, 2);
								if (randomclass == 1)
									TF2_SetPlayerClass(client, TFClass_Scout);
								else
									TF2_SetPlayerClass(client, TFClass_Soldier);
							}
						}
					}
				}
				else
				{
					//if (IsValidClient(m_lastKiller[client]) && (TF2_GetPlayerClass(m_lastKiller[client]) == TFClass_Sniper || TF2_GetPlayerClass(m_lastKiller[client]) == TFClass_Engineer))
					//	TF2_SetPlayerClass(client, TFClass_Spy);
					//else
					TF2_SetPlayerClass(client, view_as<TFClassType>(GetRandomInt(1, 9)));
				}

				if (IsPlayerAlive(client))
				{
					TF2_RespawnPlayer(client);
					DeletePathNodes(client);
				}
			}
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
		m_enterFail[client] = false;
		m_exitFail[client] = false;
		m_pathDelay[client] = 0.0;
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
				AStarFindPath(-1, index, client, m_paths[index].origin);
				m_nextStuckCheck[client] = GetGameTime() + 20.0;
			}
		}

		m_nextStuckCheck[client] = GetGameTime() + 20.0;
	}

	return Plugin_Handled;
}

// trigger when bot spawns
public Action BotDeath(Handle event, char[] name, bool dontBroadcast)
{
	int victim = GetClientOfUserId(GetEventInt(event, "userid"));
	int client = GetClientOfUserId(GetEventInt(event, "attacker"));
	
	if (IsValidClient(client) && IsEBot(client) && IsValidClient(victim))
	{
		if (TF2_GetPlayerClass(client) == TFClass_Spy)
			TF2_DisguisePlayer(client, (GetClientTeam(client) == 2 ? TFTeam_Blue : TFTeam_Red), TF2_GetPlayerClass(victim), victim);
		
		if (GetRandomInt(1, 3) == 1 && !m_lowHealth[client] && !m_hasEntitiesNear[client])
		{
			FindFriendsAndEnemiens(client);
			if (!m_hasEnemiesNear[client] && (!m_hasFriendsNear[client] || TF2_GetPlayerClass(client) == TFClass_Sniper))
				PlayTaunt(client, 463);
		}
	}

	if (IsValidClient(client) && IsEBot(victim))
	{
		m_lastKiller[victim] = client;
		EBotDeathChat(victim);
	}

	if (IsValidClient(victim))
	{
		if (IsEBot(victim) && m_currentWaypointIndex[victim] > 0)
			IncreaseDeaths(m_currentWaypointIndex[victim]);
		else
		{
			int index = FindNearestWaypoint(GetOrigin(victim), 999999.0, victim);
			if (index > 0)
				IncreaseDeaths(index);
		}
	}
	
	return Plugin_Handled;
}

// trigger when bot gets damaged
public Action BotHurt(Handle event, char[] name, bool dontBroadcast)
{
	int client = GetClientOfUserId(GetEventInt(event, "userid"));
	int target = GetClientOfUserId(GetEventInt(event, "attacker"));

	if (!IsValidClient(client) || client == target)
		return Plugin_Handled;
	
	if (!m_hasEntitiesNear[client] && IsValidEntity(target))
	{
		m_damageTime[client] = GetGameTime();
		m_pauseTime[client] = GetGameTime() + GetRandomFloat(2.5, 5.0);
		m_lookAt[client] = GetCenter(target);
		m_nearestEntity[client] = target;
		m_hasEntitiesNear[client] = true;
	}
	
	if (IsEBot(client) && IsValidClient(target))
	{
		m_damageTime[client] = GetGameTime();
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

	return Plugin_Handled;
}

// on bot take damage
public Action OnTakeDamage(victim, &attacker, &inflictor, &Float:damage, &damagetype)
{
	if (IsFakeClient(victim) && victim == attacker)
	{
		damage = 0.0;
		return Plugin_Handled;
	}

	if (IsValidClient(victim))
	{
		if (IsEBot(victim) && m_currentWaypointIndex[victim] != -1)
			IncreaseDamage(m_currentWaypointIndex[victim], damage);
		else
		{
			int index = FindNearestWaypoint(GetOrigin(victim), 999999.0, victim);
			if (index != -1)
				IncreaseDamage(index, damage);
		}
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
				AStarFindPath(-1, index, client, m_paths[index].origin);
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

	return Plugin_Handled;
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

	return Plugin_Handled;
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
			currentActiveArea = currentArea + 1;
		else
			currentActiveArea = currentArea - 1;

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

	return Plugin_Handled;
}

public Action PrintHudTextOnStart(Handle timer)
{
    PrintHintTextToAll("%s", m_aboutTheWaypoint);
	return Plugin_Handled;
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