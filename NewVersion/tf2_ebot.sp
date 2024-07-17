const int TFMaxPlayers = 101;

#include <sourcemod>
#include <sdktools>
#include <sdkhooks>
#include <tf2>
#include <tf2_stocks>
#include <tf2items>
#include <profiler>

#include <cbasenpc>
CNavMesh NavMesh;

float autoupdate = 0.0;

float DJTime[TFMaxPlayers];
float NoDodge[TFMaxPlayers];
float CrouchTime[TFMaxPlayers];
float m_spawnTime[TFMaxPlayers];
float m_afkTime[TFMaxPlayers];

TFClassType m_class[TFMaxPlayers];
int m_team[TFMaxPlayers];
bool m_isAlive[TFMaxPlayers];

public Plugin myinfo =
{
	name = "[TF2] E-BOT",
	author = "EfeDursun125",
	description = "",
	version = "0.24",
	url = "https://steamcommunity.com/id/EfeDursun91/"
}

bool isOT;

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
#include <ebotai/voice>

// Simple State Machine
#include <ebotai/ssm/defaultai>
#include <ebotai/ssm/gethealth>
#include <ebotai/ssm/getammo>
#include <ebotai/ssm/attack>
#include <ebotai/ssm/idle>
#include <ebotai/ssm/defend>
#include <ebotai/ssm/spylurk>
#include <ebotai/ssm/spyhunt>
#include <ebotai/ssm/spysap>
#include <ebotai/ssm/heal>
#include <ebotai/ssm/hunt>
#include <ebotai/ssm/engineeridle>
#include <ebotai/ssm/engineerbuildsentry>
#include <ebotai/ssm/engineerbuilddispenser>
#include <ebotai/ssm/engineerbuildteleenter>
#include <ebotai/ssm/engineerbuildteleexit>
#include <ebotai/ssm/hide>
#include <ebotai/ssm/followpath>

// trigger on plugin start
public void OnPluginStart()
{
	RegConsoleCmd("ebot_waypoint_on", WaypointOn);
	RegConsoleCmd("ebot_waypoint_off", WaypointOff);
	RegConsoleCmd("ebot_waypoint_add", WaypointCreate);
	RegConsoleCmd("ebot_waypoint_delete", WaypointDelete);
	RegConsoleCmd("ebot_waypoint_save", SaveWaypoint);
	RegConsoleCmd("ebot_waypoint_save_nav", SaveWaypointNAV);
	RegConsoleCmd("ebot_waypoint_add_path", PathAdd);
	RegConsoleCmd("ebot_waypoint_select", Select);
	RegConsoleCmd("ebot_waypoint_delete_path", PathDelete);
	RegConsoleCmd("ebot_waypoint_set_radius", SetRadius);
	RegConsoleCmd("ebot_waypoint_add_radius", AddRadius, "after 128 it will be 0");
	RegConsoleCmd("ebot_waypoint_set_flag", SetFlag);
	RegConsoleCmd("ebot_waypoint_add_flag", AddFlag);
	RegConsoleCmd("ebot_waypoint_remove_flag", RemoveFlag);
	RegConsoleCmd("ebot_waypoint_set_team", SetTeam);
	RegConsoleCmd("ebot_waypoint_set_area", SetArea);
	RegConsoleCmd("ebot_waypoint_add_area", AddArea);
	RegConsoleCmd("ebot_waypoint_remove_area", RemoveArea);
	RegConsoleCmd("ebot_waypoint_set_aim_start", SetAim1);
	RegConsoleCmd("ebot_waypoint_set_aim_end", SetAim2);
	RegConsoleCmd("ebot_addbot", AddEBot);
	RegConsoleCmd("ebot_kickbot", KickEBot);
	RegConsoleCmd("sm_afk", Command_Afk);
	CreateDirectory("addons/sourcemod/ebot", 3);
	HookEvent("player_sapped_object", BotSap, EventHookMode_Post);
	HookEvent("player_spawn", BotSpawn, EventHookMode_Post);
	HookEvent("player_death", BotDeath, EventHookMode_Post);
	HookEvent("teamplay_point_startcapture", PointStartCapture, EventHookMode_Post);
	HookEvent("teamplay_point_unlocked", PointUnlocked, EventHookMode_Post);
	HookEvent("teamplay_point_captured", PointCaptured, EventHookMode_Post);
	HookEvent("teamplay_round_start", OnRoundStart, EventHookMode_Post);
	//HookEvent("teamplay_waiting_ends", PlayAnimation, EventHookMode_Post);
	HookEvent("teamplay_overtime_begin", OvertimeStart);
	HookEvent("teamplay_overtime_end", OvertimeEnd);
	EBotDebug = CreateConVar("ebot_debug", "0", "", FCVAR_NONE);
	EBotFPS = CreateConVar("ebot_run_fps", "0.1", "0.0333 = 30 FPS | 0.05 = 20 FPS | 0.1 = 10 FPS | 0.2 = 5 FPS", FCVAR_NONE);
	EBotMelee = CreateConVar("ebot_melee_range", "128", "", FCVAR_NONE);
	EBotSenseMin = CreateConVar("ebot_minimum_sense_chance", "10", "Minimum 10", FCVAR_NONE);
	EBotSenseMax = CreateConVar("ebot_maximum_sense_chance", "90", "Maximum 90", FCVAR_NONE);
	EBotDifficulty = CreateConVar("ebot_difficulty", "4", "0 = Beginner | 1 = Easy | 2 = Normal | 3 = Hard | 4 = Expert", FCVAR_NONE);
	EBotMedicFollowRange = CreateConVar("ebot_medic_follow_range", "150", "", FCVAR_NONE);
	EBotChangeClass = CreateConVar("ebot_change_class", "1", "", FCVAR_NONE);
	EBotChangeClassRandom = CreateConVar("ebot_change_class_random", "0", "", FCVAR_NONE);
	EBotChangeClassChance = CreateConVar("ebot_change_class_chance", "35", "", FCVAR_NONE);
	EBotDeadChat = CreateConVar("ebot_dead_chat_chance", "30", "", FCVAR_NONE);
	EBotReplyToChat = CreateConVar("ebot_reply_to_chat_chance", "60", "", FCVAR_NONE);
	m_eBotDodgeRangeMin = CreateConVar("ebot_minimum_dodge_range", "512", "the range when enemy closer than a value, bot will start dodging enemies", FCVAR_NONE);
	m_eBotDodgeRangeMax = CreateConVar("ebot_maximum_dodge_range", "2048", "the range when enemy closer than a value, bot will start dodging enemies", FCVAR_NONE);
	m_eBotDodgeRangeChance = CreateConVar("ebot_dodge_change_range_chance", "10", "the chance for change dodge range when attack process ends (1-100)", FCVAR_NONE);
	EBotQuota = CreateConVar("ebot_quota", "-1", "", FCVAR_NONE);
	EBotAutoWaypoint = CreateConVar("ebot_waypoint_auto", "0", "", FCVAR_NONE);
	EBotRadius = CreateConVar("ebot_waypoint_default_radius", "0", "", FCVAR_NONE);
	EBotDistance = CreateConVar("ebot_waypoint_auto_distance", "250.0", "", FCVAR_NONE);
	EBotForceHuntEnemy = CreateConVar("ebot_force_hunt_enemy", "0", "", FCVAR_NONE);
	EBotAllowAttackButtons = CreateConVar("ebot_allow_attack_buttons", "1", "", FCVAR_NONE);
	EBotAFKPlayers = CreateConVar("ebot_afk_take_over_auto", "1", "", FCVAR_NOTIFY);
	EBotAFKCommand = CreateConVar("ebot_afk_by_command", "1", "", FCVAR_NONE);
	EBotAFKTime = CreateConVar("ebot_afk_time", "120", "", FCVAR_NONE);

	int i;
	for (i = 1; i <= MaxClients; i++)
	{
		if (!IsValidClient(i))
			continue;

		if (!IsFakeClient(i))
		{
			m_afkTime[i] = GetGameTime() + GetConVarFloat(EBotAFKTime);
			continue;
		}

		char namebuf[32];
    	GetEntPropString(i, Prop_Data, "m_iName", namebuf, sizeof(namebuf));

    	if (!StrEqual("ebot", namebuf))
			continue;

		SDKHook(i, SDKHook_OnTakeDamage, OnTakeDamage);
		m_spawnTime[i] = GetGameTime();
		m_difficulty[i] = -1;
		m_isEBot[i] = true;
		m_class[i] = TF2_GetPlayerClass(i);
		m_team[i] = GetClientTeam(i);
		m_isAlive[i] = IsPlayerAlive(i);
		m_ignoreEnemies[i] = 0.0;
	}
}

