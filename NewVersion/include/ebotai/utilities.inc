enum
{
	PRO_DEFAULT = 0,
	PRO_IDLE,
	PRO_ATTACK,
	PRO_HUNTENEMY,
	PRO_HEAL,
	PRO_SPYLURK,
	PRO_SPYHUNT,
	PRO_SPYSAP,
	PRO_DEFEND,
	PRO_ENGINEERIDLE,
	PRO_BUILDDISPENSER,
	PRO_BUILDTELEPORTERENTER,
	PRO_BUILDTELEPORTEREXIT,
	PRO_BUILDSENTRY,
	PRO_GETAMMO,
	PRO_GETHEALTH,
	PRO_HIDE
}

bool m_useTeleporter[TFMaxPlayers];
int m_teleporterEntity[TFMaxPlayers];

bool m_isSniper[TFMaxPlayers];
bool m_isMedic[TFMaxPlayers];

bool m_hasCrossbow[TFMaxPlayers];
bool m_hasWrench[TFMaxPlayers];
bool m_hasSandwich[TFMaxPlayers];
bool m_hasSticky[TFMaxPlayers];

int SentryGun[TFMaxPlayers];
int Dispenser[TFMaxPlayers];

int CurrentProcess[TFMaxPlayers];

int currentActiveArea;
int currentActiveAreaInt;
float BotCheckTimer = 0.0;

int healthpacks = -1;
int ammopacks = -1;

int m_laserIndex = -1;
int m_beamIndex = -1;
int m_hostEntity = -1;
bool m_hasHostEntity = false;

int m_primaryID[TFMaxPlayers];
int m_secondaryID[TFMaxPlayers];
int m_meleeID[TFMaxPlayers];

char m_primaryClassName[TFMaxPlayers][64];
char m_secondaryClassName[TFMaxPlayers][64];
char m_meleeClassName[TFMaxPlayers][64];

float m_redFlagCapPoint[3];
float m_bluFlagCapPoint[3];
int m_redFlag;
int m_bluFlag;
int m_capturePoint;

int m_targetNode[TFMaxPlayers];
int m_currentWaypointIndex[TFMaxPlayers];
float m_nextStuckCheck[TFMaxPlayers];
ArrayList m_positions[TFMaxPlayers];
ArrayList m_pathIndex[TFMaxPlayers];
ArrayList m_hidingSpots[TFMaxPlayers];
float m_pauseTime[TFMaxPlayers];
float m_stopTime[TFMaxPlayers];

int m_buttons[TFMaxPlayers];
float m_thinkTimer[TFMaxPlayers];
float m_lookAt[TFMaxPlayers][3];
float m_pathAhead[TFMaxPlayers][3];
float m_moveVel[TFMaxPlayers][3];

int m_goalEntity[TFMaxPlayers];	
float m_goalPosition[TFMaxPlayers][3];
int m_goalIndex[TFMaxPlayers];
int m_currentIndex[TFMaxPlayers];

float m_duckTimer[TFMaxPlayers];
float m_attackTimer[TFMaxPlayers];
float m_attack2Timer[TFMaxPlayers];

bool m_isSlowThink[TFMaxPlayers];

bool m_hasEnemiesNear[TFMaxPlayers];
bool m_hasFriendsNear[TFMaxPlayers];
bool m_hasEntitiesNear[TFMaxPlayers];

int m_enemiesNearCount[TFMaxPlayers];
int m_friendsNearCount[TFMaxPlayers];

int m_enemiesNearPower[TFMaxPlayers];
int m_friendsNearPower[TFMaxPlayers];

int m_nearestEnemy[TFMaxPlayers];
int m_nearestFriend[TFMaxPlayers];
int m_nearestEntity[TFMaxPlayers];
int m_lastEnemy[TFMaxPlayers];
float m_lastEnemyOrigin[TFMaxPlayers][3];

float m_enemyDistance[TFMaxPlayers];
float m_friendDistance[TFMaxPlayers];

bool m_lowHealth[TFMaxPlayers];
bool m_lowAmmo[TFMaxPlayers];

int m_eBotSenseChance[TFMaxPlayers];
int m_eBotDodgeRange[TFMaxPlayers];

int m_knownSpy[TFMaxPlayers];
int m_stickyCount[TFMaxPlayers];
int m_knownSentry[TFMaxPlayers];
int m_healTarget[TFMaxPlayers];

bool WantsBuildSentryGun[TFMaxPlayers];
bool WantsBuildDispenser[TFMaxPlayers];
bool WantsBuildTeleporterEnter[TFMaxPlayers];
bool WantsBuildTeleporterExit[TFMaxPlayers];
bool m_enterFail[TFMaxPlayers];
bool m_exitFail[TFMaxPlayers];

int m_lastFailedEntity[TFMaxPlayers];
int m_difficulty[TFMaxPlayers];

int m_redPayload = -1;
int m_bluPayload = -1;

float m_damageTime[TFMaxPlayers];
int m_lastKiller[TFMaxPlayers];

float m_origin[TFMaxPlayers][3];
float m_eyeOrigin[TFMaxPlayers][3];

float m_lastEnemySeen[TFMaxPlayers];
float m_lastEntitySeen[TFMaxPlayers];

Handle EBotDebug;
Handle EBotFPS;
Handle EBotMelee;
Handle EBotSenseMax;
Handle EBotSenseMin;
Handle EBotDifficulty;
Handle EBotMedicFollowRange;
Handle m_eBotDodgeRangeMin;
Handle m_eBotDodgeRangeMax;
Handle m_eBotDodgeRangeChance;
Handle EBotQuota;
Handle EBotChangeClassChance;
Handle EBotDeadChat;
Handle EBotAutoWaypoint;
Handle EBotRadius;
Handle EBotDistance;
Handle EBotChangeClass;
Handle EBotChangeClassRandom;
Handle EBotAllowAttackButtons;
Handle EBotForceHuntEnemy;
Handle EBotAFKPlayers;
Handle EBotAFKCommand;
Handle EBotAFKTime;
Handle EBotReplyToChat;

Handle g_hCustomTaunt;

bool m_isAFK[TFMaxPlayers];

Handle gH_RestartTimer = null;
int gI_RestartTimerIteration = 0;

int g_seed;
stock int frand()
{
	g_seed = (214013 * g_seed + 2531011);
	return (g_seed >> 16) & 32767;
}

stock int crandomint(const int min, const int max)
{
	return frand() % (max - min + 1) + min;
}

stock void GetRandomPoint(CNavArea area, float buffer[3])
{
	static float extentLow[3];
	static float extentHigh[3];
	area.GetExtent(extentLow, extentHigh);
	
	buffer[0] = GetRandomFloat(extentLow[0], extentHigh[0]);
	buffer[1] = GetRandomFloat(extentLow[1], extentHigh[1]);
	buffer[2] = (extentLow[2] + extentHigh[2]) * 0.5;
}

bool m_isEBot[TFMaxPlayers];

stock bool IsEBot(const int bot)
{
	if (m_isEBot[bot])
		return true;
	
	return false;
}

#define	MASK_CUSTOM (CONTENTS_SOLID|CONTENTS_MOVEABLE|0x40|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS)

public void InitGamedata()
{
	StartPrepSDKCall(SDKCall_Player);
	PrepSDKCall_SetSignature(SDKLibrary_Server, "\x55\x8B\xEC\x81\xEC\x0C\x01\x00\x00\x53\x8B\x5D\x08\x57\x8B", 15);
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
	if ((g_hCustomTaunt = EndPrepSDKCall()) == null)
	{
		StartPrepSDKCall(SDKCall_Player);
		PrepSDKCall_SetSignature(SDKLibrary_Server, "@_ZN9CTFPlayer22PlayTauntSceneFromItemEPK13CEconItemView", 1);
		PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
		if ((g_hCustomTaunt = EndPrepSDKCall()) == null)
			g_hCustomTaunt = null;
	}
}

stock bool IsSniper(const int client)
{
	if (IsEBot(client) || m_isAFK[client])
		return m_isSniper[client];
	else
		return m_class[client] == TFClass_Sniper;
}

