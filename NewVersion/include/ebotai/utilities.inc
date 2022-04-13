char currentMap[PLATFORM_MAX_PATH];

int healthpacks = -1;
int ammopacks = -1;

int m_laserIndex = -1;
int m_beamIndex = -1;
int m_hostEntity = -1;
bool m_hasHostEntity = false;

float m_redFlagCapPoint[3];
float m_bluFlagCapPoint[3];

int m_targetNode[MAXPLAYERS + 1];
int m_currentWaypointIndex[MAXPLAYERS + 1];
float m_nextPathUpdate[MAXPLAYERS + 1];
float m_nextStuckCheck[MAXPLAYERS + 1];
ArrayList m_positions[MAXPLAYERS + 1];
ArrayList m_pathIndex[MAXPLAYERS + 1];
float m_pauseTime[MAXPLAYERS + 1];

int m_buttons[MAXPLAYERS + 1];
float m_thinkTimer[MAXPLAYERS + 1];
float m_lookAt[MAXPLAYERS + 1][3];
float m_pathAhead[MAXPLAYERS + 1][3];
float m_movePosition[MAXPLAYERS + 1][3];
float m_moveVel[MAXPLAYERS + 1][3];

int m_goalEntity[MAXPLAYERS + 1];	
float m_goalPosition[MAXPLAYERS + 1][3];

float m_attackTimer[MAXPLAYERS + 1];
float m_attack2Timer[MAXPLAYERS + 1];

bool m_isSlowThink[MAXPLAYERS + 1];
bool m_canAttack[MAXPLAYERS+ 1 ];

bool m_hasEnemiesNear[MAXPLAYERS + 1];
bool m_hasFriendsNear[MAXPLAYERS + 1];

int m_enemiesNearCount[MAXPLAYERS + 1];
int m_friendsNearCount[MAXPLAYERS + 1];

int m_nearestEnemy[MAXPLAYERS + 1];
int m_nearestFriend[MAXPLAYERS + 1];

float m_enemyDistance[MAXPLAYERS + 1];
float m_friendDistance[MAXPLAYERS + 1];

bool m_lowHealth[MAXPLAYERS + 1];
bool m_lowAmmo[MAXPLAYERS + 1];

int m_eBotSenseChance[MAXPLAYERS + 1];
int m_eBotDodgeRange[MAXPLAYERS + 1];

int m_knownSpy[MAXPLAYERS + 1];
int m_maxSticky[MAXPLAYERS + 1];

Handle EBotDebug;
Handle EBotFPS;
Handle EBotMelee;
Handle EBotSenseMax;
Handle EBotSenseMin;
Handle m_eBotDodgeRangeMin;
Handle m_eBotDodgeRangeMax;
Handle m_eBotDodgeRangeChance;

Handle g_hLookupBone;
Handle g_hGetBonePosition;

#define	MASK_CUSTOM (CONTENTS_SOLID|CONTENTS_MOVEABLE|0x40|CONTENTS_MONSTER)

void InitGamedata()
{
	StartPrepSDKCall(SDKCall_Entity);
	PrepSDKCall_SetSignature(SDKLibrary_Server, "\x55\x8B\xEC\x56\x8B\xF1\x80\xBE\x41\x03\x00\x00\x00\x75\x2A\x83\xBE\x6C\x04\x00\x00\x00\x75\x2A\xE8\x2A\x2A\x2A\x2A\x85\xC0\x74\x2A\x8B\xCE\xE8\x2A\x2A\x2A\x2A\x8B\x86\x6C\x04\x00\x00\x85\xC0\x74\x2A\x83\x38\x00\x74\x2A\xFF\x75\x08\x50\xE8\x2A\x2A\x2A\x2A\x83\xC4\x08\x5E", 68);
	PrepSDKCall_AddParameter(SDKType_String, SDKPass_Pointer);
	PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);
	if ((g_hLookupBone = EndPrepSDKCall()) == INVALID_HANDLE) SetFailState("Failed to create SDKCall for CBaseAnimating::LookupBone signature!");
	
	StartPrepSDKCall(SDKCall_Entity);
	PrepSDKCall_SetSignature(SDKLibrary_Server, "\x55\x8B\xEC\x83\xEC\x30\x56\x8B\xF1\x80\xBE\x41\x03\x00\x00\x00", 16);
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
	PrepSDKCall_AddParameter(SDKType_Vector, SDKPass_ByRef, _, VENCODE_FLAG_COPYBACK);
	PrepSDKCall_AddParameter(SDKType_QAngle, SDKPass_ByRef, _, VENCODE_FLAG_COPYBACK);
	if ((g_hGetBonePosition = EndPrepSDKCall()) == INVALID_HANDLE) SetFailState("Failed to create SDKCall for CBaseAnimating::GetBonePosition signature!");
}