public void OnMapStart()
{
	isOT = false;
	g_seed = GetTime();
	spyTarget = FindConVar("tf_bot_spy_change_target_range_threshold");
	spyKnife = FindConVar("tf_bot_spy_knife_range");

	if (spyTarget == null)
		spyTarget = CreateConVar("tf_bot_spy_change_target_range_threshold", "300", "", FCVAR_CHEAT);

	if (spyKnife == null)
		spyKnife = CreateConVar("tf_bot_spy_knife_range", "300", "If threat is closer than this, prefer our knife", FCVAR_CHEAT);

	m_isAlive[0] = false;
	m_team[0] = -3;

	GetCurrentMap(currentMap, sizeof(currentMap));
	AutoLoadGamemode();

	SetConVarInt(FindConVar("nav_generate_fencetops"), 0);
	SetConVarInt(FindConVar("mp_waitingforplayers_cancel"), 1);
	InitGamedata();
	ServerCommand("sv_tags ebot");
	AddServerTag("ebot");
	m_laserIndex = PrecacheModel("materials/sprites/laserbeam.vmt");
	m_beamIndex = PrecacheModel("materials/sprites/lgtning.vmt");

	currentActiveAreaInt = GetBluControlPointCount();
	currentActiveArea = (1 << (currentActiveAreaInt + 1));
	healthpacks = 0;
	ammopacks = 0;
	BotCheckTimer = 0.0;
	autoupdate = 0.0;

	int x;
	for (x = 0; x <= GetMaxEntities(); x++)
	{
		if (IsValidHealthPack(x))
			healthpacks++;

		if (IsValidAmmoPack(x))
			ammopacks++;
	}

	if (isCTF)
	{
		int flag;
		while ((flag = FindEntityByClassname(flag, "item_teamflag")) != INVALID_ENT_REFERENCE)
		{
			if (IsValidEntity(flag))
			{
				if (GetTeamNumber(flag) == 2)
				{
					m_redFlag = flag;
					m_redFlagCapPoint = GetOrigin(flag);
				}
				else if (GetTeamNumber(flag) == 3)
				{
					m_bluFlag = flag;
					m_bluFlagCapPoint = GetOrigin(flag);
				}
			}
		}
	}

	if (isKOTH)
	{
		int iControlPoint;
		while ((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
		{
			if (!IsValidEntity(iControlPoint))
				continue;

			m_capturePoint = iControlPoint;
		}
	}

	InitializeWaypoints();
}

public Action OvertimeStart(Handle event, char[] name, bool dontBroadcast)
{
	isOT = true;
	if (GetConVarInt(EBotDebug) == 1)
		PrintHintTextToAll("Overtime is started!");
	return Plugin_Continue;
}

public Action OvertimeEnd(Handle event, char[] name, bool dontBroadcast)
{
	isOT = false;
	if (GetConVarInt(EBotDebug) == 1)
		PrintHintTextToAll("Overtime is over!");
	return Plugin_Continue;
}

public Action Command_Afk(const int client, int args)
{
	if (args != 0 && args != 2)
	{
		ReplyToCommand(client, "[E-BOT] Usage: sm_afk <target> [0/1]");
		return Plugin_Handled;
	}

	if (args == 0) // For People
	{
		if (GetConVarInt(EBotAFKCommand) != 1)
		{
			PrintToChat(client, "[E-BOT] AFK-BOT is disabled for players.");
			return Plugin_Handled;
		}

		if (!m_isAFK[client])
		{
			PrintToChat(client, "[E-BOT] AFK-BOT is enabled.");
			m_isAFK[client] = true;
			char name[32];
			GetClientName(client, name, sizeof(name));
			Format(name, sizeof(name), "%s (AFK)", name);
			SetClientName(client, name);
		}
		else
		{
			PrintToChat(client, "[E-BOT] AFK-BOT is disabled.");
			PrintCenterText(client, "Your AFK Mode is now disabled.");
			m_afkTime[client] = GetGameTime() + GetConVarFloat(EBotAFKTime);
			m_isAFK[client] = false;
			char name[32];
			GetClientName(client, name, sizeof(name));
			ReplaceString(name, sizeof(name), "(AFK)", "", true);
			ReplaceString(name, sizeof(name), "(AFK", "", true);
			ReplaceString(name, sizeof(name), "(AF", "", true);
			ReplaceString(name, sizeof(name), "(A", "", true);
			ReplaceString(name, sizeof(name), "(", "", true);
			SetClientName(client, name);
		}

		return Plugin_Handled;
	}
	else if (args == 2)
	{
		char arg1[PLATFORM_MAX_PATH];
		GetCmdArg(1, arg1, sizeof(arg1));
		char arg2[8];
		GetCmdArg(2, arg2, sizeof(arg2));

		int value = StringToInt(arg2);
		if (value != 0 && value != 1)
		{
			ReplyToCommand(client, "[E-BOT] Usage: sm_afk <target> [0/1]");
			return Plugin_Handled;
		}

		char target_name[MAX_TARGET_LENGTH];
		int target_list[TFMaxPlayers];
		int target_count;
		bool tn_is_ml;
		if ((target_count = ProcessTargetString(arg1, client, target_list, TFMaxPlayers, 0, target_name, sizeof(target_name), tn_is_ml)) <= 0)
		{
			ReplyToTargetError(client, target_count);
			return Plugin_Handled;
		}

		int i;
		for (i = 0; i < target_count; i++)
		{
			if (IsValidClient(target_list[i]))
			{
				if (value == 0)
				{
					if (CheckCommandAccess(client, "sm_afk_access", ADMFLAG_ROOT))
					{
						PrintToChat(target_list[i], "[E-BOT] AFK-BOT is disabled.");
						PrintCenterText(target_list[i], "Your AFK Mode is now disabled.");
						m_afkTime[target_list[i]] = GetGameTime() + GetConVarFloat(EBotAFKTime);
						m_isAFK[target_list[i]] = false;
					}
				}
				else
				{
					if (CheckCommandAccess(client, "sm_afk_access", ADMFLAG_ROOT))
					{
						PrintToChat(target_list[i], "[E-BOT] AFK-BOT is enabled.");
						m_isAFK[target_list[i]] = true;
					}
				}
			}
		}
	}

	return Plugin_Handled;
}

public void OnMapEnd()
{
	DangerMapSave();
	BotCheckTimer = 0.0;
}

public void EBotDeathChat(const int client)
{
	if (!ChanceOf(GetConVarInt(EBotDeadChat)))
		return;

	char filepath[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, filepath, sizeof(filepath), "ebot/chat/ebot_deadchat.txt");
    File fp = OpenFile(filepath, "r");
    if (fp != null)
    {
		ArrayList RandomChat = new ArrayList(ByteCountToCells(65));

		char line[MAX_NAME_LENGTH];
        while (!fp.EndOfFile())
		{
			fp.ReadLine(line, sizeof(line));
			if (!line)
				continue;

			if (StrContains(line, "//") != -1)
				continue;

			TrimString(line);
			RandomChat.PushString(line);
		}

		if (RandomChat.Length > 0)
		{
			char ChosenOne[MAX_NAME_LENGTH];
			RandomChat.GetString(crandomint(0, RandomChat.Length - 1), ChosenOne, sizeof(ChosenOne));
			DataPack pack;
			CreateDataTimer(crandomfloat(2.0, 7.0), ReplyToChat, pack);
			pack.WriteCell(client);
			pack.WriteString(ChosenOne);
		}

		fp.Close();
    }
}

char ChosenName[MAX_NAME_LENGTH];
public void OnClientPutInServer(int client)
{
	m_isEBot[client] = false;

	int ebot = FindBotByName(ChosenName);
	if (IsValidClient(ebot))
	{
		DispatchKeyValue(ebot, "targetname", "ebot");
		DispatchSpawn(ebot);
		ActivateEntity(ebot);

		m_isEBot[ebot] = true;

		FakeClientCommand(ebot, "jointeam auto");

		if (StrEqual("Engineer Gaming", ChosenName))
			FakeClientCommandEx(ebot, "joinclass engineer");
		else if (StrEqual("Rick May", ChosenName))
			FakeClientCommandEx(ebot, "joinclass soldier");
		else
			FakeClientCommandEx(ebot, "joinclass auto");

		m_aimInterval[ebot] = GetGameTime();

		int i, j;
    	for (i = 0; i < m_waypointNumber; i++)
		{
			for (j = 0; j < 7; j++)
			{
				damageWPTred[j][ebot][i] = damageGLOBALred[j][i];
				damageWPTblu[j][ebot][i] = damageGLOBALblu[j][i];
			}
		}
	}

	if (IsValidClient(client) && IsFakeClient(client))
		SDKHook(client, SDKHook_OnTakeDamage, OnTakeDamage);

	// delete path positions
	delete m_positions[client];
	m_positions[client] = new ArrayList(3);
	delete m_pathIndex[client];
	m_pathIndex[client] = new ArrayList();
	delete m_hidingSpots[client];
	m_hidingSpots[client] = new ArrayList();
	m_goalPosition[client] = NULL_VECTOR;
	m_goalEntity[client] = -1;
	m_nextStuckCheck[client] = GetGameTime() + 5.0;
	m_enemiesNearCount[client] = 0;
	m_friendsNearCount[client] = 0;
	m_hasEnemiesNear[client] = false;
	m_hasFriendsNear[client] = false;
	m_lowAmmo[client] = false;
	m_lowHealth[client] = false;
	DJTime[client] = 0.0;
	m_attack2Timer[client] = 0.0;
	m_attackTimer[client] = 0.0;
	m_duckTimer[client] = 0.0;
	m_thinkTimer[client] = 0.0;
	m_checkTimer[client] = 0.0;
	m_wasdTimer[client] = 0.0;
	m_knownSentry[client] = -1;
	m_enterFail[client] = false;
	m_exitFail[client] = false;
	CurrentProcessTime[client] = 0.0;
	CurrentProcess[client] = PRO_DEFAULT;
	m_damageTime[client] = 0.0;
	m_spawnTime[client] = GetGameTime();
	m_isAlive[client] = false;
	m_team[client] = -3;
	m_ignoreEnemies[client] = 0.0;

	if (GetConVarInt(EBotDifficulty) < 0 || GetConVarInt(EBotDifficulty) > 4)
		m_difficulty[client] = crandomint(0, 4);
	else
		m_difficulty[client] = GetConVarInt(EBotDifficulty);

	// check host entity
	if (m_hasHostEntity == false)
		FindHostEntity();
}

public Action WaypointOn(const int client, int args)
{
	showWaypoints = true;
	PrintHintTextToAll("Waypoints are now enabled");
	return Plugin_Handled;
}

public Action WaypointOff(const int client, int args)
{
	showWaypoints = false;
	PrintHintTextToAll("Waypoints are now disabled");
	return Plugin_Handled;
}

public Action WaypointCreate(const int client, int args)
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

public Action WaypointDelete(const int client, int args)
{
	if (showWaypoints)
	{
		if (nearestIndex != -1 && GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) < Squared(64))
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

public Action SaveWaypoint(const int client, int args)
{
	WaypointSave();
	return Plugin_Handled;
}

public Action SaveWaypointNAV(const int client, int args)
{
	WaypointSave(true);
	return Plugin_Handled;
}

public Action SetRadius(const int client, int args)
{
	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) < Squaredf(64.0))
	{
		char radius[32];
		GetCmdArgString(radius, sizeof(radius));
		m_paths[nearestIndex].radius = float(StringToInt(radius));
		if (m_paths[nearestIndex].radius < 0.0)
			m_paths[nearestIndex].radius = 0.0;
		PrintHintTextToAll("Current radius is %d", RoundFloat(m_paths[nearestIndex].radius));
	}
	else
		PrintHintTextToAll("Move closer to waypoint");

	return Plugin_Handled;
}

public Action AddRadius(const int client, int args)
{
	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) < Squaredf(64.0))
	{
		m_paths[nearestIndex].radius += 16.0;
		if (m_paths[nearestIndex].radius > 128.0)
			m_paths[nearestIndex].radius = 0.0;

		PrintHintTextToAll("Current radius is %d", RoundFloat(m_paths[nearestIndex].radius));
	}
	else
		PrintHintTextToAll("Move closer to waypoint");

	return Plugin_Handled;
}