stock bool IsMedic(const int client)
{
	if (IsEBot(client) || m_isAFK[client])
		return m_isMedic[client];
	else
		return m_class[client] == TFClass_Medic;
}

stock bool HasCrossbow(const int client)
{
	if (IsEBot(client) || m_isAFK[client])
		return m_hasCrossbow[client];
	else
		return false;
}

stock bool HasSandwich(const int client)
{
	if (IsEBot(client) || m_isAFK[client])
		return m_hasSandwich[client];
	else
		return false;
}

stock bool HasSticky(const int client)
{
	if (IsEBot(client) || m_isAFK[client])
		return m_hasSticky[client];
	else
		return false;
}

stock bool GotDamageByLast(const int client, const float value)
{
	return m_damageTime[client] > GetGameTime() + value;
}

stock bool ActiveCloak(const int client)
{
	if (!TF2_IsPlayerInCondition(client, TFCond_Cloaked) && !TF2_IsPlayerInCondition(client, TFCond_DeadRingered))
		m_buttons[client] |= IN_ATTACK2;

	return true;
}

stock bool DeactiveCloak(const int client)
{
	if (TF2_IsPlayerInCondition(client, TFCond_Cloaked) || TF2_IsPlayerInCondition(client, TFCond_DeadRingered))
	{
		if (m_lastEnemySeen[client] + 4.0 > GetGameTime() || m_lastEntitySeen[client] + 4.0 > GetGameTime())
			return false;
		
		m_buttons[client] |= IN_ATTACK2;
	}

	return true;
}

stock void FindPayload()
{
	int iPayload;
	while (IsValidEntity((iPayload = FindEntityByClassname(iPayload, "mapobj_cart_dispenser"))))
	{
		if (GetTeamNumber(iPayload) == 2)
			m_redPayload = iPayload;
		else
			m_bluPayload = iPayload;
	}
}

stock void FindHostEntity()
{
	int i;
	for (i = 1; i < MaxClients; i++)
	{
		if (IsValidClient(i) && !IsFakeClient(i))
		{
			m_hostEntity = i;
			m_hasHostEntity = true;
			break;
		}
	}
}

stock int FindBotByName(const char[] name)
{
	int i;
	char buffer[MAX_NAME_LENGTH];
	for (i = 1; i < MaxClients; i++)
	{
		if (IsValidClient(i) && IsFakeClient(i))
		{
			GetClientName(i, buffer, sizeof(buffer));

			if (!buffer)
				continue;

			if (StrEqual(name, buffer))
				return i;
		}
	}

	return -1;
}

stock int GetTeamsCount(const int team)
{
	int i;
	int count = 0;
	for (i = 1; i < MaxClients; i++)
	{
		if (IsValidClient(i) && GetClientTeam(i) == team)
			count++;
	}
	return count;
}

stock int GetBotCount()
{
	int i;
	int count = 0;
	for (i = 1; i < MaxClients; i++)
	{
		if (IsValidClient(i) && IsEBot(i))
			count++;
	}
	return count;
}

stock int GetPlayersCount()
{
	int i;
	int count = 0;
	for (i = 1; i < MaxClients; i++)
	{
		if (IsValidClient(i) && !IsEBot(i) && GetClientTeam(i) != 1)
			count++;
	}
	return count;
}

stock int GetPlayersCountRed()
{
	int i;
	int count = 0;
	for (i = 1; i < MaxClients; i++)
	{
		if (IsValidClient(i) && GetClientTeam(i) == 2)
			count++;
	}
	return count;
}

stock int GetPlayersCountBlu()
{
	int i;
	int count = 0;
	for (i = 1; i < MaxClients; i++)
	{
		if (IsValidClient(i) && GetClientTeam(i) == 3)
			count++;
	}
	return count;
}

stock int GetTotalPlayersCount()
{
	int i;
	int count = 0;
	for (i = 1; i < MaxClients; i++)
	{
		if (IsValidClient(i) && GetClientTeam(i) != 1)
			count++;
	}
	return count;
}

// squared int value
stock int Squared(const int number)
{
	return number * number;
}

// squared float value
stock float Squaredf(const float number)
{
	return number * number;
}

stock float[] GetEyePosition(const int client)
{
	static float vector[3];
	GetClientEyePosition(client, vector);
	return vector;
}

stock bool ChanceOf(const int number)
{
	return crandomint(1, 100) <= number;
}

stock float[] GetOrigin(const int entity)
{
	static float vector[3];
	GetEntPropVector(entity, Prop_Data, "m_vecAbsOrigin", vector);
	return vector;
}

stock int GetEnemyTeam(const int team)
{
	return team == 2 ? 3 : 2;
}

stock void EyeVectors(const int client, float fw[3] = NULL_VECTOR, float right[3] = NULL_VECTOR, float up[3] = NULL_VECTOR)
{
	GetAngleVectors(GetEyeAngles(client), fw, right, up);
}

stock void MakeVectors(float defaultangle[3] = NULL_VECTOR, float fw[3] = NULL_VECTOR, float right[3] = NULL_VECTOR, float up[3] = NULL_VECTOR)
{
	GetAngleVectors(defaultangle, fw, right, up);
}

stock float[] GetEyeAngles(const int client)
{
	static float vector[3];
	GetClientEyeAngles(client, vector);
	return vector;
}

stock float[] GetCenter(const int entity)
{
	static float center[3];
	static CBaseEntity cbase;
	cbase = view_as<CBaseEntity>(entity);
	cbase.WorldSpaceCenter(center);
	return center;
}

stock void EquipWeaponSlot(const int client, const int slot)
{
	if (IsWeaponSlotActive(client, slot))
		return;

	static int iWeapon;
	iWeapon = GetPlayerWeaponSlot(client, slot);
	if (IsValidEntity(iWeapon))
		EquipWeapon(client, iWeapon);
}

stock void EquipWeapon(const int client, const int weapon)
{
	static char class[128];
	GetEntityClassname(weapon, class, sizeof(class));
	Format(class, sizeof(class), "use %s", class);
	FakeClientCommandThrottled(client, class);
}

float g_flNextCommand[TFMaxPlayers];
stock bool FakeClientCommandThrottled(const int client, const char[] command)
{
	if (g_flNextCommand[client] > GetGameTime())
		return false;
	FakeClientCommand(client, command);
	g_flNextCommand[client] = GetGameTime() + 0.4;
	return true;
}

stock void PlayTaunt(const int client, const int tauntid = 463)
{
	if (g_hCustomTaunt != null)
	{
		int ent = MakeCEIVEnt(client, tauntid);
		if (!IsValidEntity(ent))
		{
			FakeClientCommandThrottled(client, "taunt");
			return;
		}
		
		int iCEIVOffset = GetEntSendPropOffs(ent, "m_Item", true);
		if (iCEIVOffset <= 0)
		{
			FakeClientCommandThrottled(client, "taunt");
			return;
		}

		Address pEconItemView = GetEntityAddress(ent);
		if (!IsValidAddress(pEconItemView))
		{
			FakeClientCommandThrottled(client, "taunt");
			return;
		}

		pEconItemView += view_as<Address>(iCEIVOffset);
		SDKCall(g_hCustomTaunt, client, pEconItemView);
		AcceptEntityInput(ent, "Kill");
	}
	else
		FakeClientCommandThrottled(client, "taunt");
}

stock int MakeCEIVEnt(const int client, const int itemdef)
{
	static Handle hItem;
	if (hItem == INVALID_HANDLE)
	{
		hItem = TF2Items_CreateItem(OVERRIDE_ALL|PRESERVE_ATTRIBUTES|FORCE_GENERATION);
		TF2Items_SetClassname(hItem, "tf_wearable_vm");
		TF2Items_SetQuality(hItem, 6);
		TF2Items_SetLevel(hItem, 1);
	}

	TF2Items_SetItemIndex(hItem, itemdef);
	TF2Items_SetNumAttributes(hItem, 0);
	return TF2Items_GiveNamedItem(client, hItem);
}

stock bool IsValidAddress(const Address pAddress)
{
	return pAddress != Address_Null;
}

