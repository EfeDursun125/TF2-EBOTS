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

int SentryGun[TFMaxPlayers];
int Dispenser[TFMaxPlayers];

int m_tackEntity[TFMaxPlayers];

int CurrentProcess[TFMaxPlayers];

int currentActiveArea;
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

float m_itAimStart[TFMaxPlayers];
float m_cosError[TFMaxPlayers];

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

Handle g_hLookupBone;
Handle g_hGetBonePosition;
Handle g_hSnapEyeAngles;
Handle g_hCustomTaunt;

bool m_isAFK[TFMaxPlayers];

Handle gH_RestartTimer = null;
int gI_RestartTimerIteration = 0;

bool m_isEBot[TFMaxPlayers];

stock bool IsEBot(int bot)
{
	if (m_isEBot[bot])
		return true;
	
	return false;
}

#define	MASK_CUSTOM (CONTENTS_SOLID|CONTENTS_MOVEABLE|0x40|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS)

public void InitGamedata()
{
	StartPrepSDKCall(SDKCall_Entity);
	PrepSDKCall_SetSignature(SDKLibrary_Server, "\x55\x8B\xEC\x56\x8B\xF1\x80\xBE\x41\x03\x00\x00\x00\x75\x2A\x83\xBE\x6C\x04\x00\x00\x00\x75\x2A\xE8\x2A\x2A\x2A\x2A\x85\xC0\x74\x2A\x8B\xCE\xE8\x2A\x2A\x2A\x2A\x8B\x86\x6C\x04\x00\x00\x85\xC0\x74\x2A\x83\x38\x00\x74\x2A\xFF\x75\x08\x50\xE8\x2A\x2A\x2A\x2A\x83\xC4\x08\x5E", 68);
	PrepSDKCall_AddParameter(SDKType_String, SDKPass_Pointer);
	PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);
	if ((g_hLookupBone = EndPrepSDKCall()) == null)
		g_hLookupBone = null;
	
	StartPrepSDKCall(SDKCall_Entity);
	PrepSDKCall_SetSignature(SDKLibrary_Server, "\x55\x8B\xEC\x83\xEC\x30\x56\x8B\xF1\x80\xBE\x41\x03\x00\x00\x00", 16);
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
	PrepSDKCall_AddParameter(SDKType_Vector, SDKPass_ByRef, _, VENCODE_FLAG_COPYBACK);
	PrepSDKCall_AddParameter(SDKType_QAngle, SDKPass_ByRef, _, VENCODE_FLAG_COPYBACK);
	if ((g_hGetBonePosition = EndPrepSDKCall()) == null)
		g_hGetBonePosition = null;

	StartPrepSDKCall(SDKCall_Player);
	PrepSDKCall_SetSignature(SDKLibrary_Server, "\x55\x8B\xEC\x8B\x45\x08\xD9\x00\xD9\x99\x58\x0A\x00\x00", 14);
	PrepSDKCall_AddParameter(SDKType_QAngle, SDKPass_ByRef);
	if ((g_hSnapEyeAngles = EndPrepSDKCall()) == null)
	{
		StartPrepSDKCall(SDKCall_Player);
		PrepSDKCall_SetSignature(SDKLibrary_Server, "@_ZN11CBasePlayer13SnapEyeAnglesERK6QAngle", 1);
		PrepSDKCall_AddParameter(SDKType_QAngle, SDKPass_ByRef);
		if ((g_hSnapEyeAngles = EndPrepSDKCall()) == null)
			g_hSnapEyeAngles = null;
	}

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

stock bool IsSniper(int client)
{
	if (IsEBot(client) || m_isAFK[client])
		return m_isSniper[client];
	else
		return m_class[client] == TFClass_Sniper;
}

stock bool IsMedic(int client)
{
	if (IsEBot(client) || m_isAFK[client])
		return m_isMedic[client];
	else
		return m_class[client] == TFClass_Medic;
}

stock bool HasCrossbow(int client)
{
	if (IsEBot(client) || m_isAFK[client])
		return m_hasCrossbow[client];
	else
		return false;
}

stock bool HasSandwich(int client)
{
	if (IsEBot(client) || m_isAFK[client])
		return m_hasSandwich[client];
	else
		return false;
}

stock bool GotDamageByLast(int client, float value)
{
	return m_damageTime[client] > GetGameTime() + value;
}