// repeat all clients and get host entity
stock void FindHostEntity()
{
	for (int i = 1; i <= MaxClients; i++)
	{
		// if not a bot
		if (IsValidClient(i) && !IsFakeClient(i))
		{
			// set host entity
			m_hostEntity = i;
			m_hasHostEntity = true;
		}
	}
}

stock int GetTeamsCount(int team)
{
	int count = 0;
	for (int i = 1; i <= MaxClients; i++)
		if (IsValidClient(i) && GetClientTeam(i) == team)
			count++;
	return count;
}

stock int GetBotCount()
{
	int count = 0;
	for (int i = 1; i <= MaxClients; i++)
		if (IsValidClient(i) && IsFakeClient(i))
			count++;
	return count;
}

stock int GetPlayersCount()
{
	int count = 0;
	for (int i = 1; i <= MaxClients; i++)
		if (IsValidClient(i) && !IsFakeClient(i) && GetClientTeam(i) != 1)
			count++;
	return count;
}

stock int GetPlayersCountRed()
{
	int count = 0;
	for (int i = 1; i <= MaxClients; i++)
		if (IsValidClient(i) && GetClientTeam(i) == 2)
			count++;
	return count;
}

stock int GetPlayersCountBlu()
{
	int count = 0;
	for (int i = 1; i <= MaxClients; i++)
		if (IsValidClient(i) && GetClientTeam(i) == 3)
			count++;
	return count;
}

stock int GetTotalPlayersCount()
{
	int count = 0;
	for (int i = 1; i <= MaxClients; i++)
		if (IsValidClient(i) && GetClientTeam(i) != 1)
			count++;
	return count;
}

// squared int value
stock int Squared(int number)
{
	return number * number;
}

// squared float value
stock float Squaredf(float number)
{
	return number * number;
}

stock float[] GetEyePosition(int client)
{
	float vector[3];
	GetClientEyePosition(client, vector);
	return vector;
}

stock bool ChanceOf(int number)
{
	return GetRandomInt(1, 100) <= number;
}

stock float[] GetOrigin(int entity)
{
	float vector[3];
	GetEntPropVector(entity, Prop_Data, "m_vecAbsOrigin", vector);
	return vector;
}