stock int FindNearestEnemy(const int client, const float flMaxDistance = 9999999999.0)
{
	int i;
	float flBestDistance = Squaredf(flMaxDistance);
	float flDistance;
	int iBestTarget = -1;
	for (i = 1; i <= MaxClients; i++)
	{
		if (client == i)
			continue;

		if (!IsValidClient(i))
			continue;
		
		if (!m_isAlive[i])
			continue;
		
		if (m_team[i] == m_team[client])
			continue;

		if (GetEntProp(i, Prop_Data, "m_takedamage") != 2)
			continue;
		
		if (TF2_IsPlayerInCondition(i, TFCond_HalloweenGhostMode))
			continue;
		
		if (TF2_IsPlayerInCondition(i, TFCond_Ubercharged))
			continue;
		
		if (TF2_IsPlayerInCondition(i, TFCond_Cloaked))
			continue;
		
		if (TF2_IsPlayerInCondition(i, TFCond_Disguised) && !IsRevealedSpyByTarget(i, client))
			continue;
		
		// this is important!!!
		if (TF2_HasTheFlag(i))
		{
			iBestTarget = i;
			break;
		}
		
		flDistance = GetVectorDistance(GetOrigin(client), GetOrigin(i), true);
		if (flDistance < flBestDistance)
		{
			flBestDistance = flDistance;
			iBestTarget = i;
		}
	}

	return iBestTarget;
}

float m_aimInterval[TFMaxPlayers];
stock void LookAtPosition(const int client, const float flGoal[3], float flAng[3])
{
	static float delta;
	delta = GetGameTime() - m_aimInterval[client];
	m_aimInterval[client] = GetGameTime();

	static float flPos[3];
    GetClientEyePosition(client, flPos);
    
    // get normalised direction from target to client
    static float desired_dir[3];
    MakeVectorFromPoints(flPos, flGoal, desired_dir);
    GetVectorAngles(desired_dir, desired_dir);
	
	static float aimSpeed;
	aimSpeed = 16.0 * delta;
    
    // ease the current direction to the target direction
    flAng[0] += AngleNormalize(desired_dir[0] - flAng[0]) * aimSpeed;
    flAng[1] += AngleNormalize(desired_dir[1] - flAng[1]) * aimSpeed;
	TeleportEntity(client, NULL_VECTOR, flAng, NULL_VECTOR);
}

stock float AngleNormalize(float angle)
{
	angle = fmodf(angle, 360.0);
	if (angle > 180) 
		angle -= 360;
	
	if (angle < -180)
		angle += 360;
	
	return angle;
}

stock float fmodf(const float number, const float denom)
{
	return number - RoundToFloor(number / denom) * denom;
}

stock float AngleDiff(const float destAngle, const float srcAngle)
{
	return AngleNormalize(destAngle - srcAngle);
}

stock float ApproachAngle(const float target, float value, float speed)
{
	float delta = AngleDiff(target, value);
	
	if (speed < 0.0) 
		speed = -speed;
	
	if (delta > speed) 
		value += speed;
	else if (delta < -speed) 
		value -= speed;
	else
		value = target;
	
	return AngleNormalize(value);
}

stock void MoveTo(const int client, const float flGoal[3])
{
	static float flPos[3];
	GetClientAbsOrigin(client, flPos);
	
	static float fAng[3];
	GetClientEyeAngles(client, fAng);
	
	static float newmove[3];
	SubtractVectors(flGoal, flPos, newmove);

	newmove[1] = -newmove[1];
	
	static float sin;
	static float cos;
	sin = Sine(fAng[1] * 0.01745329251);
	cos = Cosine(fAng[1] * 0.01745329251);
	
	m_moveVel[client][0] = cos * newmove[0] - sin * newmove[1];
	m_moveVel[client][1] = sin * newmove[0] + cos * newmove[1];

	ScaleVector(m_moveVel[client], GetEntPropFloat(client, Prop_Data, "m_flMaxspeed"));
}

stock void MoveToWaypoint(const int client, float flGoal[3])
{
	float flPos[3];
	GetClientAbsOrigin(client, flPos);
	
	float flVel[3];
	GetEntPropVector(client, Prop_Data, "m_vecAbsVelocity", flVel);
	
	float fps = GetConVarFloat(EBotFPS);
	flPos[0] += flVel[0] * fps;
	flPos[1] += flVel[1] * fps;
	flGoal[0] -= flVel[0] * fps;
	flGoal[1] -= flVel[1] * fps;

	float fAng[3];
	GetClientEyeAngles(client, fAng);
	
	float newmove[3];
	SubtractVectors(flGoal, flPos, newmove);
	
	newmove[1] = -newmove[1];
	
	float sin = Sine(fAng[1] * 0.01745329251);
	float cos = Cosine(fAng[1] * 0.01745329251);
	
	m_moveVel[client][0] = cos * newmove[0] - sin * newmove[1];
	m_moveVel[client][1] = sin * newmove[0] + cos * newmove[1];

	float maxSpeed = GetEntPropFloat(client, Prop_Data, "m_flMaxspeed");
	float fixed = GetVectorDistance2D(flPos, flGoal) - (10.0 + (fps * maxSpeed));
	if (fixed < fps)
		fixed = fps;

	m_nextStuckCheck[client] = GetGameTime() + 5.0;
	ScaleVector(m_moveVel[client], fixed);
}

stock void MoveOut(const int client, float flGoal[3])
{
	float flPos[3];
	GetClientAbsOrigin(client, flPos);

	float fAng[3];
	GetClientEyeAngles(client, fAng);
	
	float newmove[3];
	SubtractVectors(flGoal, flPos, newmove);
	
	float sin = Sine(fAng[1] * 0.01745329251);
	float cos = Cosine(fAng[1] * 0.01745329251);
	
	m_moveVel[client][0] = cos * newmove[0] - sin * newmove[1];
	m_moveVel[client][1] = sin * newmove[0] + cos * newmove[1];
	
	NormalizeVector(m_moveVel[client], m_moveVel[client]);
	ScaleVector(m_moveVel[client], 500.0);
}

stock void BackstabMove(const int client, const int target, const bool moveForward = true)
{
	float flBotAng[3], flTargetAng[3];
	GetClientEyeAngles(client, flBotAng);
	GetClientEyeAngles(target, flTargetAng);
	float iAngleDiff = AngleDiff(flBotAng[1], flTargetAng[1]);
	
	if (TF2_IsPlayerInCondition(client, TFCond_Cloaked) || GetClientAimTarget(target) == client)
		m_moveVel[client][0] = -500.0;
	else if (moveForward)
		m_moveVel[client][0] = 500.0;
	
	if (iAngleDiff > 90)
		m_moveVel[client][1] = -500.0;
	else if (iAngleDiff < -90)
		m_moveVel[client][1] = 500.0;
}

stock bool IsValidClient(const int client)
{
	if (client < 1 || client > MaxClients)
		return false;
	
	if (!IsClientInGame(client))
		return false;
	
	return true;
}

stock bool IsVisible(float start[3], float end[3])
{
	TR_TraceRayFilter(start, end, MASK_CUSTOM, RayType_EndPoint, TraceEntityFilterStuff);
	if (TR_GetFraction() == 1.0) 
		return true;
	
	return false;
}

float hully[3] = {-18.0, -18.0, -18.0};
float hullx[3] = {18.0, 18.0, 18.0};

stock bool IsVisibleWaypoint(float start[3], float end[3])
{
	static float fstart[3];
	static float fend[3];

	fstart[0] = start[0];
	fstart[1] = start[1];
	fstart[2] = start[2] + 64.0;
	fend[0] = end[0];
	fend[1] = end[1];
	fend[2] = end[2] + 64.0;

	Handle tr = TR_TraceHullFilterEx(fstart, fend, hully, hullx, MASK_PLAYERSOLID, WaypointFilter);
	float fr = TR_GetFraction(tr);
	CloseHandle(tr);

	return fr >= 1.0;
}

stock bool IsVisibleWaypointDebug(float start[3], float end[3])
{
	float fstart[3];
	float fend[3];

	fstart[0] = start[0];
	fstart[1] = start[1];
	fstart[2] = start[2] + 18.0;
	fend[0] = end[0];
	fend[1] = end[1];
	fend[2] = end[2] + 18.0;

	TR_TraceRayFilter(fstart, fend, MASK_CUSTOM, RayType_EndPoint, WaypointFilter);
	return TR_GetFraction() >= 0.9;
}