stock bool ActiveCloak(int client)
{
	if (!TF2_IsPlayerInCondition(client, TFCond_Cloaked) && !TF2_IsPlayerInCondition(client, TFCond_DeadRingered))
		m_buttons[client] |= IN_ATTACK2;

	return true;
}

stock bool DeactiveCloak(int client)
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
	int iPayload = -1;
	while ((iPayload = FindEntityByClassname(iPayload, "mapobj_cart_dispenser")) != -1)
	{
		if (!IsValidEntity(iPayload))
			continue;
		
		if (GetTeamNumber(iPayload) == 2)
			m_redPayload = iPayload;
		else
			m_bluPayload = iPayload;
	}
}

stock void FindHostEntity()
{
	for (int i = 1; i < MaxClients; i++)
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
	for (int i = 1; i < MaxClients; i++)
	{
		if (IsValidClient(i) && IsFakeClient(i))
		{
			char buffer[MAX_NAME_LENGTH];
			GetClientName(i, buffer, sizeof(buffer));
			if (StrEqual(name, buffer))
				return i;
		}
	}

	return -1;
}

stock int GetTeamsCount(int team)
{
	int count = 0;
	for (int i = 1; i < MaxClients; i++)
		if (IsValidClient(i) && GetClientTeam(i) == team)
			count++;
	return count;
}

stock int GetBotCount()
{
	int count = 0;
	for (int i = 1; i < MaxClients; i++)
		if (IsValidClient(i) && IsEBot(i))
			count++;
	return count;
}

stock int GetPlayersCount()
{
	int count = 0;
	for (int i = 1; i < MaxClients; i++)
		if (IsValidClient(i) && !IsEBot(i) && GetClientTeam(i) != 1)
			count++;
	return count;
}

stock int GetPlayersCountRed()
{
	int count = 0;
	for (int i = 1; i < MaxClients; i++)
		if (IsValidClient(i) && GetClientTeam(i) == 2)
			count++;
	return count;
}

stock int GetPlayersCountBlu()
{
	int count = 0;
	for (int i = 1; i < MaxClients; i++)
		if (IsValidClient(i) && GetClientTeam(i) == 3)
			count++;
	return count;
}