stock int[] VectorAsInt(float origin[3])
{
	int vector[3];
	vector[0] = RoundToFloor(origin[0]);
	vector[1] = RoundToFloor(origin[1]);
	vector[2] = RoundToFloor(origin[2]);
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

stock float[] VectorWithRoot(int origin[3], int root1 = 0, int root2 = 0, int root3 = 0)
{
	float vector[3];
	vector[0] = float(origin[0] + root1);
	vector[1] = float(origin[1] + root2);
	vector[2] = float(origin[2] + root3);
	return vector;
}

stock void EyeVectors(int client, float fw[3] = NULL_VECTOR, float right[3] = NULL_VECTOR, float up[3] = NULL_VECTOR)
{
	GetAngleVectors(GetEyeAngles(client), fw, right, up);
}

stock float[] GetEyeAngles(int client)
{
	float vector[3];
	GetClientEyeAngles(client, vector);
	return vector;
}

stock float GetCenter(int entity)
{
	float result[3], maxs[3], origin[3];
	GetEntPropVector(entity, Prop_Send, "m_vecOrigin", origin);
	GetEntPropVector(entity, Prop_Send, "m_vecMaxs", maxs);
	result[0] = origin[0];
	result[1] = origin[1];
	result[2] = origin[2] += (maxs[2] / 2);
	return result;
}

stock void EquipWeaponSlot(int client, int slot)
{
	if (IsWeaponSlotActive(client, slot))
		return;
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

stock int LookupBone(int iEntity, const char[] szName)
{
	return SDKCall(g_hLookupBone, iEntity, szName);
}

stock void GetBonePosition(int iEntity, int iBone, float origin[3], float angles[3])
{
	SDKCall(g_hGetBonePosition, iEntity, iBone, origin, angles);
}

stock int FindNearestEnemy(int client, float flMaxDistance = 999999.0)
{
	float flBestDistance = Squaredf(flMaxDistance);
	int iBestTarget = -1;
	for (int i = 1; i <= MaxClients; i++)
	{
		if (client == i)
			continue;

		if (!IsValidClient(i))
			continue;
		
		if (!IsPlayerAlive(i))
			continue;
		
		if (GetClientTeam(i) == GetClientTeam(client))
			continue;
		
		float flDistance = flMaxDistance == 999999.0 ? GetFastDistance(GetOrigin(client), GetOrigin(i)) : GetVectorDistance(GetOrigin(client), GetOrigin(i), true);
		if (flDistance < flBestDistance)
		{
			flBestDistance = flDistance;
			iBestTarget = i;
		}
	}
	return iBestTarget;
}

float m_itAimStart[MAXPLAYERS];
stock void LookAtPosition(int client, float flGoal[3], float angle[3])
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
	
	if (!m_hasEnemiesNear[client] && cos_error > 0.98)
		m_itAimStart[client] = GetGameTime();
	
	float new_eye_angle[3];
	new_eye_angle[0] = ApproachAngle(ang_to_target[0], eye_ang[0], (max_angvel * GetGameFrameTime()));
	new_eye_angle[1] = ApproachAngle(ang_to_target[1], eye_ang[1], (max_angvel * GetGameFrameTime()));
	
	new_eye_angle[0] = AngleNormalize(new_eye_angle[0]);
	new_eye_angle[1] = AngleNormalize(new_eye_angle[1]);
	
	angle = new_eye_angle;
	TeleportEntity(client, NULL_VECTOR, new_eye_angle, NULL_VECTOR);
}

stock float AngleNormalize(float angle)
{
	angle = angle - 360.0 * RoundToFloor(angle / 360.0);
	while (angle > 180.0) angle -= 360.0;
	while (angle < -180.0) angle += 360.0;
	return angle;
}

stock float AngleDiff(float destAngle, float srcAngle)
{
	return AngleNormalize(destAngle - srcAngle);
}

stock float ApproachAngle(float target, float value, float speed)
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

stock void MoveTo(int client, float flGoal[3], bool checkStuck = true)
{
	m_movePosition[client] = flGoal;

	float flPos[3];
	GetClientAbsOrigin(client, flPos);

	float fAng[3];
	GetClientEyeAngles(client, fAng);
	
	float newmove[3];
	SubtractVectors(flGoal, flPos, newmove);
	
	newmove[1] = -newmove[1];
	
	float sin = Sine(fAng[1] * FLOAT_PI / 180.0);
	float cos = Cosine(fAng[1] * FLOAT_PI / 180.0);
	
	m_moveVel[client][0] = cos * newmove[0] - sin * newmove[1];
	m_moveVel[client][1] = sin * newmove[0] + cos * newmove[1];
	
	NormalizeVector(m_moveVel[client], m_moveVel[client]);
	ScaleVector(m_moveVel[client], 500.0);

	if (checkStuck && GetEntityFlags(client) & FL_ONGROUND)
	{
		bool right = CheckWallRight(client);
		bool left = CheckWallLeft(client);
		bool right2 = CheckWallRight2(client);
		bool left2 = CheckWallLeft2(client);

		if (right && !right2)
			m_moveVel[client][1] = -500.0;
		
		if (left && !left2)
			m_moveVel[client][1] = 500.0;

		if (right2 && !right)
			m_moveVel[client][1] = -500.0;
		
		if (left2 && !left)
			m_moveVel[client][1] = 500.0;
	}
}

stock void MoveOut(int client, float flGoal[3], bool checkStuck = true)
{
	m_movePosition[client] = flGoal;

	float flPos[3];
	GetClientAbsOrigin(client, flPos);

	float fAng[3];
	GetClientEyeAngles(client, fAng);
	
	float newmove[3];
	SubtractVectors(flGoal, flPos, newmove);
	
	float sin = Sine(fAng[1] * FLOAT_PI / 180.0);
	float cos = Cosine(fAng[1] * FLOAT_PI / 180.0);
	
	m_moveVel[client][0] = cos * newmove[0] - sin * newmove[1];
	m_moveVel[client][1] = sin * newmove[0] + cos * newmove[1];
	
	NormalizeVector(m_moveVel[client], m_moveVel[client]);
	ScaleVector(m_moveVel[client], 500.0);

	if (checkStuck && GetEntityFlags(client) & FL_ONGROUND)
	{
		bool right = CheckWallRight(client);
		bool left = CheckWallLeft(client);
		bool right2 = CheckWallRight2(client);
		bool left2 = CheckWallLeft2(client);

		if (right && !right2)
			m_moveVel[client][1] = 500.0;
		
		if (left && !left2)
			m_moveVel[client][1] = -500.0;

		if (right2 && !right)
			m_moveVel[client][1] = 500.0;
		
		if (left2 && !left)
			m_moveVel[client][1] = -500.0;
	}
}

stock void BackstabMove(int client, int target, bool checkStuck = true)
{
	float flBotAng[3], flTargetAng[3];
	GetClientEyeAngles(client, flBotAng);
	GetClientEyeAngles(target, flTargetAng);
	float iAngleDiff = AngleDiff(flBotAng[1], flTargetAng[1]);
	
	if (GetClientAimTarget(target) > 0)
		m_moveVel[client][0] = -500.0;
	else
		m_moveVel[client][0] = 500.0;
	
	if (iAngleDiff > 90)
		m_moveVel[client][1] = 500.0;
	else if (iAngleDiff < -90)
		m_moveVel[client][1] = -500.0;
	else
		m_moveVel[client][0] = -500.0;
}

stock bool IsValidClient(int client)
{
	if (client <= 0 || client > MaxClients)
		return false;
	
	if (!IsClientInGame(client))
		return false;
	
	return true;
}

stock bool IsVisible(float start[3], float end[3])
{
	Handle trace = TR_TraceRayFilterEx(start, end, MASK_CUSTOM, RayType_EndPoint, TraceEntityFilterStuff);
	float flFraction = TR_GetFraction(trace);
	delete trace;
	if (flFraction >= 1.0) 
		return !(flFraction == 0.0);
	return false;
}

stock bool IsVisibleWaypoint(int start[3], int end[3])
{
	float fstart[3];
	float fend[3];
	fstart[0] = float(start[0]);
	fstart[1] = float(start[1]);
	fstart[2] = float(start[2] + 18);
	fend[0] = float(end[0]);
	fend[1] = float(end[1]);
	fend[2] = float(end[2] + 18);
	Handle trace = TR_TraceRayFilterEx(fstart, fend, MASK_CUSTOM, RayType_EndPoint, TraceEntityFilterStuff);
	float flFraction = TR_GetFraction(trace);
	delete trace;
	if (flFraction >= 1.0) 
		return !(flFraction == 0.0);
	return false;
}

public bool TraceEntityFilterStuff(int entity, int mask)
{
	char class[64];
	GetEntityClassname(entity, class, sizeof(class));
	if (StrEqual(class, "entity_medigun_shield"))
		return false;
	else if (StrEqual(class, "func_respawnroomvisualizer"))
		return false;
	else if (StrContains(class, "tf_projectile_", false) != -1)
		return false;
	else if (StrContains(class, "obj_", false) != -1)
		return false;
	else if (StrEqual(class, "entity_revive_marker"))
		return false;
	return entity > MaxClients;
}

stock int TF2_GetNumberOfHealers(client)
{
    return GetEntProp(client, Prop_Send, "m_nNumHealers");
}

stock int GetTeamNumber(int entity)
{
	return GetEntProp(entity, Prop_Send, "m_iTeamNum");
}

stock int GetOwnerEntity(int entity)
{
	return GetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity");
}

stock int GetEntityThrower(int entity)
{
	return GetEntPropEnt(entity, Prop_Send, "m_hThrower");
}

stock bool IsWeaponSlotActive(int client, int slot)
{
    return GetPlayerWeaponSlot(client, slot) == GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
}

stock int GetActiveWeaponSlot(int client)
{
	int active = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
	if (active == -1)
		return -1;
	for (int i = 0; i < 5; i++)
	{
		if (GetPlayerWeaponSlot(client, i) == active)
			return i;
	}
	return -1;
}

stock int GetMetal(int client)
{
	return GetEntProp(client, Prop_Data, "m_iAmmo", 4, 3);
}

stock bool TF2_HasTheFlag(int client)
{
    int ent = -1;
    while ((ent = FindEntityByClassname(ent, "item_teamflag")) != -1)
    {
        if (GetEntPropEnt(ent, Prop_Data, "m_hOwnerEntity") == client)
            return true;
    }
    return false;
}


stock bool IsReloading(int client)
{
	return GetEntProp(GetEntPropEnt(client, Prop_Data, "m_hActiveWeapon"), Prop_Data, "m_bInReload") != 0;
}

stock float GetMaxSpeed(int entity)
{
	return GetEntPropFloat(entity, Prop_Send, "m_flMaxspeed");
}

stock int TF2_GetPlayerWeaponID(int client, int slot)
{
	int validslot = GetPlayerWeaponSlot(client, slot);
	if (!IsValidEntity(validslot))
		return -1;
	return GetEntProp(validslot, Prop_Send, "m_iItemDefinitionIndex");
}

stock int TF2_GetHealingTarget(int client)
{
	int index = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
	return GetEntPropEnt(index, Prop_Send, "m_hHealingTarget");
}

stock int GetMaxHealth(int entity)
{
	return GetEntProp(entity, Prop_Data, "m_iMaxHealth");
}

stock bool IsMoving(int client)
{
	float buffer[3];
	GetEntPropVector(client, Prop_Data, "m_vecAbsVelocity", buffer);
	return (GetVectorLength(buffer) > 10.0);
}

stock bool IsResupply(int entity, int team)
{
	char model[PLATFORM_MAX_PATH];
	if (GetTeamNumber(entity) == team)
	{
		GetEntPropString(entity, Prop_Data, "m_ModelName", model, sizeof(model));
		return StrEqual(model, "models/props_gameplay/resupply_locker.mdl");
	}
	return false;
}

stock bool IsAttacking(int client)
{
	if (GetClientButtons(client) == IN_ATTACK)
		return true;
	return false;
}

stock float GetHeight(int client)
{
	float eyepos[3]; eyepos = GetEyePosition(client);
	float origin[3]; origin = GetOrigin(client);
	return (eyepos[2] - origin[2]);
}

stock int GetAmmo(int client, int slot)
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

stock int GetPrimaryClip(int client, int slot)
{
	int weapon = GetPlayerWeaponSlot(client, slot);
	if (!IsValidEntity(weapon))
		return 9999;
	return GetEntProp(weapon, Prop_Send, "m_iClip1");
}

stock int GetPrimaryClipN(int client, int slot)
{
	int weapon = GetPlayerWeaponSlot(client, slot);
	if (!IsValidEntity(weapon))
		return -1;
	return GetEntProp(weapon, Prop_Send, "m_iClip1");
}

// checks wall
stock bool CheckWall(int client)
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
	Handle TraceRay = TR_TraceHullFilterEx(flPos, flPos, flMins, flMaxs, MASK_PLAYERSOLID, ExcludeFilter, client);
	bool bHit = TR_DidHit(TraceRay);
	
	delete TraceRay;
	return bHit;
}