stock float[] VectorWithRoot(float origin[3], float root1 = 0.0, float root2 = 0.0, float root3 = 0.0)
{
	float vector[3];
	vector[0] = origin[0] + root1;
	vector[1] = origin[1] + root2;
	vector[2] = origin[2] + root3;
	return vector;
}

stock float[] VectorAsFloat(int origin[3])
{
	float vector[3];
	vector[0] = float(origin[0]);
	vector[1] = float(origin[1]);
	vector[2] = float(origin[2]);
	return vector;
}

public bool TraceEntityFilterStuff(int entity, int mask)
{
	static char class[64];
	GetEntityClassname(entity, class, sizeof(class));
	if (StrEqual(class, "entity_medigun_shield"))
		return false;
	else if (StrEqual(class, "func_respawnroomvisualizer"))
		return false;
	else if (StrContains(class, "tf_projectile_", false) != -1)
		return false;
	else if (StrContains(class, "obj_", false) != -1)
		return false;
	else if (StrContains(class, "tf_zombie", false) != -1)
		return false;
	else if (StrContains(class, "tank_boss", false) != -1)
		return false;
	else if (StrContains(class, "merasmus", false) != -1)
		return false;
	else if (StrContains(class, "eyeball_boss", false) != -1)
		return false;
	else if (StrContains(class, "headless_hatman", false) != -1)
		return false;
	else if (StrContains(class, "tf_robot_destruction_robot", false) != -1)
		return false;
	else if (StrEqual(class, "entity_revive_marker"))
		return false;
	else if (StrEqual(class, "mapobj_cart_dispenser"))
		return false;
	return entity > MaxClients;
}

stock int TF2_GetNumberOfHealers(const int client)
{
    return GetEntProp(client, Prop_Send, "m_nNumHealers");
}

stock int GetTeamNumber(const int entity)
{
	return GetEntProp(entity, Prop_Send, "m_iTeamNum");
}

stock int GetOwnerEntity(const int entity)
{
	return GetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity");
}

stock int GetEntityThrower(const int entity)
{
	return GetEntPropEnt(entity, Prop_Send, "m_hThrower");
}

stock bool IsWeaponSlotActive(int client, int slot)
{
    return GetPlayerWeaponSlot(client, slot) == GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
}

stock int GetActiveWeaponSlot(const int client)
{
	int active = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
	if (active == -1)
		return -1;
	
	int i;
	for (i = 0; i < 5; i++)
	{
		if (GetPlayerWeaponSlot(client, i) == active)
			return i;
	}
	
	return -1;
}

stock int GetActiveWeapon(const int client)
{
	return GetEntPropEnt(client, Prop_Data, "m_hActiveWeapon");
}

stock int GetMetal(const int client)
{
	return GetEntProp(client, Prop_Data, "m_iAmmo", 4, 3);
}

stock bool TF2_HasTheFlag(const int client)
{
	if (isCTF)
	{
		if (IsValidEntity(m_redFlag) && GetEntPropEnt(m_redFlag, Prop_Data, "m_hOwnerEntity") == client)
			return true;
	
		if (IsValidEntity(m_bluFlag) && GetEntPropEnt(m_bluFlag, Prop_Data, "m_hOwnerEntity") == client)
			return true;
	}
	else if (isMVM)
	{
		int ent;
    	while ((ent = FindEntityByClassname(ent, "item_teamflag")) != -1)
    	{
        	if (IsValidEntity(ent) && GetEntPropEnt(ent, Prop_Data, "m_hOwnerEntity") == client)
            	return true;
    	}
	}

    return false;
}

stock int FindLeader(const int team)
{
	int score;
	int i;
	int bestScore = 0;
	int leader = -1;
	for (i = 1; i <= MaxClients; i++)
	{
		if (!IsValidClient(i))
			continue;

		if (GetClientTeam(i) != team)
			continue;
		
		score = GetClientScore(i);
		if (score > bestScore)
		{
			bestScore = score;
			leader = i;
		}
	}

	return leader;
}

stock int FindBestKD(const int team)
{
	int i;
	float score;
	float stableKD = 0.0;
	int leader = -1;
	for (i = 1; i <= MaxClients; i++)
	{
		if (!IsValidClient(i))
			continue;

		if (GetClientTeam(i) != team)
			continue;
		
		score = GetKDR(i);
		if (score > stableKD)
		{
			stableKD = score;
			leader = i;
		}
	}

	return leader;
}

stock bool IsReloading(const int client)
{
	int weapon = GetActiveWeapon(client);
	if (weapon != -1)
		return GetEntProp(weapon, Prop_Data, "m_bInReload") != 0;
	return false;
}

stock float GetMaxSpeed(const int entity)
{
	return GetEntPropFloat(entity, Prop_Send, "m_flMaxspeed");
}

stock int GetClientScore(const int client)
{
	return GetEntProp(client, Prop_Data, "m_iFrags");
}

stock int GetClientDeath(const int client)
{
	return GetEntProp(client, Prop_Data, "m_iDeaths");
}

stock float GetKDR(const int client)
{
	return float(GetClientScore(client) + 1) / float(GetClientDeath(client) + 1);
}

stock int TF2_GetPlayerWeaponID(const int client, const int slot)
{
	int validslot = GetPlayerWeaponSlot(client, slot);
	if (!IsValidEntity(validslot))
		return -1;
	return GetEntProp(validslot, Prop_Send, "m_iItemDefinitionIndex");
}

stock int TF2_GetWeaponID(const int weapon)
{
	return GetEntProp(weapon, Prop_Send, "m_iItemDefinitionIndex");
}

stock char[] TF2_GetWeaponClassName(const int weapon)
{
	char className[64] = "UNKNOWN";
	GetEntityClassname(weapon, className, sizeof(className));
	return className;
}

stock bool hasWrench(const int client)
{
	return m_hasWrench[client];
}

stock int TF2_GetHealingTarget(const int client)
{
	int index = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
	if (!HasEntProp(index, Prop_Send, "m_hHealingTarget"))
		return -1;
	return GetEntPropEnt(index, Prop_Send, "m_hHealingTarget");
}

stock float TF2_GetUberchargeLevel(const int client)
{
	int index = GetPlayerWeaponSlot(client, 1);
	if (IsValidEntity(index))
		return GetEntPropFloat(index, Prop_Send, "m_flChargeLevel") * 100.0;
	
	return 0.0;
}

stock int GetMaxHealth(const int entity)
{
	return GetEntProp(entity, Prop_Data, "m_iMaxHealth");
}

stock bool IsMoving(const int client)
{
	static float buffer[3];
	GetEntPropVector(client, Prop_Data, "m_vecAbsVelocity", buffer);
	return GetVectorLength(buffer, true) > GetEntPropFloat(client, Prop_Send, "m_flMaxspeed");
}

stock bool IsMetalLow(const int client)
{
	if (WantsBuildSentryGun[client])
		return GetMetal(client) <= 130.0;
	else if (WantsBuildDispenser[client])
		return GetMetal(client) <= 100.0;
	else if (WantsBuildTeleporterEnter[client] || WantsBuildTeleporterExit[client])
		return GetMetal(client) <= 50.0;
	else if (GetMetal(client) <= 0.0)
		return true;
	
	return false;
}

stock bool IsResupply(const int entity, const int team)
{
	char model[PLATFORM_MAX_PATH];
	if (GetTeamNumber(entity) == team)
	{
		GetEntPropString(entity, Prop_Data, "m_ModelName", model, sizeof(model));
		return StrEqual(model, "models/props_gameplay/resupply_locker.mdl");
	}
	return false;
}

stock bool IsAttacking(const int client)
{
	if (GetClientButtons(client) & IN_ATTACK)
		return true;
	return false;
}

stock float GetHeight(const int client)
{
	static float eyepos[3]; eyepos = GetEyePosition(client);
	static float origin[3]; origin = GetOrigin(client);
	return (eyepos[2] - origin[2]);
}