public Action SetFlag(const int client, int args)
{

	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) < Squaredf(64.0))
	{
		char flag[32];
		GetCmdArgString(flag, sizeof(flag));

		int flagone = StringToInt(flag);
		if (flagone <= 0)
		{
			m_paths[nearestIndex].flags = 0;
			PrintHintTextToAll("Waypoint Flag Set: None");
			return Plugin_Handled;
		}

		int intflag = (1 << flagone);
		if (intflag & WAYPOINT_DEFEND || intflag & WAYPOINT_SENTRY || intflag & WAYPOINT_DEMOMANCAMP || intflag & WAYPOINT_SNIPER || intflag & WAYPOINT_TELEPORTERENTER || intflag & WAYPOINT_TELEPORTEREXIT)
		{
			float origin[3];
			GetAimOrigin(m_hostEntity, origin);
			m_paths[nearestIndex].campStart = origin;
			m_paths[nearestIndex].campEnd = origin;
		}

		m_paths[nearestIndex].flags = intflag;
		PrintHintTextToAll("Waypoint Flag Set: %s", GetWaypointName(intflag));
	}
	else
		PrintHintTextToAll("Move closer to waypoint");

	return Plugin_Handled;
}

public Action AddFlag(const int client, int args)
{
	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) < Squaredf(64.0))
	{
		char flag[32];
		GetCmdArgString(flag, sizeof(flag));

		int value = StringToInt(flag);
		if (value <= 0)
		{
			PrintHintTextToAll("You cannot add this number");
			return Plugin_Handled;
		}

		int intflag = (1 << value);
		if (intflag & WAYPOINT_DEFEND || intflag & WAYPOINT_SENTRY || intflag & WAYPOINT_DEMOMANCAMP || intflag & WAYPOINT_SNIPER || intflag & WAYPOINT_TELEPORTERENTER || intflag & WAYPOINT_TELEPORTEREXIT)
		{
			float origin[3];
			GetAimOrigin(m_hostEntity, origin);

			if (IsNullVector(m_paths[nearestIndex].campStart))
				m_paths[nearestIndex].campStart = origin;

			if (IsNullVector(m_paths[nearestIndex].campEnd))
				m_paths[nearestIndex].campEnd = origin;
		}

		m_paths[nearestIndex].flags |= intflag;
		PrintHintTextToAll("Waypoint Flag Added: %s", GetWaypointName(intflag));
	}
	else
		PrintHintTextToAll("Move closer to waypoint");

	return Plugin_Handled;
}

public Action RemoveFlag(const int client, int args)
{
	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) < Squaredf(64.0))
	{
		char flag[32];
		GetCmdArgString(flag, sizeof(flag));

		int value = StringToInt(flag);
		if (value <= 0)
		{
			PrintHintTextToAll("You cannot remove this number");
			return Plugin_Handled;
		}

		int intflag = (1 << value);
		m_paths[nearestIndex].flags &= ~intflag;
		PrintHintTextToAll("Waypoint Flag Removed: %s", GetWaypointName(intflag));
	}
	else
		PrintHintTextToAll("Move closer to waypoint");

	return Plugin_Handled;
}

public Action SetTeam(const int client, int args)
{
	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) < Squaredf(64.0))
	{
		char team[32];
		GetCmdArgString(team, sizeof(team));
		m_paths[nearestIndex].team = StringToInt(team);
		PrintHintTextToAll("Waypoint Team Changed: %d", view_as<TFTeam>(StringToInt(team)));
	}
	else
		PrintHintTextToAll("Move closer to waypoint");

	return Plugin_Handled;
}

public Action Select(const int client, int args)
{
	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) < Squaredf(64.0))
	{
		savedIndex = nearestIndex;
		PrintHintTextToAll("Waypoint selected %d", savedIndex);
	}
	else
		PrintHintTextToAll("Move closer to waypoint");

	return Plugin_Handled;
}

public Action PathAdd(const int client, int args)
{
	if (savedIndex == -1)
		PrintHintTextToAll("Select a waypoint first");
	else if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) < Squaredf(64.0))
	{
		AddPath(savedIndex, nearestIndex, GetVectorDistance(m_paths[savedIndex].origin, m_paths[nearestIndex].origin));
		savedIndex = -1;
	}
	else
		PrintHintTextToAll("Move closer to waypoint");

	return Plugin_Handled;
}