stock int GetTotalPlayersCount()
{
	int count = 0;
	for (int i = 1; i < MaxClients; i++)
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

stock int GetEnemyTeam(int team)
{
	return team == 2 ? 3 : 2;
}

stock void EyeVectors(int client, float fw[3] = NULL_VECTOR, float right[3] = NULL_VECTOR, float up[3] = NULL_VECTOR)
{
	GetAngleVectors(GetEyeAngles(client), fw, right, up);
}

stock void MakeVectors(float defaultangle[3] = NULL_VECTOR, float fw[3] = NULL_VECTOR, float right[3] = NULL_VECTOR, float up[3] = NULL_VECTOR)
{
	GetAngleVectors(defaultangle, fw, right, up);
}

stock float[] GetEyeAngles(int client)
{
	float vector[3];
	GetClientEyeAngles(client, vector);
	return vector;
}

stock float[] GetCenter(int entity)
{
	float result[3], maxs[3], origin[3];
	GetEntPropVector(entity, Prop_Data, "m_vecAbsOrigin", origin);
	GetEntPropVector(entity, Prop_Data, "m_vecMaxs", maxs);
	result[0] = origin[0];
	result[1] = origin[1];
	result[2] = origin[2] += (maxs[2] * 0.5);
	return result;
}

stock void EquipWeaponSlot(int client, int slot)
{
	if (IsWeaponSlotActive(client, slot))
		return;

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

float g_flNextCommand[TFMaxPlayers];
stock bool FakeClientCommandThrottled(int client, const char[] command)
{
	if (g_flNextCommand[client] > GetGameTime())
		return false;
	FakeClientCommand(client, command);
	g_flNextCommand[client] = GetGameTime() + 0.4;
	return true;
}

stock int LookupBone(int iEntity, const char[] szName)
{
	if (g_hLookupBone != null)
		return SDKCall(g_hLookupBone, iEntity, szName);
	return -1;
}

stock void GetBonePosition(int iEntity, int iBone, float origin[3], float angles[3])
{
	if (g_hGetBonePosition != null)
		SDKCall(g_hGetBonePosition, iEntity, iBone, origin, angles);
}

stock void SnapEyeAngles(int client, float viewAngles[3])
{
	if (g_hSnapEyeAngles != null)
		SDKCall(g_hSnapEyeAngles, client, viewAngles);
	else
		TeleportEntity(client, NULL_VECTOR, viewAngles, NULL_VECTOR);
}

stock void PlayTaunt(int client, int tauntid = 463)
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

stock int MakeCEIVEnt(int client, int itemdef)
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

stock bool IsValidAddress(Address pAddress)
{
	return pAddress != Address_Null;
}

stock int FindNearestEnemy(int client, float flMaxDistance = 9999999999.0)
{
	float flBestDistance = Squaredf(flMaxDistance);
	int iBestTarget = -1;
	for (int i = 1; i < MaxClients; i++)
	{
		if (client == i)
			continue;

		if (!IsValidClient(i))
			continue;
		
		if (!IsPlayerAlive(i))
			continue;
		
		if (GetClientTeam(i) == GetClientTeam(client))
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
		
		float flDistance = GetVectorDistance(GetOrigin(client), GetOrigin(i), true);
		if (flDistance < flBestDistance)
		{
			flBestDistance = flDistance;
			iBestTarget = i;
		}
	}

	return iBestTarget;
}

stock void LookAtPosition(int client, float flGoal[3], float eye_ang[3], int trackEntity = -1)
{
	float frametime = GetGameFrameTime();
	if (frametime < 0.00001)
		return;
	
	float fps = GetConVarFloat(EBotFPS);

	float flAimSpeed = 2.0;
	flAimSpeed -= fps;

	float tempGoal[3];
	tempGoal = flGoal;

	if (IsValidEntity(trackEntity))
	{
		float flVel[3];
		GetEntPropVector(trackEntity, Prop_Data, "m_vecAbsVelocity", flVel);
		float time = GetGameTime() - m_thinkTimer[client];
		tempGoal[0] += flVel[0] * time;
		tempGoal[1] += flVel[1] * time;
		tempGoal[2] += flVel[2] * time;
		trackEntity = -1;
	}

	float eye_to_target[3];
	SubtractVectors(tempGoal, GetEyePosition(client), eye_to_target);
	
	NormalizeVector(eye_to_target, eye_to_target);
	
	float ang_to_target[3];
	GetVectorAngles(eye_to_target, ang_to_target);
	
	float eye_vec[3];
	GetAngleVectors(eye_ang, eye_vec, NULL_VECTOR, NULL_VECTOR);
	
	m_cosError[client] = GetVectorDotProduct(eye_to_target, eye_vec);
	
	float max_angvel = 1024.0 + (1024.0 * fps);
	
	if (m_cosError[client] > 0.7)
		max_angvel *= Sine((3.14 * 0.5) * (1.0 + ((-49.0 * 0.06666666666) * (m_cosError[client] - 0.7))));
	
	if (m_itAimStart[client] != -1 && (GetGameTime() - m_itAimStart[client] < 0.25))
		max_angvel *= 4.0 * (GetGameTime() - m_itAimStart[client]);
	
	float new_eye_angle[3];
	new_eye_angle[0] = ApproachAngle(ang_to_target[0], eye_ang[0], (max_angvel * frametime) * flAimSpeed);
	new_eye_angle[1] = ApproachAngle(ang_to_target[1], eye_ang[1], (max_angvel * frametime) * (flAimSpeed * 1.5));

	new_eye_angle[0] = AngleNormalize(new_eye_angle[0]);
	new_eye_angle[1] = AngleNormalize(new_eye_angle[1]);
	new_eye_angle[2] = 0.0;
	
	SnapEyeAngles(client, new_eye_angle);
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

stock float fmodf(float number, float denom)
{
	return number - RoundToFloor(number / denom) * denom;
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
	float flPos[3];
	GetClientAbsOrigin(client, flPos);

	float flVel[3];
	GetEntPropVector(client, Prop_Data, "m_vecAbsVelocity", flVel);

	float fps = GetConVarFloat(EBotFPS);
	float time = GetGameTime() - m_thinkTimer[client];
	flPos[0] += flVel[0] * time + fps;
	flPos[1] += flVel[1] * time + fps;
	
	float fAng[3];
	GetClientEyeAngles(client, fAng);
	
	float newmove[3];
	SubtractVectors(flGoal, flPos, newmove);

	newmove[1] = -newmove[1];
	
	float sin = Sine(fAng[1] * FLOAT_PI / 180.0);
	float cos = Cosine(fAng[1] * FLOAT_PI / 180.0);
	
	m_moveVel[client][0] = cos * newmove[0] - sin * newmove[1];
	m_moveVel[client][1] = sin * newmove[0] + cos * newmove[1];

	float maxSpeed = GetEntPropFloat(client, Prop_Data, "m_flMaxspeed");
	ScaleVector(m_moveVel[client], maxSpeed);

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

	float vecDir[3];
	if (AvoidBumpingEnemies(client, vecDir))
	{
		m_moveVel[client][0] += vecDir[0];
		m_moveVel[client][1] += vecDir[1];
		m_moveVel[client][2] += vecDir[2];
	}
}

stock void MoveToWaypoint(int client, const float flGoal[3], float flDest[3])
{
	float flPos[3];
	GetClientAbsOrigin(client, flPos);

	if (flPos[0] == flGoal[0] && flPos[1] == flGoal[1])
		return;
	
	float flVel[3];
	GetEntPropVector(client, Prop_Data, "m_vecAbsVelocity", flVel);
	
	flPos[0] += flVel[0] * (GetConVarFloat(EBotFPS));
	flPos[1] += flVel[1] * (GetConVarFloat(EBotFPS));

	float fAng[3];
	GetClientEyeAngles(client, fAng);
	
	float newmove[3];
	SubtractVectors(flGoal, flPos, newmove);
	
	newmove[1] = -newmove[1];
	
	float sin = Sine(fAng[1] * FLOAT_PI / 180.0);
	float cos = Cosine(fAng[1] * FLOAT_PI / 180.0);
	
	m_moveVel[client][0] = cos * newmove[0] - sin * newmove[1];
	m_moveVel[client][1] = sin * newmove[0] + cos * newmove[1];
	
	float distance = GetVectorDistance(flPos, flDest, true);
	float maxSpeed = GetEntPropFloat(client, Prop_Data, "m_flMaxspeed");
	if (distance <= Squared(2))
		m_nextStuckCheck[client] = GetGameTime() + 5.0;
	else if (distance <= Squaredf(maxSpeed))
	{
		m_nextStuckCheck[client] = GetGameTime() + 5.0;
		ScaleVector(m_moveVel[client], distance / maxSpeed);
	}
	else
		ScaleVector(m_moveVel[client], maxSpeed);
}

stock void MoveOut(int client, float flGoal[3], bool checkStuck = true)
{
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

stock void BackstabMove(int client, int target, bool moveForward = true)
{
	float flBotAng[3], flTargetAng[3];
	GetClientEyeAngles(client, flBotAng);
	GetClientEyeAngles(target, flTargetAng);
	float iAngleDiff = AngleDiff(flBotAng[1], flTargetAng[1]);
	
	if (moveForward)
	{
		if (TF2_IsPlayerInCondition(client, TFCond_Cloaked) || GetClientAimTarget(target) == client)
			m_moveVel[client][0] = -500.0;
		else
			m_moveVel[client][0] = 500.0;
	}
	
	if (iAngleDiff > 90)
		m_moveVel[client][1] = -500.0;
	else if (iAngleDiff < -90)
		m_moveVel[client][1] = 500.0;
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
	TR_TraceRayFilter(start, end, MASK_CUSTOM, RayType_EndPoint, TraceEntityFilterStuff);
	float flFraction = TR_GetFraction();

	if (flFraction >= 1.0) 
		return !(flFraction == 0.0);
	
	return false;
}

float hully[3] = {-18.0, -18.0, -18.0};
float hullx[3] = {18.0, 18.0, 18.0};

stock bool IsVisibleWaypoint(float start[3], float end[3])
{
	float fstart[3];
	float fend[3];

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

stock int GetActiveWeapon(int client)
{
	return GetEntPropEnt(client, Prop_Data, "m_hActiveWeapon");
}

stock int GetMetal(int client)
{
	return GetEntProp(client, Prop_Data, "m_iAmmo", 4, 3);
}

stock bool TF2_HasTheFlag(int client)
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
		int ent = -1;
    	while ((ent = FindEntityByClassname(ent, "item_teamflag")) != -1)
    	{
        	if (GetEntPropEnt(ent, Prop_Data, "m_hOwnerEntity") == client)
            	return true;
    	}
	}

    return false;
}

stock bool IsReloading(int client)
{
	int weapon = GetActiveWeapon(client);
	if (weapon != -1)
		return GetEntProp(weapon, Prop_Data, "m_bInReload") != 0;
	return false;
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

stock int TF2_GetWeaponID(int weapon)
{
	return GetEntProp(weapon, Prop_Send, "m_iItemDefinitionIndex");
}

stock char[] TF2_GetWeaponClassName(int weapon)
{
	char className[64] = "UNKNOWN";
	GetEntityClassname(weapon, className, sizeof(className));
	return className;
}

stock bool hasWrench(int client)
{
	return m_hasWrench[client];
}

stock int TF2_GetHealingTarget(int client)
{
	int index = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
	if (!HasEntProp(index, Prop_Send, "m_hHealingTarget"))
		return -1;
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
	return GetVectorLength(buffer, true) > GetEntPropFloat(client, Prop_Send, "m_flMaxspeed");
}

stock bool IsMetalLow(int client)
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
	if (GetClientButtons(client) & IN_ATTACK)
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

stock float Clamp(float value, float min, float max)
{
    float result = value;
    if (value < min)
        result = min;
    else if (value > max)
        result = max;
    return result;
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
	Handle TraceRay = TR_TraceHullFilterEx(flPos, flPos, flMins, flMaxs, MASK_PLAYERSOLID, StuckFilter, client);
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

public bool StuckFilter(int entity, int contentsMask, any iExclude)
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

stock int GetBluControlPointCount()
{
	int count = 0;
	for (int x = 0; x <= GetMaxEntities(); x++)
	{
		if (!IsValidEntity(x))
			continue;
		
		if (!HasEntProp(x, Prop_Send, "m_iTeamNum"))
			continue;
		
		if (GetTeamNumber(x) != 3)
			continue
		
		char ClassName[32];
		GetEdictClassname(x, ClassName, 32);
		if (StrEqual(ClassName, "team_control_point", false))
			count++;
	}

	return count;
}

stock GetNearestEnemyControlPoint(int client)
{
	int NearestControlPoint = -1;
	
	float distance = -1.0; 
	float nearestDistance = -1.0;
	
	int iControlPoint = -1;
	while ((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
	{
		if (!IsValidEntity(iControlPoint))
			continue;
		
		if (GetClientTeam(client) == GetTeamNumber(iControlPoint))
			continue;
		
		distance = GetVectorDistance(GetOrigin(client), GetOrigin(iControlPoint), true);
        
		if (distance < nearestDistance || nearestDistance == -1.0)
		{
			NearestControlPoint = iControlPoint;
			nearestDistance = distance;
		}
	}
	
	return NearestControlPoint;
}

stock int GetNearestPayload(int client, int team)
{
	int NearestPayload = -1;
	float distance = -1.0; 
	float nearestDistance = -1.0;
	int iPayload = -1;
	while ((iPayload = FindEntityByClassname(iPayload, "mapobj_cart_dispenser")) != -1)
	{
		if (!IsValidEntity(iPayload))
			continue;
		
		if (GetTeamNumber(iPayload) != team)
			continue;
		
		distance = GetFastDistance(GetOrigin(client), GetOrigin(iPayload));
		if (distance < nearestDistance || nearestDistance == -1.0)
		{
			NearestPayload = iPayload;
			nearestDistance = distance;
		}
	}
	return NearestPayload;
}

// check wall on right
stock bool CheckWallRight(int client)
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
		TE_SetupBeamPoints(clientOrigin, clientTargetOrigin, m_laserIndex, m_laserIndex, 0, 30, 0.1, 1.0, 1.0, 5, 0.0, (flFraction == 1.0 ? {255, 0, 0, 255} : {0, 0, 255, 255}), 30);
		TE_SendToClient(m_hostEntity);
	}
	
	if (flFraction >= 1.0) 
		return !(flFraction == 0.0);
	return false;
}

// check wall on left
stock bool CheckWallLeft(int client)
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
		TE_SetupBeamPoints(clientOrigin, clientTargetOrigin, m_laserIndex, m_laserIndex, 0, 30, 0.1, 1.0, 1.0, 5, 0.0, (flFraction == 1.0 ? {255, 0, 0, 255} : {0, 0, 255, 255}), 30);
		TE_SendToClient(m_hostEntity);
	}

	if (flFraction >= 1.0) 
		return !(flFraction == 0.0);
	return false;
}

// check wall on right
stock bool CheckWallRight2(int client)
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
		TE_SetupBeamPoints(clientOrigin, clientTargetOrigin, m_laserIndex, m_laserIndex, 0, 30, 0.1, 1.0, 1.0, 5, 0.0, (flFraction == 1.0 ? {255, 0, 0, 255} : {0, 0, 255, 255}), 30);
		TE_SendToClient(m_hostEntity);
	}
	
	if (flFraction >= 1.0) 
		return !(flFraction == 0.0);
	return false;
}

// check wall on left
stock bool CheckWallLeft2(int client)
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
		TE_SetupBeamPoints(clientOrigin, clientTargetOrigin, m_laserIndex, m_laserIndex, 0, 30, 0.1, 1.0, 1.0, 5, 0.0, (flFraction == 1.0 ? {255, 0, 0, 255} : {0, 0, 255, 255}), 30);
		TE_SendToClient(m_hostEntity);
	}

	if (flFraction >= 1.0) 
		return !(flFraction == 0.0);
	return false;
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

	TR_TraceRayFilter(clientOrigin, clientTargetOrigin, MASK_PLAYERSOLID, RayType_EndPoint, RocketFilter, client);
	float flFraction = TR_GetFraction();

	if (GetConVarInt(EBotDebug) == 1)
	{
		TE_SetupBeamPoints(clientOrigin, clientTargetOrigin, m_laserIndex, m_laserIndex, 0, 30, 0.1, 1.0, 1.0, 5, 0.0, (flFraction == 1.0 ? {255, 0, 0, 255} : {0, 255, 0, 255}), 30);
		TE_SendToClient(m_hostEntity);
	}

	if (flFraction >= 1.0) 
		return !(flFraction == 0.0);
	
	return false;
}