stock int GetAmmo(const int client, const int slot)
{
    int weapon = GetPlayerWeaponSlot(client, slot);
    if (IsValidEntity(weapon))
    {
        int iOffset = GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType", 1) * 4;
        int iAmmoTable = FindSendPropInfo("CTFPlayer", "m_iAmmo");
        return GetEntData(client, iAmmoTable + iOffset);
    }
    return 9999;
}

stock int GetPrimaryClip(const int client, const int slot)
{
	int weapon = GetPlayerWeaponSlot(client, slot);
	if (!IsValidEntity(weapon))
		return 9999;
	return GetEntProp(weapon, Prop_Send, "m_iClip1");
}

stock int GetPrimaryClipN(const int client, const int slot)
{
	int weapon = GetPlayerWeaponSlot(client, slot);
	if (!IsValidEntity(weapon))
		return -1;
	return GetEntProp(weapon, Prop_Send, "m_iClip1");
}

stock float Clamp(const float value, const float min, const float max)
{
    if (value < min)
        return min;
    else if (value > max)
        return max;
	
    return value;
}  

// checks wall
stock bool CheckWall(const int client)
{
	float flPos[3];
	GetClientAbsOrigin(client, flPos);
	
	float flMaxs[3], flMins[3];
	GetEntPropVector(client, Prop_Send, "m_vecMaxs", flMaxs);
	GetEntPropVector(client, Prop_Send, "m_vecMins", flMins);
	
	flMaxs[0] += 3;
	flMaxs[1] += 3;
	flMins[0] -= 3;
	flMins[1] -= 3;
	
	flPos[2] += 18.0;
	
	// Perform a wall check to see if we are near any obstacles we should try jump over
	Handle TraceRay = TR_TraceHullFilterEx(flPos, flPos, flMins, flMaxs, MASK_PLAYERSOLID, StuckFilter, client);
	bool bHit = TR_DidHit(TraceRay);
	
	delete TraceRay;
	return bHit;
}

public bool ExcludeFilter(int entity, int contentsMask, any iExclude)
{
	static char class[64];
	GetEntityClassname(entity, class, sizeof(class));
	if (StrEqual(class, "player"))
	{
		if (GetClientTeam(entity) == GetClientTeam(iExclude))
			return false;
	}
	else if (StrEqual(class, "entity_medigun_shield"))
	{
		if (GetEntProp(entity, Prop_Send, "m_iTeamNum") == GetClientTeam(iExclude))
			return false;
	}
	else if (StrEqual(class, "func_respawnroomvisualizer"))
		return false;
	else if (StrContains(class, "tf_projectile_", false) != -1)
		return false;
	else if (StrContains(class, "obj_", false) != -1)
		return false;
	else if (StrEqual(class, "entity_revive_marker"))
		return false;
	return !(entity == iExclude);
}

public bool StuckFilter(int entity, int contentsMask, any iExclude)
{
	static char class[64];
	GetEntityClassname(entity, class, sizeof(class));
	if (StrEqual(class, "player"))
	{
		if (GetClientTeam(entity) == GetClientTeam(iExclude))
			return false;
	}
	else if (StrEqual(class, "entity_medigun_shield"))
	{
		if (GetEntProp(entity, Prop_Send, "m_iTeamNum") == GetClientTeam(iExclude))
			return false;
	}
	else if (StrEqual(class, "func_respawnroomvisualizer"))
		return false;
	else if (StrEqual(class, "entity_revive_marker"))
		return false;
	return !(entity == iExclude);
}

stock int GetPlayerStickyCount(const int client)
{
	int x;
	int StickyCount = 0;
	char ClassName[32];
	for (x = 0; x <= GetMaxEntities(); x++)
	{
		if (!IsValidEntity(x))
			continue;
		
		if (!HasEntProp(x, Prop_Send, "m_hThrower"))
			continue;
		
		if (GetEntityThrower(x) != client)
			continue;

		GetEdictClassname(x, ClassName, 32);
		if (!ClassName)
			continue;

		if (StrContains(ClassName, "tf_projectile_pipe_remote", false) != -1)
			StickyCount++;
	}
	return StickyCount;
}

stock int GetBluControlPointCount()
{
	int x;
	int count = 0;
	char ClassName[32];
	for (x = 0; x <= GetMaxEntities(); x++)
	{
		if (!IsValidEntity(x))
			continue;
		
		if (!HasEntProp(x, Prop_Send, "m_iTeamNum"))
			continue;
		
		if (GetTeamNumber(x) != 3)
			continue

		GetEdictClassname(x, ClassName, 32);
		if (!ClassName)
			continue;

		if (StrEqual(ClassName, "team_control_point", false))
			count++;
	}

	return count;
}