public Action PathDelete(const int client, int args)
{
	if (savedIndex == -1)
		PrintHintTextToAll("Select a waypoint first");
	else if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) <= Squaredf(64.0))
	{
		DeletePath(savedIndex, nearestIndex);
		savedIndex = -1;
	}
	else
		PrintHintTextToAll("Move closer to waypoint");

	return Plugin_Handled;
}

public Action SetArea(const int client, int args)
{
	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) < Squaredf(64.0))
	{
		char area[32];
		GetCmdArgString(area, sizeof(area));

		int value = StringToInt(area);
		if (value <= 0)
		{
			m_paths[nearestIndex].activeArea = 0;
			PrintHintTextToAll("Waypoint Area Set: All Time");
		}
		else
		{
			m_paths[nearestIndex].activeArea = (1 << value);
			PrintHintTextToAll("Waypoint Area Set: %s", GetAreaName((1 << value)));
		}
	}
	else
		PrintHintTextToAll("Move closer to waypoint");

	return Plugin_Handled;
}

public Action AddArea(const int client, int args)
{
	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) < Squaredf(64.0))
	{
		char area[32];
		GetCmdArgString(area, sizeof(area));

		int value = StringToInt(area);
		if (value <= 0)
		{
			PrintHintTextToAll("You cannot add this number");
			return Plugin_Handled;
		}

		int intarea = (1 << value);
		m_paths[nearestIndex].activeArea |= intarea;
		PrintHintTextToAll("Waypoint Area Added: %s", GetAreaName(intarea));
	}
	else
		PrintHintTextToAll("Move closer to waypoint");

	return Plugin_Handled;
}

public Action RemoveArea(const int client, int args)
{
	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) < Squaredf(64.0))
	{
		char area[32];
		GetCmdArgString(area, sizeof(area));

		int value = StringToInt(area);
		if (value <= 0)
		{
			PrintHintTextToAll("You cannot remove this number");
			return Plugin_Handled;
		}

		int intarea = (1 << value);
		m_paths[nearestIndex].activeArea &= ~intarea;
		PrintHintTextToAll("Waypoint Area Removed: %s", GetAreaName(intarea));
	}
	else
		PrintHintTextToAll("Move closer to waypoint");

	return Plugin_Handled;
}

public Action SetAim1(const int client, int args)
{
	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) < Squaredf(64.0))
	{
		float origin[3];
		GetAimOrigin(m_hostEntity, origin);
		m_paths[nearestIndex].campStart = origin;
		PrintHintTextToAll("Waypoint Set Aim Start: %d %d %d", RoundFloat(m_paths[nearestIndex].campStart[0]), RoundFloat(m_paths[nearestIndex].campStart[1]), RoundFloat(m_paths[nearestIndex].campStart[2]));
	}
	else
		PrintHintTextToAll("Move closer to waypoint");

	return Plugin_Handled;
}

public Action SetAim2(const int client, int args)
{
	if (GetVectorDistance(GetOrigin(m_hostEntity), m_paths[nearestIndex].origin, true) < Squaredf(64.0))
	{
		float origin[3];
		GetAimOrigin(m_hostEntity, origin);
		m_paths[nearestIndex].campEnd = origin;
		PrintHintTextToAll("Waypoint Set Aim End: %d %d %d", RoundFloat(m_paths[nearestIndex].campEnd[0]), RoundFloat(m_paths[nearestIndex].campEnd[1]), RoundFloat(m_paths[nearestIndex].campEnd[2]));
	}
	else
		PrintHintTextToAll("Move closer to waypoint");

	return Plugin_Handled;
}

public Action AddEBot(const int client, int args)
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
		char line[MAX_NAME_LENGTH];
        while (!fp.EndOfFile())
		{
			fp.ReadLine(line, sizeof(line));
			if (!line)
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

	if (BotNames.Length > 0)
		BotNames.GetString(crandomint(0, BotNames.Length - 1), ChosenName, sizeof(ChosenName));
	else
		FormatEx(ChosenName, sizeof(ChosenName), "ebot %d", crandomint(0, 9999));

	if (crandomint(1, 10) == 1 && !NameAlreadyTakenByPlayer("Rick May"))
		FormatEx(ChosenName, sizeof(ChosenName), "Rick May");
	else if (crandomint(1, 10) == 1 && !NameAlreadyTakenByPlayer("Engineer Gaming"))
		FormatEx(ChosenName, sizeof(ChosenName), "Engineer Gaming");

	ConVar cheats = FindConVar("sv_cheats");
	if (cheats != null)
	{
		if (cheats.BoolValue)
			ServerCommand("bot -name \"%s\"", ChosenName);
		else
		{
			int flags = cheats.Flags;
			flags &= ~FCVAR_NOTIFY;
			cheats.Flags = flags;
			cheats.SetBool(true, false, false);
			ServerCommand("bot -name \"%s\"", ChosenName);
			CreateTimer(GetGameFrameTime() * 2.0, SetCheats);
		}
	}
	else
		ServerCommand("bot -name \"%s\"", ChosenName);
}

public Action KickEBot(const int client, int args)
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
		EBotTeam = crandomint(2, 3);

	int i;
	for (i = 1; i <= MaxClients; i++)
	{
		if (!IsValidClient(i))
			continue;

		if (!IsEBot(i))
			continue;

		if (GetClientTeam(i) != EBotTeam)
			continue;

		KickClient(i);
		m_isEBot[i] = false;
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
			if (total < wanted)
				AddEBotConsole();
			else if (total > wanted)
				KickEBotConsole();
			else
			{
				int red = GetPlayersCountRed();
				int blu = GetPlayersCountBlu();

				if ((red + 1) < blu)
					KickEBotConsole();
				else if ((blu + 1) < red)
					KickEBotConsole();
			}

			BotCheckTimer = GetGameTime() + 1.0;
		}
	}

	if (!isArena && !isKOTH && autoupdate < GetGameTime())
	{
		currentActiveAreaInt = GetBluControlPointCount();
		int search = (1 << (currentActiveAreaInt + 1));
		if (search != currentActiveArea)
		{
			currentActiveArea = search;

			if (GetConVarInt(EBotDebug) == 1)
				PrintHintTextToAll("Active Area: %s", GetAreaName(currentActiveArea));

			for (search = 1; search <= MaxClients; search++)
			{
				if (!IsValidClient(search))
					continue;

				if (!IsEBot(search))
					continue;

				if (CurrentProcess[search] != PRO_DEFAULT)
					continue;

				SelectObjective(search);
				DeletePathNodes(search);
			}
		}

		autoupdate = GetGameTime() + crandomfloat(3.0, 9.0);
	}

	DrawWaypoints();
	AutoWaypoint();

	if (GetConVarInt(EBotDebug) == 1)
	{
		if (!IsValidClient(m_hostEntity))
		{
			FindHostEntity();
			return;
		}

		if (hudtext < GetGameTime())
		{
			int bot = GetEntPropEnt(m_hostEntity, Prop_Send, "m_hObserverTarget");
			if (IsValidClient(bot) && IsFakeClient(bot))
			{
				SetHudTextParams(0.0, -1.0, 0.52, 255, 255, 255, 255, 2, 1.0, 0.0, 0.0);
        		ShowHudText(m_hostEntity, -1, "Goal Index: %d\nGoal Pos: %d %d %d\nGoal Ent: %d\nEnemy Near: %s\nFriend Near: %s\nEnt Near: %s\nSense: %d\nCurrent Process: %s\nRemembered Process: %s\nDiff: %d\nMax DMG: RED - %d | BLU - %d",
				m_goalIndex[bot],
				RoundFloat(m_goalPosition[bot][0]),
				RoundFloat(m_goalPosition[bot][1]),
				RoundFloat(m_goalPosition[bot][2]),
				m_goalEntity[bot],
				m_hasEnemiesNear[bot] ? "Yes" : "No",
				m_hasFriendsNear[bot] ? "Yes" : "No",
				m_hasEntitiesNear[bot] ? "Yes" : "No",
				m_eBotSenseChance[bot],
				GetProcessName(CurrentProcess[bot], bot),
				GetProcessName(RememberedProcess[bot], bot),
				m_difficulty[bot],
				RoundFloat(m_maxDamageRed[bot]),
				RoundFloat(m_maxDamageBlu[bot]));

				int i;
				for (i = m_targetNode[bot]; i > 0; i--)
				{
					if ((i - 1) < 0 || i > m_positions[bot].Length)
						continue;

					float flFromPos[3];
					float flToPos[3];
					m_positions[bot].GetArray(i, flFromPos, 3);
					m_positions[bot].GetArray(i - 1, flToPos, 3);
					TE_SetupBeamPoints(flFromPos, flToPos, m_laserIndex, m_laserIndex, 0, 30, 0.5, 1.0, 1.0, 5, 0.0, {0, 255, 0, 255}, 30);
					TE_SendToClient(m_hostEntity);
				}

				hudtext = GetGameTime() + 0.5;
			}
		}
	}

	if (GetConVarFloat(EBotFPS) > 0.2)
		SetConVarFloat(EBotFPS, 0.2);

	if (GetConVarFloat(EBotFPS) < 0.0333)
		SetConVarFloat(EBotFPS, 0.0333);
}