stock void ClampAngle(float fAngles[3])
{
	while (fAngles[0] > 89.0)
		fAngles[0]-=360.0;
	while (fAngles[0] < -89.0)
		fAngles[0]+=360.0;
	while (fAngles[1] > 180.0)
		fAngles[1]-=360.0;
	while (fAngles[1] <-180.0)
		fAngles[1]+=360.0;
}

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
stock float GetFastDistance(float vec1[3], float vec[3])
{
	float result = (vec1[0] - vec[0]) + (vec1[1] - vec[1]) + (vec1[2] - vec[2]);
	if (result < 0)
		return -result;

	return result;
}

stock float GetFastDistance2D(float vec1[3], float vec[3])
{
	float result = (vec1[0] - vec[0]) + (vec1[1] - vec[1]);
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

stock void DisguiseAsEnemy(int client)
{
	if (isVSH && GetClientTeam(client) == 3)
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
	
	TF2_DisguisePlayer(client, (GetClientTeam(client) == 2 ? TFTeam_Blue : TFTeam_Red), view_as<TFClassType>(class.Get(GetRandomInt(0, class.Length - 1))));
	delete class;
}

stock bool AvoidBumpingEnemies(int client, float out[3])
{
	if (m_class[client] != TFClass_Spy)
		return false;
	
	int ClosestPlayer = 0;
	float flClosestDistance = Squaredf(200.0);
	
	for (int i = 1; i < MaxClients; i++)
	{
		if (!IsValidClient(i))
			continue;
		
		if (!IsPlayerAlive(i))
			continue;
		
		if (GetClientTeam(i) != GetEnemyTeam(i))
			continue;

		if (TF2_IsPlayerInCondition(i, TFCond_Cloaked))
			continue;
		
		if (TF2_IsPlayerInCondition(i, TFCond_Disguised))
			continue;
		
		float flDistance = GetVectorDistance(GetOrigin(client), GetOrigin(i), true);
		if (flClosestDistance > flDistance)
		{
			flClosestDistance = flDistance;
			ClosestPlayer = i;
		}
	}
	
	if (ClosestPlayer)
	{
		SubtractVectors(GetOrigin(client), GetOrigin(ClosestPlayer), out);
		return true;
	}
	
	return false;
}

stock bool IsClientAimingToMe(int client, int target)
{
	if (ClientViews(target, client, 0.9))
		return true;
		
	return false;
}

stock bool IsClientVisibleInScreen(int client, int target)
{
	if (ClientViews(client, target))
		return true;
		
	return false;
}

stock bool IsClientAimingToPosition(int client, float targetposition[3])
{
	if (ClientViewsOrigin(client, targetposition, 0.9))
		return true;
		
	return false;
}

stock void GetAimOrigin(int client, float aimorigin[3]) 
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

stock int Max(int one, int two)
{
	if (one > two)
		return one;
	return two;
}

stock int Min(int one, int two)
{
	if (one < two)
		return one;
	return two;
}

stock int Abs(int value)
{
   return value < 0 ? -value : value;
}

stock bool HasSapper(int entity)
{
	if (!HasEntProp(entity, Prop_Send, "m_bHasSapper"))
		return false;

	return GetEntProp(entity, Prop_Send, "m_bHasSapper") != 0;
}

stock bool IsSappable(int entity)
{
	return HasEntProp(entity, Prop_Send, "m_bHasSapper");
}

stock bool ClientViews(int Viewer, int Target, float fThreshold = 0.70)
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

    NormalizeVector(fDistance, fTargetDir);
    if (GetVectorDotProduct(fViewDir, fTargetDir) < fThreshold)
		return false;
    
    return true;
}