stock int GetNearestEnemyControlPoint(const int client)
{
	int NearestControlPoint = -1;
	
	float distance = -1.0; 
	float nearestDistance = 9999999999999999.0;
	
	int iControlPoint;
	while ((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
	{
		if (!IsValidEntity(iControlPoint))
			continue;
		
		if (GetClientTeam(client) == GetTeamNumber(iControlPoint))
			continue;
		
		distance = GetFastDistance(GetOrigin(client), GetOrigin(iControlPoint));
		if (distance < nearestDistance)
		{
			NearestControlPoint = iControlPoint;
			nearestDistance = distance;
		}
	}
	
	return NearestControlPoint;
}

stock int GetNearestPayload(const int client, const int team)
{
	int NearestPayload = -1;
	float distance = -1.0; 
	float nearestDistance = 9999999999999999.0;
	int iPayload;

	while ((iPayload = FindEntityByClassname(iPayload, "mapobj_cart_dispenser")) != -1)
	{
		if (!IsValidEntity(iPayload))
			continue;
		
		if (GetTeamNumber(iPayload) != team)
			continue;
		
		distance = GetFastDistance(GetOrigin(client), GetOrigin(iPayload));
		if (distance < nearestDistance)
		{
			NearestPayload = iPayload;
			nearestDistance = distance;
		}
	}

	return NearestPayload;
}

// check wall on right
stock bool CheckWallRight(const int client)
{
	float right[3], fw[3];
	EyeVectors(client, fw, right);

	float clientOrigin[3];
	clientOrigin = GetCenter(client);

	float clientTargetOrigin[3];
	clientTargetOrigin[0] = clientOrigin[0] + (fw[0] * 32.0) + (right[0] * 32.0);
	clientTargetOrigin[1] = clientOrigin[1] + (fw[1] * 32.0) + (right[1] * 32.0);
	clientTargetOrigin[2] = clientOrigin[2];

	TR_TraceRayFilter(clientOrigin, clientTargetOrigin, MASK_PLAYERSOLID, RayType_EndPoint, StuckFilter, client);
	float flFraction = TR_GetFraction();

	if (GetConVarInt(EBotDebug) == 1)
	{
		TE_SetupBeamPoints(clientOrigin, clientTargetOrigin, m_laserIndex, m_laserIndex, 0, 30, 0.1, 1.0, 1.0, 5, 0.0, (flFraction != 1.0 ? {255, 0, 0, 255} : {0, 0, 255, 255}), 30);
		TE_SendToClient(m_hostEntity);
	}
	
	if (flFraction != 1.0) 
		return true;
	
	return false;
}

// check wall on left
stock bool CheckWallLeft(const int client)
{
	float right[3], fw[3];
	EyeVectors(client, fw, right);

	float clientOrigin[3];
	clientOrigin = GetCenter(client);

	float clientTargetOrigin[3];
	clientTargetOrigin[0] = clientOrigin[0] + (fw[0] * 32.0) - (right[0] * 32.0);
	clientTargetOrigin[1] = clientOrigin[1] + (fw[1] * 32.0) - (right[1] * 32.0);
	clientTargetOrigin[2] = clientOrigin[2];

	TR_TraceRayFilter(clientOrigin, clientTargetOrigin, MASK_PLAYERSOLID, RayType_EndPoint, StuckFilter, client);
	float flFraction = TR_GetFraction();

	if (GetConVarInt(EBotDebug) == 1)
	{
		TE_SetupBeamPoints(clientOrigin, clientTargetOrigin, m_laserIndex, m_laserIndex, 0, 30, 0.1, 1.0, 1.0, 5, 0.0, (flFraction != 1.0 ? {255, 0, 0, 255} : {0, 0, 255, 255}), 30);
		TE_SendToClient(m_hostEntity);
	}

	if (flFraction != 1.0) 
		return true;
	
	return false;
}

// check wall on right
stock bool CheckWallRight2(const int client)
{
	float right[3], fw[3];
	EyeVectors(client, fw, right);

	float clientOrigin[3];
	clientOrigin = GetCenter(client);

	float clientTargetOrigin[3];
	clientTargetOrigin[0] = clientOrigin[0] - (fw[0] * 32.0) + (right[0] * 32.0);
	clientTargetOrigin[1] = clientOrigin[1] - (fw[1] * 32.0) + (right[1] * 32.0);
	clientTargetOrigin[2] = clientOrigin[2];

	TR_TraceRayFilter(clientOrigin, clientTargetOrigin, MASK_PLAYERSOLID, RayType_EndPoint, StuckFilter, client);
	float flFraction = TR_GetFraction();

	if (GetConVarInt(EBotDebug) == 1)
	{
		TE_SetupBeamPoints(clientOrigin, clientTargetOrigin, m_laserIndex, m_laserIndex, 0, 30, 0.1, 1.0, 1.0, 5, 0.0, (flFraction != 1.0 ? {255, 0, 0, 255} : {0, 0, 255, 255}), 30);
		TE_SendToClient(m_hostEntity);
	}
	
	if (flFraction != 1.0) 
		return true;
	
	return false;
}

// check wall on left
stock bool CheckWallLeft2(const int client)
{
	float right[3], fw[3];
	EyeVectors(client, fw, right);

	float clientOrigin[3];
	clientOrigin = GetCenter(client);

	float clientTargetOrigin[3];
	clientTargetOrigin[0] = clientOrigin[0] - (fw[0] * 32.0) - (right[0] * 32.0);
	clientTargetOrigin[1] = clientOrigin[1] - (fw[1] * 32.0) - (right[1] * 32.0);
	clientTargetOrigin[2] = clientOrigin[2];

	TR_TraceRayFilter(clientOrigin, clientTargetOrigin, MASK_PLAYERSOLID, RayType_EndPoint, StuckFilter, client);
	float flFraction = TR_GetFraction();

	if (GetConVarInt(EBotDebug) == 1)
	{
		TE_SetupBeamPoints(clientOrigin, clientTargetOrigin, m_laserIndex, m_laserIndex, 0, 30, 0.1, 1.0, 1.0, 5, 0.0, (flFraction != 1.0 ? {255, 0, 0, 255} : {0, 0, 255, 255}), 30);
		TE_SendToClient(m_hostEntity);
	}

	if (flFraction != 1.0) 
		return true;
	
	return false;
}

// check line of sight
stock bool CheckForward(const int client)
{
	float right[3], fw[3];
	EyeVectors(client, fw, right);

	float clientOrigin[3];
	GetClientEyePosition(client, clientOrigin);

	float clientTargetOrigin[3];
	clientTargetOrigin[0] = clientOrigin[0] + (fw[0] * 128.0);
	clientTargetOrigin[1] = clientOrigin[1] + (fw[1] * 128.0);
	clientTargetOrigin[2] = clientOrigin[2];

	TR_TraceRayFilter(clientOrigin, clientTargetOrigin, MASK_PLAYERSOLID, RayType_EndPoint, RocketFilter, client);
	float flFraction = TR_GetFraction();

	if (GetConVarInt(EBotDebug) == 1)
	{
		TE_SetupBeamPoints(clientOrigin, clientTargetOrigin, m_laserIndex, m_laserIndex, 0, 30, 0.1, 1.0, 1.0, 5, 0.0, (flFraction == 1.0 ? {255, 0, 0, 255} : {0, 255, 0, 255}), 30);
		TE_SendToClient(m_hostEntity);
	}

	if (flFraction != 1.0) 
		return true;
	
	return false;
}

stock void ClampAngle(float fAngles[3])
{
	if (fAngles[0] > 89.0)
		fAngles[0] -= 360.0;

	if (fAngles[0] < -89.0)
		fAngles[0] += 360.0;

	if (fAngles[1] > 180.0)
		fAngles[1] -= 360.0;

	if (fAngles[1] <-180.0)
		fAngles[1] += 360.0;
}

#define WALK_THRU_PROP_DOORS		0x01
#define WALK_THRU_FUNC_DOORS		0x02
#define WALK_THRU_DOORS 			(WALK_THRU_PROP_DOORS | WALK_THRU_FUNC_DOORS)
#define WALK_THRU_BREAKABLES		0x04
#define WALK_THRU_TOGGLE_BRUSHES	0x08
#define WALK_THRU_EVERYTHING		(WALK_THRU_DOORS | WALK_THRU_BREAKABLES | WALK_THRU_TOGGLE_BRUSHES)

public bool WaypointFilter(int entity, int mask)
{
	char sClass[64];
	GetEntityClassname(entity, sClass, sizeof(sClass));
	
	if (!strcmp(sClass, "worldspawn"))
		return false;
	
	if (!strcmp(sClass, "player"))
		return false;
	
	int iFlags = GetEntityFlags(entity);
	
	if (!strcmp(sClass, "func_door*"))
		return (iFlags & WALK_THRU_PROP_DOORS) ? true : false;
	
	if (!strcmp(sClass, "prop_door*"))
		return (iFlags & WALK_THRU_PROP_DOORS) ? true : false;
	
	if (!strcmp(sClass, "func_brush"))
	{
		int iSolidity = GetEntProp(entity, Prop_Data, "m_iSolidity");
		
		switch (iSolidity)
		{
			case 2: // BRUSHSOLID_ALWAYS
				return false;
			case 1: // BRUSHSOLID_NEVER
				return true;
			case 0: // BRUSHSOLID_TOGGLE
				return (iFlags & WALK_THRU_TOGGLE_BRUSHES) ? true : false;
		}
		
		return (iFlags & WALK_THRU_PROP_DOORS) ? true : false;
	}
	
	if (!strcmp(sClass, "func_breakable") && GetEntProp(entity, Prop_Data, "m_iHealth") && GetEntProp(entity, Prop_Data, "m_takedamage") == 2) // DAMAGE_YES
		return (iFlags & WALK_THRU_BREAKABLES) ? true : false;
	
	if (!strcmp(sClass, "func_playerinfected_clip"))
		return true;

	return entity <= MaxClients;
}

public bool RocketFilter(int entity, int mask)
{
	char class[64];
	GetEntityClassname(entity, class, sizeof(class));
	if (StrEqual(class, "func_respawnroomvisualizer"))
		return false;
	else if (StrContains(class, "tf_projectile_", false) != -1)
		return false;
	else if (StrEqual(class, "entity_revive_marker"))
		return false;
	return entity > MaxClients;
}

stock float GetVectorDistance2D(float vec1[3], float vec2[3])
{
	float x = vec1[0] - vec2[0];
	float y = vec1[1] - vec2[1];
	return SquareRoot(x * x + y * y);
}

// subtracts the given vector from the given vector and return the x + y + z
stock float GetFastDistance(const float vec1[3], const float vec[3])
{
	static float result;
	result = (vec1[0] - vec[0]) + (vec1[1] - vec[1]) + (vec1[2] - vec[2]);
	if (result < 0)
		return -result;

	return result;
}

stock float GetFastDistance2D(const float vec1[3], const float vec[3])
{
	static float result;
	result = (vec1[0] - vec[0]) + (vec1[1] - vec[1]);
	if (result < 0)
		return -result;

	return result;
}

stock int GetFastDistanceInt(int vec1[3], int vec[3])
{
	int result = (vec1[0] - vec[0]) + (vec1[1] - vec[1]) + (vec1[2] - vec[2]);
	if (result < 0)
		return -result;

	return result;
}

stock int GetUsableSniperHintsCount(const int team)
{
	int SniperHintsCount = 0;
	int iEntity;
	while ((iEntity = FindEntityByClassname(iEntity, "func_tfbot_hint")) != -1)
	{
		if (!IsValidEntity(iEntity))
			continue;
			
		if (GetEntProp(iEntity, Prop_Data, "m_hint") != 0)
			continue;
		
		if (GetEntProp(iEntity, Prop_Data, "m_isDisabled") == 1)
			continue;
			
		if (GetTeamNumber(iEntity) == team) // don't use enemy hints. but still can use UNDEFINED TEAM hints. (team != GetTeamNumber(iEntity) is team only hints.)
			continue;
		
		SniperHintsCount++;
	}

	return SniperHintsCount;
}

stock int GetRandomSniperSpot(const int team)
{
	int iEntity;
	int iSelectedHint = -1;
	ArrayList SniperHints = new ArrayList();

	while ((iEntity = FindEntityByClassname(iEntity, "func_tfbot_hint")) != -1)
	{
		if (!IsValidEntity(iEntity))
			continue;
		
		if (GetEntPropEnt(iEntity, Prop_Data, "m_hint") != 0)
			continue;
		
		if (GetEntProp(iEntity, Prop_Data, "m_isDisabled") == 1)
			continue;
			
		if (GetTeamNumber(iEntity) == team)
			continue;
		
		SniperHints.Push(iEntity);
	}

	if (SniperHints.Length > 0)
		iSelectedHint = SniperHints.Get(crandomint(0, SniperHints.Length - 1));

	delete SniperHints;
	return iSelectedHint;
}

stock int TF2_GetObject(const int client, TFObjectType type, TFObjectMode mode)
{
	int iObject = INVALID_ENT_REFERENCE;
	while ((iObject = FindEntityByClassname(iObject, "obj_*")) != -1)
	{
		if (GetEntPropEnt(iObject, Prop_Send, "m_hBuilder") == client && TF2_GetObjectType(iObject) == type && TF2_GetObjectMode(iObject) == mode && !GetEntProp(iObject, Prop_Send, "m_bPlacing") && !GetEntProp(iObject, Prop_Send, "m_bDisposableBuilding"))		
			return iObject;
	}
	
	return iObject;
}

stock float GetPercentInvisible(const int client)
{
	int offset = FindSendPropInfo("CTFPlayer", "m_flInvisChangeCompleteTime") - 8;
	return GetEntDataFloat(client, offset);
}

stock void DisguiseAsEnemy(const int client)
{
	if (isVSH && m_team[client] == 3)
		return;

	ArrayList class = new ArrayList();
	if (m_difficulty[client] < 3)
	{	class.Push(1);
		class.Push(2);
		class.Push(3);
		class.Push(4);
		class.Push(5);
		class.Push(6);
		class.Push(7);
		class.Push(8);
		class.Push(9);
	}
	else
	{	class.Push(3);
		class.Push(4);
		class.Push(6);
		class.Push(7);
		class.Push(8);
		class.Push(9);
	}
	
	TF2_DisguisePlayer(client, (m_team[client] == 2 ? TFTeam_Blue : TFTeam_Red), view_as<TFClassType>(class.Get(crandomint(0, class.Length - 1))));
	delete class;
}

stock bool IsClientAimingToMe(const int client, const int target)
{
	if (ClientViews(target, client, 0.9))
		return true;
		
	return false;
}

stock bool IsClientVisibleInScreen(const int client, const int target)
{
	if (ClientViews(client, target))
		return true;
		
	return false;
}

stock bool IsClientAimingToPosition(const int client, float targetposition[3])
{
	if (ClientViewsOrigin(client, targetposition, 0.9))
		return true;
		
	return false;
}

stock void GetAimOrigin(const int client, float aimorigin[3]) 
{
	if (!IsValidClient(client))
		return;
	
    float angles[3];
	float origin[3];
    GetClientEyePosition(client, origin);
    GetClientEyeAngles(client, angles);

    Handle trace = TR_TraceRayFilterEx(origin, angles, MASK_CUSTOM, RayType_Infinite, TraceEntityFilterStuff);
	TR_GetEndPosition(aimorigin, trace);
    delete trace;
}

stock int Max(const int one, const int two)
{
	if (one > two)
		return one;
	return two;
}

stock int Min(const int one, const int two)
{
	if (one < two)
		return one;
	return two;
}

stock int Abs(const int value)
{
	if (value < 0)
		return -value;
	return value;
}

stock bool HasSapper(const int entity)
{
	if (!HasEntProp(entity, Prop_Send, "m_bHasSapper"))
		return false;

	return GetEntProp(entity, Prop_Send, "m_bHasSapper") != 0;
}

stock bool IsSappable(const int entity)
{
	return HasEntProp(entity, Prop_Send, "m_bHasSapper");
}

stock bool ClientViews(const int Viewer, const int Target, const float fThreshold = 0.70)
{
    static float fViewPos[3];   GetClientEyePosition(Viewer, fViewPos);
    static float fViewAng[3];   GetClientEyeAngles(Viewer, fViewAng);
    static float fViewDir[3];
    static float fTargetPos[3]; GetClientEyePosition(Target, fTargetPos);
    static float fTargetDir[3];
    static float fDistance[3];

    fViewAng[0] = fViewAng[2] = 0.0;
    GetAngleVectors(fViewAng, fViewDir, NULL_VECTOR, NULL_VECTOR);

    fDistance[0] = fTargetPos[0] - fViewPos[0];
    fDistance[1] = fTargetPos[1] - fViewPos[1];
    fDistance[2] = 0.0;

    NormalizeVector(fDistance, fTargetDir);
    if (GetVectorDotProduct(fViewDir, fTargetDir) < fThreshold)
		return false;
    
    return true;
}

stock bool ClientViewsOrigin(const int Viewer, float fTargetPos[3], const float fThreshold = 0.70)
{
    static float fViewPos[3];
	GetClientEyePosition(Viewer, fViewPos);

    static float fViewAng[3];
	GetClientEyeAngles(Viewer, fViewAng);

    static float fViewDir[3];
    static float fTargetDir[3];
    static float fDistance[3];
	
    fViewAng[0] = fViewAng[2] = 0.0;
    GetAngleVectors(fViewAng, fViewDir, NULL_VECTOR, NULL_VECTOR);
    
    fDistance[0] = fTargetPos[0] - fViewPos[0];
    fDistance[1] = fTargetPos[1] - fViewPos[1];
    fDistance[2] = 0.0;

    NormalizeVector(fDistance, fTargetDir);
    if (GetVectorDotProduct(fViewDir, fTargetDir) < fThreshold)
		return false;
    
    return true;
}

stock bool NameAlreadyTakenByPlayer(const char[] name)
{
	int i;
	char buffer[MAX_NAME_LENGTH];
	for (i = 1; i < MaxClients; i++)
	{
		if (!IsValidClient(i))
			continue;

		GetClientName(i, buffer, sizeof(buffer));
		if (!buffer)
			continue;

		if (StrEqual(name, buffer))
			return true;
	}
	
	return false;
}

stock bool AttackToSpy(const int client)
{
	if (isVSH)
		return true;

	if (TF2_IsPlayerInCondition(client, TFCond_CritOnWin))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_CritOnFlagCapture))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_CritOnKill))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_CritOnFirstBlood))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_CritHype))
		return true;

	return false;
}