public Action OnPlayerRunCmd(int client, &buttons, &impulse, float vel[3], float angles[3])
{
	if (!IsClientInGame(client))
		return Plugin_Continue;

	if (!IsEBot(client) && !m_isAFK[client])
	{
		m_class[client] = TF2_GetPlayerClass(client);
		m_isAlive[client] = IsPlayerAlive(client);
		m_team[client] = GetClientTeam(client);

		if (!IsFakeClient(client) && GetConVarInt(EBotAFKPlayers) == 1)
		{
			if (m_team[client] == 2 || m_team[client] == 3)
			{
				if (IsAttacking(client) || IsMoving(client))
					m_afkTime[client] = GetGameTime() + GetConVarFloat(EBotAFKTime);
				else if (m_afkTime[client] < GetGameTime())
				{
					m_isAFK[client] = true;
					char name[32];
					GetClientName(client, name, sizeof(name));
					Format(name, sizeof(name), "%s (AFK)", name);
					SetClientName(client, name);
				}
			}
		}

		return Plugin_Continue;
	}

	// auto backstab
	if (IsWeaponSlotActive(client, 2))
	{
		static int melee;
		melee = GetPlayerWeaponSlot(client, TFWeaponSlot_Melee);
		if (IsValidEntity(melee) && HasEntProp(melee, Prop_Send, "m_bReadyToBackstab"))
		{
			if (GetEntProp(melee, Prop_Send, "m_bReadyToBackstab"))
				buttons |= IN_ATTACK;
			else
				buttons &= ~IN_ATTACK;
		}
	}

	// trigger scout double jump
	if (DJTime[client] < GetGameTime())
	{
		if (m_positions[client] && m_positions[client].Length > 0 && m_targetNode[client] != -1)
		{
			float flGoPos[3];
			m_positions[client].GetArray(m_targetNode[client], flGoPos);
			MoveTo(client, flGoPos);
		}

		buttons |= IN_DUCK;
		buttons |= IN_JUMP;
		DJTime[client] = GetGameTime() + 999999.0;
	}
	else if (CrouchTime[client] > GetGameTime())
		buttons |= IN_DUCK;

	// check if client pressing any buttons and set them
	if (m_buttons[client] != 0)
	{
		buttons |= m_buttons[client];
		m_buttons[client] = 0;
	}

	// is client alive? if not, we can't do anything
	m_isAlive[client] = IsPlayerAlive(client);
	if (m_isAlive[client])
	{
		if (GetConVarBool(EBotAllowAttackButtons))
		{
			if (m_attackTimer[client] > GetGameTime())
				buttons |= IN_ATTACK;

			if (m_attack2Timer[client] > GetGameTime())
				buttons |= IN_ATTACK2;
		}

		if (m_duckTimer[client] > GetGameTime())
			buttons |= IN_DUCK;
		else if (GetEntProp(client, Prop_Send, "m_bJumping"))
			buttons |= IN_DUCK;

		if (m_thinkTimer[client] < GetGameTime())
		{
			if (CurrentProcess[client] != PRO_ATTACK)
			{
				m_moveVel[client][0] = 0.0;
				m_moveVel[client][1] = 0.0;
				m_moveVel[client][2] = 0.0;
			}

			m_origin[client] = GetOrigin(client);
			m_eyeOrigin[client] = GetEyePosition(client);

			CheckSlowThink(client);
			ThinkAI(client);
			m_thinkTimer[client] = GetGameTime() + GetConVarFloat(EBotFPS);
		}
		else
		{
			if (m_class[client] != TFClass_Engineer && m_class[client] != TFClass_Medic && m_class[client] != TFClass_Spy)
			{
				if (m_hasEnemiesNear[client] && m_hasEntitiesNear[client] && IsValidEntity(m_nearestEntity[client]))
				{
					static float center[3];
					center = GetCenter(m_nearestEntity[client]);

					if (m_enemyDistance[client] < GetVectorDistance(GetOrigin(client), center, true))
						LookAtEnemiens(client);
					else
						m_lookAt[client] = center;

					if (CurrentProcess[client] != PRO_SPYSAP)
					{
						AutoAttack(client);
						SelectBestCombatWeapon(client);
					}

					m_pauseTime[client] = GetGameTime() + crandomfloat(1.5, 2.5);
				}
				else if (m_hasEnemiesNear[client])
				{
					LookAtEnemiens(client);
					AutoAttack(client);
					SelectBestCombatWeapon(client);
					m_pauseTime[client] = GetGameTime() + crandomfloat(1.5, 2.5);
				}
				else if (m_hasEntitiesNear[client] && IsValidEntity(m_nearestEntity[client]))
				{
					m_lookAt[client] = GetCenter(m_nearestEntity[client]);

					if (CurrentProcess[client] != PRO_SPYSAP)
					{
						AutoAttack(client);
						SelectBestCombatWeapon(client);
					}

					m_pauseTime[client] = GetGameTime() + crandomfloat(1.5, 2.5);
				}
			}

			LookAtPosition(client, m_lookAt[client], angles);

			if (m_duckTimer[client] < GetGameTime())
			{
				static int nOldButtons;
				nOldButtons = GetEntProp(client, Prop_Data, "m_nOldButtons");
				if (nOldButtons & (IN_JUMP|IN_DUCK))
					SetEntProp(client, Prop_Data, "m_nOldButtons", (nOldButtons &= ~(IN_JUMP|IN_DUCK)));
			}

			if (CurrentProcess[client] == PRO_BUILDDISPENSER && m_isSlowThink[client] && crandomint(1, 3) == 1)
			{
				if (m_hasWaypoints)
				{
					m_lookAt[client] = m_paths[crandomint(1, m_waypointNumber)].origin;
					m_lookAt[client][2] = GetHeight(client);
				}
				else
				{
					m_lookAt[client][0] = crandomfloat(1.0, 359.0);
					m_lookAt[client][1] = crandomfloat(1.0, 359.0);
					m_lookAt[client][2] = crandomfloat(1.0, 359.0);
				}

				// force deploy
				buttons |= IN_ATTACK;
			}
		}

		if (TF2_IsPlayerInCondition(client, TFCond_Charging))
			buttons &= ~IN_ATTACK;

		vel = m_moveVel[client];

		if (m_currentIndex[client] > 0 && m_paths[m_currentIndex[client]].flags & WAYPOINT_DEMOCHARGE && m_class[client] == TFClass_DemoMan)
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
			m_stopTime[client] = GetGameTime() + 3.0;
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
	static int client;
	client = GetClientOfUserId(GetEventInt(event, "userid"));
	if (!IsValidClient(client))
		return Plugin_Handled;

	m_afkTime[client] = GetGameTime() + 60.0;

	if (IsEBot(client) || m_isAFK[client])
	{
		static int i;
		static int j;
		static float totalRed;
		totalRed = 0.0;
		static float totalBlu;
		totalBlu = 0.0;

		for (i = 0; i < m_waypointNumber; i++)
		{
			for (j = 0; j < 7; j++)
			{
				totalRed += damageWPTred[j][client][i];
				totalBlu += damageWPTblu[j][client][i];
			}

			totalRed += damageWPTred[currentActiveAreaInt][client][i];
			totalBlu += damageWPTblu[currentActiveAreaInt][client][i];
		}

		m_maxDamageRed[client] = (totalRed / m_waypointNumber) * GetKDR(client);
		m_maxDamageBlu[client] = (totalBlu / m_waypointNumber) * GetKDR(client);

		SetFakeClientConVar(client, "cl_autowepswitch", "1");
		SetFakeClientConVar(client, "hud_fastswitch", "1");
		SetFakeClientConVar(client, "tf_medigun_autoheal", "0");
		SetFakeClientConVar(client, "cl_autoreload", "1");

		static int team;
		team = GetClientTeam(client);
		if (GetConVarBool(EBotChangeClass))
		{
			static int rand;
			rand = crandomint(1, 100);
			if (rand <= GetConVarInt(EBotChangeClassChance) && client != FindLeader(team) && client != FindBestKD(team) && (TF2_GetPlayerClass(client) != TFClass_Engineer || (!IsValidEntity(SentryGun[client]) && !IsValidEntity(TeleporterExit[client]))))
			{
				static char nickname[32];
       		 	GetClientName(client, nickname, sizeof(nickname))
        		if (!StrEqual("Rick May", nickname) && !StrEqual("Engineer Gaming", nickname))
				{
					if (GameRules_GetProp("m_bPlayingMedieval"))
					{
						rand = crandomint(1, 5);
						if (rand == 1)
							TF2_SetPlayerClass(client, TFClass_DemoMan);
						else if (rand == 2)
							TF2_SetPlayerClass(client, TFClass_Spy);
						else if (rand == 3)
							TF2_SetPlayerClass(client, TFClass_Sniper);
						else if (rand == 4)
						{
							if (team == 3)
							{
								if (currentActiveArea & AREA1)
									TF2_SetPlayerClass(client, TFClass_Sniper);
								else
									TF2_SetPlayerClass(client, TFClass_DemoMan);
							}
							else
							{
								rand = crandomint(1, 2);
								if (rand == 1)
									TF2_SetPlayerClass(client, TFClass_Sniper);
								else
									TF2_SetPlayerClass(client, TFClass_Spy);
							}
						}
						else
						{
							rand = crandomint(1, 2);
							if (rand == 1)
								TF2_SetPlayerClass(client, TFClass_Medic);
							else
							{
								rand = crandomint(1, 3);
								if (rand == 1)
									TF2_SetPlayerClass(client, TFClass_Heavy);
								else if (rand == 2)
									TF2_SetPlayerClass(client, TFClass_Pyro);
								else
								{
									rand = crandomint(1, 2);
									if (rand == 1)
										TF2_SetPlayerClass(client, TFClass_Scout);
									else
										TF2_SetPlayerClass(client, TFClass_Soldier);
								}
							}
							}
					}
					else
					{
						if (!GetConVarBool(EBotChangeClassRandom) && crandomint(1, 2) == 1 && IsValidClient(m_lastKiller[client]))
						{
							if (m_class[m_lastKiller[client]] == TFClass_Scout)
							{
								rand = crandomint(1, 3);
								if (rand == 1)
									TF2_SetPlayerClass(client, TFClass_Pyro);
								else if (rand == 2)
								{
									if (IsOnDefense(client))
										TF2_SetPlayerClass(client, TFClass_Engineer);
									else
									{
										rand = crandomint(1, 3);
										if (rand == 1)
											TF2_SetPlayerClass(client, TFClass_Engineer);
										else if (rand == 2)
											TF2_SetPlayerClass(client, TFClass_Medic);
										else
											TF2_SetPlayerClass(client, TFClass_Heavy);
									}
								}
								else
									TF2_SetPlayerClass(client, TFClass_Heavy);
							}
							else if (m_class[m_lastKiller[client]] == TFClass_Soldier)
							{
								rand = crandomint(1, 3);
								if (rand == 1)
									TF2_SetPlayerClass(client, TFClass_Pyro);
								else if (rand == 2)
									TF2_SetPlayerClass(client, TFClass_Scout);
								else
									TF2_SetPlayerClass(client, TFClass_Sniper);
							}
							else if (m_class[m_lastKiller[client]] == TFClass_Pyro)
							{
								rand = crandomint(1, 3);
								if (rand == 1)
									TF2_SetPlayerClass(client, TFClass_Sniper);
								else if (rand == 2)
								{
									if (IsOnDefense(client))
										TF2_SetPlayerClass(client, TFClass_Engineer);
									else
									{
										rand = crandomint(1, 3);
										if (rand == 1)
											TF2_SetPlayerClass(client, TFClass_Engineer);
										else if (rand == 2)
											TF2_SetPlayerClass(client, TFClass_Sniper);
										else
											TF2_SetPlayerClass(client, TFClass_Heavy);
									}
								}
								else
									TF2_SetPlayerClass(client, TFClass_Heavy);
							}
							else if (m_class[m_lastKiller[client]] == TFClass_DemoMan)
							{
								rand = crandomint(1, 3);
								if (rand == 1)
									TF2_SetPlayerClass(client, TFClass_Soldier);
								else if (rand == 2)
									TF2_SetPlayerClass(client, TFClass_Scout);
								else
									TF2_SetPlayerClass(client, TFClass_Pyro);
							}
							else if (m_class[m_lastKiller[client]] == TFClass_Heavy)
							{
								rand = crandomint(1, 3);
								if (rand == 1)
									TF2_SetPlayerClass(client, TFClass_Spy);
								else if (rand == 2)
									TF2_SetPlayerClass(client, TFClass_Sniper);
								else
									TF2_SetPlayerClass(client, TFClass_DemoMan);
							}
							else if (m_class[m_lastKiller[client]] == TFClass_Engineer)
							{
								rand = crandomint(1, 5);
								if (rand == 1)
									TF2_SetPlayerClass(client, TFClass_Spy);
								else if (rand == 2)
									TF2_SetPlayerClass(client, TFClass_Medic);
								else if (rand == 3)
									TF2_SetPlayerClass(client, TFClass_Heavy);
								else if (rand == 4)
									TF2_SetPlayerClass(client, TFClass_DemoMan);
								else
									TF2_SetPlayerClass(client, TFClass_Sniper);
							}
							else if (m_class[m_lastKiller[client]] == TFClass_Sniper)
							{
								rand = crandomint(1, 3);
								if (rand == 1)
									TF2_SetPlayerClass(client, TFClass_Scout);
								else if (rand == 2)
									TF2_SetPlayerClass(client, TFClass_Spy);
								else
									TF2_SetPlayerClass(client, TFClass_Sniper);
							}
							else if (m_class[m_lastKiller[client]] == TFClass_Medic)
							{
								rand = crandomint(1, 3);
								if (rand == 1)
									TF2_SetPlayerClass(client, TFClass_Pyro);
								else if (rand == 2)
									TF2_SetPlayerClass(client, TFClass_Scout);
								else
									TF2_SetPlayerClass(client, TFClass_Spy);
							}
							else if (m_class[m_lastKiller[client]] == TFClass_Spy)
							{
								rand = crandomint(1, 3);
								if (rand == 1)
									TF2_SetPlayerClass(client, TFClass_Pyro);
								else if (rand == 2)
									TF2_SetPlayerClass(client, TFClass_Scout);
								else
									TF2_SetPlayerClass(client, TFClass_DemoMan);
							}
						}
						else
							FakeClientCommand(client, "joinclass auto");
					}

					if (IsPlayerAlive(client))
					{
						TF2_RespawnPlayer(client);
						DeletePathNodes(client);
						CurrentProcess[client] = PRO_DEFAULT;
						RememberedProcess[client] = PRO_DEFAULT;
					}
				}
			}
		}

		delete m_positions[client];
		m_positions[client] = new ArrayList(3);
		delete m_hidingSpots[client];
		m_hidingSpots[client] = new ArrayList();
		m_enemiesNearCount[client] = 0;
		m_friendsNearCount[client] = 0;
		m_hasEnemiesNear[client] = false;
		m_hasFriendsNear[client] = false;
		m_lowAmmo[client] = false;
		m_lowHealth[client] = false;
		m_knownSpy[client] = -1;
		m_goalIndex[client] = -1;
		m_goalEntity[client] = -1;
		m_goalPosition[client] = NULL_VECTOR;
		m_lastFailedWaypoint[client] = -1;
		m_currentIndex[client] = -1;
		m_lastEnemySeen[client] = 0.0;
		m_lastEntitySeen[client] = 0.0;
		m_stopTime[client] = 0.0;
		m_pauseTime[client] = 0.0;
		m_ignoreEnemies[client] = 0.0;
		CrouchTime[client] = 0.0;
		m_aimInterval[client] = GetGameTime();
		DeletePathNodes(client);

		if (TF2_GetPlayerClass(client) == TFClass_Engineer)
		{
			m_enterFail[client] = false;
			m_exitFail[client] = false;
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
			ArrayList RouteWaypoints = new ArrayList();
			for (i = 0; i < m_waypointNumber; i++)
			{
				if (!(m_paths[i].flags & WAYPOINT_ROUTE))
					continue;

				// blocked waypoint
   				if (m_paths[i].activeArea != 0 && !(m_paths[i].activeArea & currentActiveArea))
					continue;

				if (m_lastFailedWaypoint[client] == i)
					continue;

    			// not for our team
 				if (m_team[client] == 3 && m_paths[i].team == 2)
					continue;

   				if (m_team[client] == 2 && m_paths[i].team == 3)
					continue;

				RouteWaypoints.Push(i);
			}

			if (RouteWaypoints.Length > 0)
				i = RouteWaypoints.Get(crandomint(0, RouteWaypoints.Length - 1));
			else
				i = -1;

			delete RouteWaypoints;
			if (i != -1)
			{
				AStarFindPath(-1, i, client, m_paths[i].origin);
				m_nextStuckCheck[client] = GetGameTime() + 10.0;
			}
		}

		m_nextStuckCheck[client] = GetGameTime() + 10.0;
		m_spawnTime[client] = GetGameTime();
		m_class[client] = TF2_GetPlayerClass(client);
		m_team[client] = GetClientTeam(client);

		m_teleporterEntity[client] = -1;
		if (ChanceOf(60))
			m_useTeleporter[client] = true;
		else
			m_useTeleporter[client] = false;
	}

	return Plugin_Handled;
}

// trigger when bot spawns
public Action BotDeath(Handle event, char[] name, bool dontBroadcast)
{
	static int victim;
	static int client;
	victim = GetClientOfUserId(GetEventInt(event, "userid"));
	client = GetClientOfUserId(GetEventInt(event, "attacker"));

	if (IsValidClient(victim) && IsValidClient(client))
	{
		if (IsEBot(victim) || m_isAFK[victim])
		{
			CurrentProcess[victim] = PRO_DEFAULT;
			RememberedProcess[victim] = PRO_DEFAULT;
			m_lastKiller[victim] = client;
			EBotDeathChat(victim);
		}

		if (IsEBot(client) || m_isAFK[client])
		{
			if (m_class[client] == TFClass_Spy)
				TF2_DisguisePlayer(client, (GetClientTeam(client) == 2 ? TFTeam_Blue : TFTeam_Red), m_class[victim], victim);

			if (crandomint(1, 3) == 1 && !m_lowHealth[client] && !m_hasEntitiesNear[client])
			{
				FindFriendsAndEnemiens(client);
				if (!m_hasEnemiesNear[client] && (!m_hasFriendsNear[client] || m_class[client] == TFClass_Sniper))
					PlayTaunt(client, 463);
			}
		}
	}

	return Plugin_Handled;
}

public Action BotSap(Handle event, char[] name, bool dontBroadcast)
{
	static int spy;
	static int engineer;
	spy = GetClientOfUserId(GetEventInt(event, "userid"));
	engineer = GetClientOfUserId(GetEventInt(event, "ownerid"));

	if (IsValidClient(engineer) && (IsEBot(engineer) || m_isAFK[engineer]))
	{
		FakeClientCommand(engineer, "voicemenu 1 1");
		int index = FindWaypointFastest(GetOrigin(engineer));
		if (index > 0)
			AStarFindShortestPath(-1, index, engineer, m_paths[index].origin);

		m_knownSpy[engineer] = spy;
	}

	if (IsValidClient(spy))
	{
		int search;
		for (search = 1; search <= MaxClients; search++)
		{
			if (IsValidClient(search) && m_isAlive[search] && m_team[spy] != m_team[search] && IsVisible(GetEyePosition(spy), GetEyePosition(search)))
				m_knownSpy[search] = spy;
		}
	}

	return Plugin_Handled;
}

public Action OnTakeDamage(victim, &attacker, &inflictor, &Float:damage, &damagetype)
{
	if (!IsValidClient(victim))
		return Plugin_Continue;

	if (victim == attacker)
	{
		if (!m_hasRocketJumpWaypoints || m_lowHealth[victim] || m_class[victim] != TFClass_Soldier)
		{
			damage = 0.0;
			return Plugin_Handled;
		}

		return Plugin_Continue;
	}

	if (IsEBot(victim) || m_isAFK[victim])
	{
		if (m_ignoreEnemies[victim] > GetGameTime())
			return Plugin_Continue;

		if (IsValidClient(attacker))
		{
			m_damageTime[victim] = GetGameTime();

			if ((!m_hasEnemiesNear[victim] || !m_isAlive[m_nearestEnemy[victim]]) && !TF2_IsPlayerInCondition(victim, TFCond_OnFire))
			{
				m_lookAt[victim] = GetEyePosition(attacker);
				m_pauseTime[victim] = GetGameTime() + crandomfloat(2.5, 5.0);
				m_nearestEnemy[victim] = attacker;
				m_hasEnemiesNear[victim] = true;
			}

			if (m_currentIndex[victim] > 0)
				IncreaseDamage(victim, m_currentIndex[victim], damage);
			else
			{
				int index = FindWaypointFastest(GetOrigin(victim));
				if (index != -1)
					IncreaseDamage(victim, index, damage);
			}

			if ((m_class[victim] == TFClass_Scout || m_class[victim] == TFClass_Pyro) && TF2_IsPlayerInCondition(attacker, TFCond_Zoomed))
			{
				m_hasEnemiesNear[victim] = true;
				m_nearestEnemy[victim] = attacker;
				SetProcess(victim, PRO_HUNTENEMY, 180.0, "trying to hunt down sniper", true);
			}
			else if (m_class[victim] == TFClass_Spy)
			{
				if (crandomint(1, GetMaxHealth(victim)) > GetClientHealth(victim))
					ActiveCloak(victim);

				if (ChanceOf(m_eBotSenseChance[attacker]) && IsEBot(attacker))
					m_knownSpy[attacker] = victim;
				else
				{
					int search;
					for (search = 1; search <= MaxClients; search++)
					{
						if (IsValidClient(search) && m_isAlive[search] && search != victim && m_team[victim] != m_team[search])
						{
							if (ChanceOf(m_eBotSenseChance[search]) && IsEBot(search))
								m_knownSpy[search] = victim;
						}
					}
				}
			}
			else if (m_class[victim] == TFClass_Medic && IsWeaponSlotActive(victim, 1) && TF2_GetUberchargeLevel(victim) >= 100 && damage > float(GetClientHealth(victim) - 1))
			{
				damage = 0.0;
				m_buttons[victim] |= IN_ATTACK2;
				return Plugin_Handled;
			}
		}
		else if (!m_hasEntitiesNear[victim] && attacker > TFMaxPlayers && IsValidEntity(attacker))
		{
			m_damageTime[victim] = GetGameTime();
			m_pauseTime[victim] = GetGameTime() + crandomfloat(2.5, 5.0);
			m_lookAt[victim] = GetCenter(attacker);
			m_nearestEntity[victim] = attacker;
			m_hasEntitiesNear[victim] = true;
		}
	}

	return Plugin_Continue;
}

public Action OnRoundStart(Handle event, char[] name, bool dontBroadcast)
{
	if (isArena || isKOTH)
	{
		currentActiveAreaInt = 0;
		currentActiveArea = AREA1;

		if (GetConVarInt(EBotDebug) == 1)
			PrintHintTextToAll("Active Area: %s", GetAreaName(currentActiveArea));
	}
	else
	{
		currentActiveAreaInt = GetBluControlPointCount();
		currentActiveArea = (1 << (currentActiveAreaInt + 1));

		if (GetConVarInt(EBotDebug) == 1)
			PrintHintTextToAll("Active Area: %s", GetAreaName(currentActiveArea));

		/*GameRules_SetProp("m_nRoundsPlayed", 0);
		GameRules_SetProp("m_nMatchGroupType", 7);
		RequestFrame(Frame_RestartTime);*/
	}

	int client, i, cteam;
	for (client = 1; client <= MaxClients; client++)
	{
		if (!IsValidClient(client))
			continue;

		if (!IsEBot(client) && !m_isAFK[client])
			continue;

		if (!m_isAlive[client])
			continue;

		if (m_hasRouteWaypoints)
		{
			int index = -1;
			ArrayList RouteWaypoints = new ArrayList();
			for (i = 0; i < m_waypointNumber; i++)
			{
				if (!(m_paths[i].flags & WAYPOINT_ROUTE))
					continue;

				// blocked waypoint
   				if (m_paths[i].activeArea != 0 && !(m_paths[i].activeArea & currentActiveArea))
					continue;

				if (m_lastFailedWaypoint[client] == i)
					continue;

    			// not for our team
				cteam = GetClientTeam(client);
 				if (cteam == 3 && m_paths[i].team == 2)
    				continue;

   				if (cteam == 2 && m_paths[i].team == 3)
        			continue;

				RouteWaypoints.Push(i);
			}

			if (RouteWaypoints.Length > 0)
				index = RouteWaypoints.Get(crandomint(0, RouteWaypoints.Length - 1));

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
	if (isArena)
	{
		currentActiveAreaInt = 1
		currentActiveArea = AREA2;

		if (GetConVarInt(EBotDebug) == 1)
			PrintHintTextToAll("Active Area: %s", GetAreaName(currentActiveArea));
	}
	else if (!isKOTH)
	{
		currentActiveAreaInt = GetEventInt(event, "cp");
		currentActiveArea = (1 << (currentActiveAreaInt + 1));

		if (currentActiveArea > AREA9)
		{
			currentActiveAreaInt = 0;
			currentActiveArea = AREA1;
		}

		if (currentActiveArea < AREA1)
		{
			currentActiveAreaInt = 0;
			currentActiveArea = AREA1;
		}

		if (GetConVarInt(EBotDebug) == 1)
			PrintHintTextToAll("Active Area: %s", GetAreaName(currentActiveArea));
	}

	return Plugin_Handled;
}

public Action PointUnlocked(Handle event, char[] name, bool dontBroadcast)
{
	if (isArena)
	{
		currentActiveAreaInt = 1
		currentActiveArea = AREA2;

		if (GetConVarInt(EBotDebug) == 1)
			PrintHintTextToAll("Active Area: %s", GetAreaName(currentActiveArea));
	}
	else if (isKOTH)
	{
		currentActiveAreaInt = 0;
		currentActiveArea = AREA1;
	}
	else
	{
		currentActiveAreaInt = GetEventInt(event, "cp");
		currentActiveArea = (1 << (currentActiveAreaInt + 1));

		if (currentActiveArea > AREA9)
		{
			currentActiveAreaInt = 0;
			currentActiveArea = AREA1;
		}

		if (currentActiveArea < AREA1)
		{
			currentActiveAreaInt = 0;
			currentActiveArea = AREA1;
		}

		if (GetConVarInt(EBotDebug) == 1)
			PrintHintTextToAll("Active Area: %s", GetAreaName(currentActiveArea));
	}

	return Plugin_Handled;
}

public Action PointCaptured(Handle event, char[] name, bool dontBroadcast)
{
	if (isArena)
	{
		currentActiveAreaInt = 1;
		currentActiveArea = AREA2;

		if (GetConVarInt(EBotDebug) == 1)
			PrintHintTextToAll("Active Area: %s", GetAreaName(currentActiveArea));
	}
	else if (isKOTH)
	{
		int team = GetEventInt(event, "team");
		if (team == 3)
		{
			currentActiveAreaInt = 2;
			currentActiveArea = AREA3;
		}
		else if (team == 2)
		{
			currentActiveAreaInt = 1;
			currentActiveArea = AREA2;
		}
		else
		{
			currentActiveAreaInt = 0;
			currentActiveArea = AREA1;
		}
	}
	else
	{
		currentActiveAreaInt = GetEventInt(event, "cp");
		int search = GetEventInt(event, "team");

		if (search == 3)
			currentActiveArea = (1 << currentActiveAreaInt + 1)
		else
			currentActiveArea = (1 << currentActiveAreaInt - 1);

		if (currentActiveArea > AREA9)
		{
			currentActiveAreaInt = 0;
			currentActiveArea = AREA1;
		}

		if (currentActiveArea < AREA1)
		{
			currentActiveAreaInt = 0;
			currentActiveArea = AREA1;
		}

		if (GetConVarInt(EBotDebug) == 1)
			PrintHintTextToAll("Active Area: %s", GetAreaName(currentActiveArea));

		for (search = 1; search <= MaxClients; search++)
		{
			if (!IsValidClient(search))
				continue;

			if (!IsEBot(search) && !m_isAFK[search])
				continue;

			if (CurrentProcess[search] != PRO_DEFAULT)
				continue;

			SelectObjective(search);
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

public Action SetCheats(Handle timer)
{
	ConVar cheats = FindConVar("sv_cheats");
	if (cheats != null)
	{
		cheats.SetBool(false, false, false);
		int flags = cheats.Flags;
		flags |= FCVAR_NOTIFY;
		cheats.Flags = flags;
	}
	return Plugin_Handled;
}

void StringToLower(char[] string, const int length)
{
	int i;
	for (i = 0; i < length; i++)
		string[i] = CharToLower(string[i]);
}

void StringToUpper(char[] string, const int length)
{
	int i;
	for (i = 0; i < length; i++)
		string[i] = CharToUpper(string[i]);
}

int lastChat;
int repliedBy;
bool copyString;
public void OnClientSayCommand_Post(int client, const char[] command, const char[] args)
{
	copyString = false;
	for (repliedBy = 1; repliedBy <= MaxClients; repliedBy++)
	{
		if (repliedBy == lastChat)
			continue;

		if (repliedBy == client)
			continue;

		if (!IsValidClient(repliedBy))
			continue;

		if (!IsEBot(repliedBy) && !m_isAFK[repliedBy])
			continue;

		if (m_isAlive[repliedBy])
			continue;

		if (!ChanceOf(GetConVarInt(EBotReplyToChat)))
			continue;

		copyString = true;
		break;
	}

	if (copyString)
	{
		bool defaultReply = true;
		char message[256];
		strcopy(message, sizeof(message), args);
		TrimString(message);
		StringToLower(message, sizeof(message));

		char filepath[PLATFORM_MAX_PATH];
		while (!IsNullString(message) && SplitString(message, " ", message, sizeof(message)) != -1)
		{
			TrimString(message);
			BuildPath(Path_SM, filepath, sizeof(filepath), "ebot/chat/replies/%s.txt", message);
			File fp = OpenFile(filepath, "r");
			if (fp != null)
			{
				ArrayList RandomReply = new ArrayList(ByteCountToCells(65));
				char line[256];
				while (!fp.EndOfFile())
				{
					fp.ReadLine(line, sizeof(line));
					if (!line)
						continue;

					TrimString(line);
					RandomReply.PushString(line);
				}

				if (RandomReply.Length > 0)
				{
					char ChosenOne[MAX_NAME_LENGTH];
					RandomReply.GetString(crandomint(0, RandomReply.Length - 1), ChosenOne, sizeof(ChosenOne));
					DataPack pack;
					CreateDataTimer(crandomfloat(2.0, 7.0), ReplyToChat, pack);
					pack.WriteCell(client);
					pack.WriteString(ChosenOne);
					fp.Close();
					defaultReply = false
					break;
				}

				fp.Close();
			}
		}

		if (defaultReply && crandomint(1, 5) == 1)
		{
			BuildPath(Path_SM, filepath, sizeof(filepath), "ebot/chat/replies/ebot_default.txt");
			File fp = OpenFile(filepath, "r");
			if (fp != null)
			{
				ArrayList RandomReply = new ArrayList(ByteCountToCells(65));
				char line[256];
				while (!fp.EndOfFile())
				{
					fp.ReadLine(line, sizeof(line));
					if (!line)
						continue;

					TrimString(line);
					RandomReply.PushString(line);
				}

				if (RandomReply.Length > 0)
				{
					char ChosenOne[MAX_NAME_LENGTH];
					RandomReply.GetString(crandomint(0, RandomReply.Length - 1), ChosenOne, sizeof(ChosenOne));
					DataPack pack;
					CreateDataTimer(crandomfloat(2.0, 7.0), ReplyToChat, pack);
					pack.WriteCell(client);
					pack.WriteString(ChosenOne);
				}

				fp.Close();
			}
		}
	}
}

public Action ReplyToChat(Handle timer, DataPack pack)
{
	pack.Reset();
	int client = pack.ReadCell();
	if (IsValidClient(client))
	{
		char str[256];
		pack.ReadString(str, sizeof(str));
		if (ChanceOf(50))
			StringToLower(str, sizeof(str));
		else if (ChanceOf(15))
			StringToUpper(str, sizeof(str));
		FakeClientCommand(client, "say %s", str);
		lastChat = client;
	}
	return Plugin_Continue;
}