stock bool ClientViewsOrigin(int Viewer, float fTargetPos[3], float fThreshold = 0.70)
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

    NormalizeVector(fDistance, fTargetDir);
    if (GetVectorDotProduct(fViewDir, fTargetDir) < fThreshold)
		return false;
    
    return true;
}

stock bool NameAlreadyTakenByPlayer(const char[] name)
{
	for (int i = 1; i < MaxClients; i++)
	{
		if (!IsValidClient(i))
			continue;
		
		char buffer[MAX_NAME_LENGTH];
		GetClientName(i, buffer, sizeof(buffer));
		if (StrEqual(name, buffer))
			return true;
	}
	
	return false;
}

stock bool AttackToSpy(int client)
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

stock bool ScreetSpy(int client)
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

stock bool IsRevealedSpyByTarget(int client, target)
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

stock int GetEnemyControlPointCount(int client)
{
	int EnemyControlPointCount = 0;
	
	int iControlPoint = -1;
	
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

stock int GetNearestMyControlPoint(int client)
{
	int NearestControlPoint = -1;
	
	float distance = -1.0; 
	float nearestDistance = -1.0;
	
	int iControlPoint = -1;
	
	while ((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
	{
		if (!IsValidEntity(iControlPoint))
			continue;
		
		if (GetEntProp(iControlPoint, Prop_Data, "m_bLocked") != 0)
			continue;
		
		if (GetClientTeam(client) != GetTeamNumber(iControlPoint))
			continue;
		
		distance = GetVectorDistance(GetOrigin(client), GetOrigin(iControlPoint), true);
        
		if (distance < nearestDistance || nearestDistance == -1.0)
		{
			NearestControlPoint = iControlPoint;
			nearestDistance = distance;
		}
	}
	
	return NearestControlPoint;
}

stock int GetMyControlPointCount(int client)
{
	int MyControlPointCount = 0;
	
	int iControlPoint = -1;
	
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
	int iControlPoint = -1;
	
	while ((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
	{
		if (!IsValidEntity(iControlPoint))
			continue;
		
		if (GetTeamNumber(iControlPoint) != 3)
			continue;
	}
	
	return iControlPoint;
}

stock int GetNearestDefendableControlPoint(int client)
{
	int NearestControlPoint = -1;
	
	float distance = -1.0; 
	float nearestDistance = -1.0;
	
	int iControlPoint = -1;
	
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
			
			if (distance < nearestDistance || nearestDistance == -1.0)
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

stock FindEntityByTargetname(const String:targetname[], const String:classname[])
{
	decl String:namebuf[32];
	int index = -1;
	namebuf[0] = '\0';
	
	while(strcmp(namebuf, targetname) != 0 && (index = FindEntityByClassname(index, classname)) != -1)
	{
		GetEntPropString(index, Prop_Data, "m_iName", namebuf, sizeof(namebuf));
	}
	
	return index;
}