stock bool ScreetSpy(const int client)
{
	if (TF2_IsPlayerInCondition(client, TFCond_DeadRingered))
		return true;
	
	if (isVSH)
		return false;

	if (TF2_IsPlayerInCondition(client, TFCond_Disguising))
		return false;

	if (TF2_IsPlayerInCondition(client, TFCond_CloakFlicker))
		return false;
	
	if (TF2_IsPlayerInCondition(client, TFCond_OnFire))
		return false;

	if (TF2_IsPlayerInCondition(client, TFCond_Jarated))
		return false;

	if (TF2_IsPlayerInCondition(client, TFCond_Bleeding))
		return false;
	
	if (TF2_IsPlayerInCondition(client, TFCond_Milked))
		return false;
	
	if (TF2_IsPlayerInCondition(client, TFCond_TeleportedGlow))
		return false;

	if (TF2_IsPlayerInCondition(client, TFCond_Sapped))
		return false;

	if (TF2_IsPlayerInCondition(client, TFCond_Gas))
		return false;
	
	if (TF2_IsPlayerInCondition(client, TFCond_Bonked))
		return false;
	
	if (TF2_IsPlayerInCondition(client, TFCond_Disguised))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_Cloaked))
		return true;

	if (TF2_IsPlayerInCondition(client, TFCond_Stealthed))
		return true;

	return false;
}