public bool ExcludeFilter(int entity, int contentsMask, any iExclude)
{
	char class[64];
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

stock int GetPlayerStickyCount(int client)
{
	int StickyCount = 0;
	for (int x = 0; x <= GetMaxEntities(); x++)
	{
		if (!IsValidEntity(x))
			continue;
		
		if (!HasEntProp(x, Prop_Send, "m_hThrower"))
			continue;
		
		if (GetEntityThrower(x) != client)
			continue;
		
		char ClassName[32];
		GetEdictClassname(x, ClassName, 32);
		if (StrContains(ClassName, "tf_projectile_pipe_remote", false) != -1)
			StickyCount++;
	}
	return StickyCount;
}

// check wall on right
stock bool CheckWallRight(int client)
{
	float right[3], fw[3];
	EyeVectors(client, fw, right);

	float clientOrigin[3];
	GetClientEyePosition(client, clientOrigin);

	float clientTargetOrigin[3];
	clientTargetOrigin[0] = clientOrigin[0] + (fw[0] * 28.0) + (right[0] * 28.0);
	clientTargetOrigin[1] = clientOrigin[1] + (fw[1] * 28.0) + (right[1] * 28.0);
	clientTargetOrigin[2] = clientOrigin[2];

	Handle tr = TR_TraceRayFilterEx(clientOrigin, clientTargetOrigin, MASK_PLAYERSOLID, RayType_EndPoint, ExcludeFilter, client);
	bool hit = TR_DidHit(tr);

	if (GetConVarInt(EBotDebug) == 1)
	{
		TE_SetupBeamPoints(clientOrigin, clientTargetOrigin, m_laserIndex, m_laserIndex, 0, 30, 0.1, 1.0, 1.0, 5, 0.0, (hit ? {255, 0, 0, 255} : {0, 0, 255, 255}), 30);
		TE_SendToClient(m_hostEntity);
	}
	
	delete tr;
	return hit;
}

// check wall on left
stock bool CheckWallLeft(int client)
{
	float right[3], fw[3];
	EyeVectors(client, fw, right);

	float clientOrigin[3];
	GetClientEyePosition(client, clientOrigin);

	float clientTargetOrigin[3];
	clientTargetOrigin[0] = clientOrigin[0] + (fw[0] * 28.0) - (right[0] * 28.0);
	clientTargetOrigin[1] = clientOrigin[1] + (fw[1] * 28.0) - (right[1] * 28.0);
	clientTargetOrigin[2] = clientOrigin[2];

	Handle tr = TR_TraceRayFilterEx(clientOrigin, clientTargetOrigin, MASK_PLAYERSOLID, RayType_EndPoint, ExcludeFilter, client);
	bool hit = TR_DidHit(tr);

	if (GetConVarInt(EBotDebug) == 1)
	{
		TE_SetupBeamPoints(clientOrigin, clientTargetOrigin, m_laserIndex, m_laserIndex, 0, 30, 0.1, 1.0, 1.0, 5, 0.0, (hit ? {255, 0, 0, 255} : {0, 0, 255, 255}), 30);
		TE_SendToClient(m_hostEntity);
	}

	delete tr;
	return hit;
}

// check wall on right
stock bool CheckWallRight2(int client)
{
	float right[3], fw[3];
	EyeVectors(client, fw, right);

	float clientOrigin[3];
	GetClientEyePosition(client, clientOrigin);

	float clientTargetOrigin[3];
	clientTargetOrigin[0] = clientOrigin[0] - (fw[0] * 28.0) + (right[0] * 28.0);
	clientTargetOrigin[1] = clientOrigin[1] - (fw[1] * 28.0) + (right[1] * 28.0);
	clientTargetOrigin[2] = clientOrigin[2];

	Handle tr = TR_TraceRayFilterEx(clientOrigin, clientTargetOrigin, MASK_PLAYERSOLID, RayType_EndPoint, ExcludeFilter, client);
	bool hit = TR_DidHit(tr);

	if (GetConVarInt(EBotDebug) == 1)
	{
		TE_SetupBeamPoints(clientOrigin, clientTargetOrigin, m_laserIndex, m_laserIndex, 0, 30, 0.1, 1.0, 1.0, 5, 0.0, (hit ? {255, 0, 0, 255} : {0, 0, 255, 255}), 30);
		TE_SendToClient(m_hostEntity);
	}
	
	delete tr;
	return hit;
}

// check wall on left
stock bool CheckWallLeft2(int client)
{
	float right[3], fw[3];
	EyeVectors(client, fw, right);

	float clientOrigin[3];
	GetClientEyePosition(client, clientOrigin);

	float clientTargetOrigin[3];
	clientTargetOrigin[0] = clientOrigin[0] - (fw[0] * 28.0) - (right[0] * 28.0);
	clientTargetOrigin[1] = clientOrigin[1] - (fw[1] * 28.0) - (right[1] * 28.0);
	clientTargetOrigin[2] = clientOrigin[2];

	Handle tr = TR_TraceRayFilterEx(clientOrigin, clientTargetOrigin, MASK_PLAYERSOLID, RayType_EndPoint, ExcludeFilter, client);
	bool hit = TR_DidHit(tr);

	if (GetConVarInt(EBotDebug) == 1)
	{
		TE_SetupBeamPoints(clientOrigin, clientTargetOrigin, m_laserIndex, m_laserIndex, 0, 30, 0.1, 1.0, 1.0, 5, 0.0, (hit ? {255, 0, 0, 255} : {0, 0, 255, 255}), 30);
		TE_SendToClient(m_hostEntity);
	}

	delete tr;
	return hit;
}

// check line of sight
stock bool CheckForward(int client)
{
	float right[3], fw[3];
	EyeVectors(client, fw, right);

	float clientOrigin[3];
	GetClientEyePosition(client, clientOrigin);

	float clientTargetOrigin[3];
	clientTargetOrigin[0] = clientOrigin[0] + (fw[0] * 128.0);
	clientTargetOrigin[1] = clientOrigin[1] + (fw[1] * 128.0);
	clientTargetOrigin[2] = clientOrigin[2];

	Handle tr = TR_TraceRayFilterEx(clientOrigin, clientTargetOrigin, MASK_PLAYERSOLID, RayType_EndPoint, RocketFilter, client);
	bool hit = TR_DidHit(tr);

	if (GetConVarInt(EBotDebug) == 1)
	{
		TE_SetupBeamPoints(clientOrigin, clientTargetOrigin, m_laserIndex, m_laserIndex, 0, 30, 0.1, 1.0, 1.0, 5, 0.0, (hit ? {255, 0, 0, 255} : {0, 255, 0, 255}), 30);
		TE_SendToClient(m_hostEntity);
	}
	
	delete tr;
	return hit;
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

// subtracts the given vector from the given vector and return the x + y + z
stock float GetFastDistance(float vec1[3], float vec[3])
{
	float result = (vec1[0] - vec[0]) + (vec1[1] - vec[1]) + (vec1[2] - vec[2]);
	if (result < 0)
		result = -result;
	return result;
}

stock int GetUsableSniperHintsCount(int team)
{
	int SniperHintsCount = 0;
	int iEntity = -1;
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

stock int GetRandomSniperSpot(int team)
{
	int iEntity = -1;
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
		iSelectedHint = SniperHints.Get(GetRandomInt(0, SniperHints.Length - 1));

	delete SniperHints;
	return iSelectedHint;
}

stock int TF2_GetObject(int client, TFObjectType type, TFObjectMode mode)
{
	int iObject = INVALID_ENT_REFERENCE;
	while ((iObject = FindEntityByClassname(iObject, "obj_*")) != -1)
	{
		TFObjectType iObjType = TF2_GetObjectType(iObject);
		TFObjectMode iObjMode = TF2_GetObjectMode(iObject);
		if (GetEntPropEnt(iObject, Prop_Send, "m_hBuilder") == client && iObjType == type && iObjMode == mode && !GetEntProp(iObject, Prop_Send, "m_bPlacing") && !GetEntProp(iObject, Prop_Send, "m_bDisposableBuilding"))		
			return iObject;
	}
	return iObject;
}

stock float GetPercentInvisible(int client)
{
	int offset = FindSendPropInfo("CTFPlayer", "m_flInvisChangeCompleteTime") - 8;
	return GetEntDataFloat(client, offset);
}

stock bool IsClientAimingToMe(int client, int target)
{
	if (ClientViews(target, client, 0.9))
		return true;
		
	return false;
}

stock bool IsClientVisibleInScreen(int client, int target)
{
	if (ClientViews(client, target, 0.6))
		return true;
		
	return false;
}

stock bool IsClientAimingToPosition(int client, float targetposition[3])
{
	if (ClientViewsOrigin(client, targetposition, 0.9))
		return true;
		
	return false;
}

stock float Max(float one, float two)
{
	if (one > two)
		return one;
	else if (two > one)
		return two;
	return two;
}

stock float Min(float one, float two)
{
	if (one < two)
		return one;
	else if (two < one)
		return two;
	return two;
}

stock bool ClientViews(int Viewer, int Target, float fMaxDistance = 0.0, float fThreshold = 0.70)
{
    float fViewPos[3];   GetClientEyePosition(Viewer, fViewPos);
    float fViewAng[3];   GetClientEyeAngles(Viewer, fViewAng);
    float fViewDir[3];
    float fTargetPos[3]; GetClientEyePosition(Target, fTargetPos);
    float fTargetDir[3];
    float fDistance[3];

    fViewAng[0] = fViewAng[2] = 0.0;
    GetAngleVectors(fViewAng, fViewDir, NULL_VECTOR, NULL_VECTOR);

    fDistance[0] = fTargetPos[0] - fViewPos[0];
    fDistance[1] = fTargetPos[1] - fViewPos[1];
    fDistance[2] = 0.0;
    if (fMaxDistance != 0.0)
    {
        if ((Squaredf(fDistance[0]) + Squaredf(fDistance[1])) >= Squaredf(fMaxDistance))
            return false;
    }

    NormalizeVector(fDistance, fTargetDir);
    if (GetVectorDotProduct(fViewDir, fTargetDir) < fThreshold)
		return false;
    
    return true;
}

stock bool ClientViewsOrigin(int Viewer, float fTargetPos[3], float fMaxDistance=0.0, float fThreshold=0.70)
{
    float fViewPos[3];   GetClientEyePosition(Viewer, fViewPos);
    float fViewAng[3];   GetClientEyeAngles(Viewer, fViewAng);
    float fViewDir[3];
    float fTargetDir[3];
    float fDistance[3];
	
    fViewAng[0] = fViewAng[2] = 0.0;
    GetAngleVectors(fViewAng, fViewDir, NULL_VECTOR, NULL_VECTOR);
    
    fDistance[0] = fTargetPos[0] - fViewPos[0];
    fDistance[1] = fTargetPos[1] - fViewPos[1];
    fDistance[2] = 0.0;
    if (fMaxDistance != 0.0)
    {
        if ((Squaredf(fDistance[0]) + Squaredf(fDistance[1])) >= Squaredf(fMaxDistance))
            return false;
    }

    NormalizeVector(fDistance, fTargetDir);
    if (GetVectorDotProduct(fViewDir, fTargetDir) < fThreshold)
		return false;
    
    return true;
}

stock bool AttackToSpy(int client)
{
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

stock bool ScreetSpy(int client)
{
	if (TF2_IsPlayerInCondition(client, TFCond_DeadRingered))
		return true;
	
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

stock bool IsStealthed(int client)
{
	if (TF2_IsPlayerInCondition(client, TFCond_Cloaked))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_DeadRingered))
		return true;
	
	if (TF2_IsPlayerInCondition(client, TFCond_Stealthed))
		return true;
		
	return TF2_IsPlayerInCondition(client, TFCond_StealthedUserBuffFade);
}

stock bool IsRevealedSpy(int client)
{
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

stock bool IsRevealedSpyByTarget(int client, target)
{
	if (GetVectorDistance(GetOrigin(client), GetOrigin(target), true) <= Squared(80))
		return true;
	
	if (IsStealthed(client))
		return (GetPercentInvisible(client) <= 0.80);
	
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