stock bool IsStealthed(const int client)
{
	if (TF2_IsPlayerInCondition(client, TFCond_Cloaked))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_DeadRingered))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_Stealthed))
		return true;
		
	return TF2_IsPlayerInCondition(client, TFCond_StealthedUserBuffFade);
}

stock bool IsRevealedSpy(const int client)
{
	if (isVSH)
		return true;

	if (IsStealthed(client) && TF2_IsPlayerInCondition(client, TFCond_CloakFlicker))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_Bleeding))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_Bonked))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_Dazed))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_Gas))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_Jarated))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_Milked))
		return true;
	
	return false;
}

stock bool IsRevealedSpyByTarget(const int client, const int target)
{
	if (isVSH)
		return true;

	if (GetVectorDistance(GetOrigin(client), GetOrigin(target), true) <= Squaredf(72.0))
		return true;
	
	if (IsStealthed(client))
		return (GetPercentInvisible(client) < 1.0);
	
	if (!IsStealthed(client) && m_knownSpy[target] == client)
		return true;
	
	if (!TF2_IsPlayerInCondition(client, TFCond_Disguised))
		return true;
	
	if (IsStealthed(client) && TF2_IsPlayerInCondition(client, TFCond_CloakFlicker))
		return true;
	
	if (!TF2_IsPlayerInCondition(client, TFCond_CritOnFirstBlood) && TF2_IsPlayerInCondition(target, TFCond_CritOnFirstBlood))
		return true;
	
	if (!TF2_IsPlayerInCondition(client, TFCond_CritOnWin) && TF2_IsPlayerInCondition(target, TFCond_CritOnWin))
		return true;
	
	if (!TF2_IsPlayerInCondition(client, TFCond_CritOnFlagCapture) && TF2_IsPlayerInCondition(target, TFCond_CritOnFlagCapture))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_Bleeding))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_Bonked))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_Dazed))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_Gas))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_Jarated))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_Milked))
		return true;
	
	return false;
}

stock int GetEnemyControlPointCount(const int client)
{
	int EnemyControlPointCount = 0;
	int iControlPoint;
	
	while ((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
	{
		if (!IsValidEntity(iControlPoint))
			continue;

		if (GetEntProp(iControlPoint, Prop_Data, "m_bLocked") != 0)
			continue;
		
		if (GetClientTeam(client) == GetTeamNumber(iControlPoint))
			continue;
		
		EnemyControlPointCount++;
	}
	
	return EnemyControlPointCount;
}

stock int GetNearestMyControlPoint(const int client)
{
	int NearestControlPoint = -1;
	float distance; 
	float nearestDistance = 999999999999999.0;
	int iControlPoint;
	
	while ((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
	{
		if (!IsValidEntity(iControlPoint))
			continue;
		
		if (GetEntProp(iControlPoint, Prop_Data, "m_bLocked") != 0)
			continue;
		
		if (GetClientTeam(client) != GetTeamNumber(iControlPoint))
			continue;
		
		distance = GetVectorDistance(GetOrigin(client), GetOrigin(iControlPoint), true);
		if (distance < nearestDistance)
		{
			NearestControlPoint = iControlPoint;
			nearestDistance = distance;
		}
	}
	
	return NearestControlPoint;
}

stock int GetMyControlPointCount(const int client)
{
	int MyControlPointCount = 0;
	int iControlPoint;
	
	while ((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
	{
		if (!IsValidEntity(iControlPoint))
			continue;
		
		if (GetEntProp(iControlPoint, Prop_Data, "m_bLocked") != 0)
			continue;
		
		if (GetClientTeam(client) != GetTeamNumber(iControlPoint))
			continue;
		
		MyControlPointCount++;
	}
	
	return MyControlPointCount;
}

stock int GetNearestBluControlPoint()
{
	int iControlPoint;
	while ((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
	{
		if (!IsValidEntity(iControlPoint))
			continue;
		
		if (GetTeamNumber(iControlPoint) != 3)
			continue;
	}
	
	return iControlPoint;
}

stock int GetNearestDefendableControlPoint(const int client)
{
	int NearestControlPoint = -1;
	float distance; 
	float nearestDistance = 99999999999999.0;
	int iControlPoint;
	
	if (GetNearestBluControlPoint() != -1)
	{
		while ((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
		{
			if (!IsValidEntity(iControlPoint))
				continue;
			
			if (GetEntProp(iControlPoint, Prop_Data, "m_bLocked") != 0)
				continue;
			
			if (GetClientTeam(client) != GetTeamNumber(iControlPoint))
				continue;
			
			distance = GetVectorDistance(GetOrigin(GetNearestBluControlPoint()), GetOrigin(iControlPoint), true);
			if (distance < nearestDistance)
			{
				NearestControlPoint = iControlPoint;
				nearestDistance = distance;
			}
		}
	}
	else
	{
		while ((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
		{
			if (!IsValidEntity(iControlPoint))
				continue;
			
			if (GetEntProp(iControlPoint, Prop_Data, "m_bLocked") != 0)
				continue;
			
			if (GetClientTeam(client) != GetTeamNumber(iControlPoint))
				continue;
			
			NearestControlPoint = iControlPoint;
		}
	}
	
	return NearestControlPoint;
}

stock FindEntityByTargetname(const char[] targetname, const char[] classname)
{
	char namebuf[32];
	int index = -1;
	namebuf[0] = '\0';
	
	while (strcmp(namebuf, targetname) != 0 && (index = FindEntityByClassname(index, classname)) != -1)
		GetEntPropString(index, Prop_Data, "m_iName", namebuf, sizeof(namebuf));
	
	return index;
}

stock bool IsVisibleToEnemy(const int client, float origin[3], const float height)
{
    origin[2] += height;
    int i;
    for (i = 1; i <= MaxClients; i++)
    {
        if (!IsValidClient(i))
			continue;
        
        // count deads too, because (almost) every teammate knows where is his friend is died, or died teammate can say where is enemy, etc. so basically even if he's dead this area is not safe
		//if (!m_isAlive[i])
		//	continue;
		
		if (m_team[i] == m_team[client])
			continue;
        
        if (IsVisible(origin, GetEyePosition(i)))
		{
			origin[2] -= height;
			return true;
		}
    }

	origin[2] -= height;
    return false;
}

stock bool IsCloserToEnemy(const int client, const float origin[3])
{
	float distance = GetVectorDistance(GetOrigin(client), origin, true);
    int i;
    for (i = 1; i <= MaxClients; i++)
    {
        if (!IsValidClient(i))
			continue;
        
        // count deads too, because (almost) every teammate knows where is his friend is died, or died teammate can say where is enemy, etc. so basically even if he's dead this area is not safe
		//if (!m_isAlive[i])
		//	continue;
		
		if (m_team[i] == m_team[client])
			continue;
        
        if (distance > GetVectorDistance(GetOrigin(i), origin, true))
            return true;
    }

    return false;
}

stock bool IsDangerous(const int client, float origin[3], const float height)
{
    origin[2] += height;
	float distance = GetVectorDistance(GetOrigin(client), origin, true);
    int i;
    for (i = 1; i <= MaxClients; i++)
    {
        if (!IsValidClient(i))
			continue;
        
        // count deads too, because (almost) every teammate knows where is his friend is died, or died teammate can say where is enemy, etc. so basically even if he's dead this area is not safe
		//if (!m_isAlive[i])
		//	continue;
		
		if (m_team[i] > 1 && m_team[i] == m_team[client])
			continue;
        
        if (distance > GetVectorDistance(GetOrigin(i), origin, true))
		{
			origin[2] -= height;
			return true;
		}

		if (IsVisible(origin, GetEyePosition(i)))
		{
			origin[2] -= height;
			return true;
		}
    }

	origin[2] -= height;
    return false;
}