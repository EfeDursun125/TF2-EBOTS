float AttackTimer[MAXPLAYERS + 1];
float Attack2Timer[MAXPLAYERS + 1];

float StopTime[MAXPLAYERS + 1];

float ProtectTeammateTime[MAXPLAYERS + 1];

float SniperSpotOrigin[MAXPLAYERS + 1][3];

bool g_bHealthIsLow[MAXPLAYERS+1];

bool g_bAmmoIsLow[MAXPLAYERS+1];

bool TryUnStuck[MAXPLAYERS+1];

bool IsSlowThink[MAXPLAYERS+1];

bool CanAttack[MAXPLAYERS+1];

float TargetGoal[MAXPLAYERS + 1][3];

int ForcePressButton[MAXPLAYERS + 1];

bool DefendMode[MAXPLAYERS+1];

float g_flGoal[MAXPLAYERS + 1][3];

float g_flClientEyePos[MAXPLAYERS + 1][3];
float g_flClientOrigin[MAXPLAYERS + 1][3];

bool WantsBuildSentryGun[MAXPLAYERS + 1];

bool WantsBuildDispenser[MAXPLAYERS + 1];

bool WantsBuildTeleporterEnter[MAXPLAYERS + 1];

bool WantsBuildTeleporterExit[MAXPLAYERS + 1];

bool g_bPickRandomSentrySpot[MAXPLAYERS + 1];

int TargetSpyClient[MAXPLAYERS + 1];

int PrimaryID[MAXPLAYERS + 1];

int SecondaryID[MAXPLAYERS + 1];

int MeleeID[MAXPLAYERS + 1];

Handle g_hHasAmmo;

Handle g_hSnapEyeAngles;

Handle g_hLookupBone;

Handle g_hGetBonePosition;

Handle g_hWorldSpaceCenter;

Address navarea_count;

Address TheNavAreas;

void InitGamedata()
{
	Handle hConf = LoadGameConfigFile("tf2.pets");
	
	StartPrepSDKCall(SDKCall_Entity);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Virtual, "CTFWeaponBaseGun::HasAmmo");
	PrepSDKCall_SetReturnInfo(SDKType_Bool, SDKPass_ByValue);	//Returns true/false
	if ((g_hHasAmmo = EndPrepSDKCall()) == null) SetFailState("Failed to create SDKCall for HasAmmo offset!");
	
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
	
	StartPrepSDKCall(SDKCall_Player);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Signature, "CBasePlayer::SnapEyeAngles");
	PrepSDKCall_AddParameter(SDKType_QAngle, SDKPass_ByRef);
	if ((g_hSnapEyeAngles = EndPrepSDKCall()) == null) SetFailState("Failed to create SDKCall for CBasePlayer::SnapEyeAngles!");
	
	StartPrepSDKCall(SDKCall_Entity);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Virtual, "CBaseEntity::WorldSpaceCenter");
	PrepSDKCall_SetReturnInfo(SDKType_Vector, SDKPass_ByRef);
	if ((g_hWorldSpaceCenter = EndPrepSDKCall()) == null) SetFailState("Failed to create SDKCall for CBaseEntity::WorldSpaceCenter offset!");
	
	navarea_count = GameConfGetAddress(hConf, "navarea_count");
	PrintToServer("Found \"navarea_count\" @ 0x%X", navarea_count);
	
	TheNavAreas = view_as<Address>(LoadFromAddress(navarea_count + view_as<Address>(0x4), NumberType_Int32));
	PrintToServer("Found \"TheNavAreas\" @ 0x%X", TheNavAreas);
	
	delete hConf;
}

stock bool IsValidClient(int client)
{
	if(client <= 0 || client > MaxClients)
		return false;
	
	if(!IsClientInGame(client))
		return false;
	
	if(!IsClientConnected(client))
		return false;
	
	if(IsClientSourceTV(client))
		return false;
	
	if(IsClientReplay(client))
		return false;
	
	return true;
}

stock float[] GetEyePosition(int client)
{
	if(!IsValidClient(client))
		return NULL_VECTOR;
	
	float vector[3];
	GetClientEyePosition(client, vector);
	return vector;
}

stock float[] GetOrigin(int entity)
{
	float vector[3];
	GetEntPropVector(entity, Prop_Data, "m_vecAbsOrigin", vector);
	return vector;
}

stock float[] GetAngles(int entity)
{	
	float vector[3];
	GetEntPropVector(entity, Prop_Data, "m_angRotation", vector);
	return vector;
}

stock int GetTeamsCount(int team)
{
	int count, i; count = 0;
	
	for(i = 1; i <= MaxClients; i++)
		if(IsValidClient(i) && GetClientTeam(i) == team)
			count++;
		
	return count;
}

stock int GetBotCount()
{
	int count, i; count = 0;
	
	for(i = 1; i <= MaxClients; i++)
		if(IsValidClient(i) && IsFakeClient(i))
			count++;
		
	return count;
}

stock int GetPlayersCount()
{
	int count, i; count = 0;
	
	for(i = 1; i <= MaxClients; i++)
		if(IsValidClient(i) && !IsFakeClient(i) && GetClientTeam(i) != 1)
			count++;
		
	return count;
}

stock int GetPlayersCountRed()
{
	int count, i; count = 0;
	
	for(i = 1; i <= MaxClients; i++)
		if(IsValidClient(i) && GetClientTeam(i) == 2)
			count++;
		
	return count;
}

stock int GetPlayersCountBlu()
{
	int count, i; count = 0;
	
	for(i = 1; i <= MaxClients; i++)
		if(IsValidClient(i) && GetClientTeam(i) == 3)
			count++;
		
	return count;
}

stock int GetTotalPlayersCount()
{
	int count, i; count = 0;
	
	for(i = 1; i <= MaxClients; i++)
		if(IsValidClient(i) && GetClientTeam(i) != 1)
			count++;
		
	return count;
}

stock bool IsMetalLow(int client)
{
	if(TF2_GetPlayerClass(client) == TFClass_Engineer)
	{
		if(WantsBuildSentryGun[client])
		{
			return GetMetal(client) <= 130.0;
		}
		else if(WantsBuildDispenser[client])
		{
			return GetMetal(client) <= 100.0;
		}
		else if(WantsBuildTeleporterEnter[client] || WantsBuildTeleporterExit[client])
		{
			return GetMetal(client) <= 50.0;
		}
		else if(GetMetal(client) <= 0.0)
		{
			return true;
		}
	}
	
	return false;
}

stock bool IsAmmoLow(int client)
{
	int Primary = GetPlayerWeaponSlot(client, TFWeaponSlot_Primary);
	int Secondary = GetPlayerWeaponSlot(client, TFWeaponSlot_Secondary);
	
	if(IsValidEntity(Primary) && !HasAmmo(Primary) || IsValidEntity(Secondary) && !HasAmmo(Secondary))
	{
		return true;
	}
	
	return false;
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
    int ent = -1;
    while ((ent = FindEntityByClassname(ent, "item_teamflag")) != -1)
    {
        if (GetEntPropEnt(ent, Prop_Data, "m_hOwnerEntity") == client)
            return true;
    }
    return false;
}

stock int AngleDifference(float angle1, float angle2)
{
	int diff = RoundToNearest((angle2 - angle1 + 180)) % 360 - 180;
	return diff < -180 ? diff + 360 : diff;
}

stock float AngleDiff( float destAngle, float srcAngle )
{
	return AngleNormalize(destAngle - srcAngle);
}

stock float ApproachAngle( float target, float value, float speed )
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

public int GetNearestEntity(int client, char[] classname)
{    
	float clientOrigin[3];
	float entityOrigin[3];
	float distance = -1.0;
	int nearestEntity = -1;
	int entity = -1;
	while((entity = FindEntityByClassname(entity, classname)) != INVALID_ENT_REFERENCE)
	{
		if(IsValidEntity(entity))
		{
			GetEntPropVector(entity, Prop_Data, "m_vecOrigin", entityOrigin);
			GetClientAbsOrigin(client, clientOrigin);
			
			float edict_distance = GetVectorDistance(clientOrigin, entityOrigin, true);
			if((edict_distance < distance) || (distance == -1.0))
			{
				distance = edict_distance;
				nearestEntity = entity;
			}
		}
	}
	
	return nearestEntity;
}

stock int GetPlayerStickyCount(int client)
{
	int StickyCount = 0;
	char ClassName[32];
	for(int x = 0; x <= GetMaxEntities(); x++)
	{
		if(IsValidEdict(x) && IsValidEntity(x))
		{
			GetEdictClassname(x, ClassName, 32);
			
			if(!HasEntProp(x, Prop_Send, "m_hThrower"))
				continue;
			
			if(StrContains(ClassName, "tf_projectile_pipe_remote", false) != -1 && GetEntityThrower(x) == client)
			{
				StickyCount++;
			}
		}
	}
	
	return StickyCount;
}

stock TF2_GetNumberOfHealers(client)
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

stock bool HasAmmo(int weapon)
{
	return SDKCall(g_hHasAmmo, weapon);
}

stock bool IsAttacking(int client)
{
	if(!IsValidClient(client))
		return false;
	
	if(GetClientButtons(client) == IN_ATTACK)
		return true;
	
	return false;
}

stock void SnapEyeAngles(int client, float viewAngles[3])
{
	SDKCall(g_hSnapEyeAngles, client, viewAngles);
}

stock int LookupBone(int iEntity, const char[] szName)
{
	return SDKCall(g_hLookupBone, iEntity, szName);
}

stock void GetBonePosition(int iEntity, int iBone, float origin[3], float angles[3])
{
	SDKCall(g_hGetBonePosition, iEntity, iBone, origin, angles);
}

stock bool ChanceOf(int number)
{
	return GetRandomInt(1, 100) <= number;
}

stock float[] WorldSpaceCenter(int entity)
{
	if(!IsValidEntity(entity))
		return NULL_VECTOR;
	
	if(!IsValidEdict(entity))
		return NULL_VECTOR;
	
	float vecPos[3];
	SDKCall(g_hWorldSpaceCenter, entity, vecPos);
	
	return vecPos;
}

stock float[] GetAbsVelocity(int client)
{
	float v[3];
	GetEntPropVector(client, Prop_Data, "m_vecAbsVelocity", v);
	
	return v;
}

stock bool IsResupply(int entity)
{
	char model[PLATFORM_MAX_PATH];
	GetEntPropString(entity, Prop_Data, "m_ModelName", model, sizeof(model));
	
	return StrEqual(model, "models/props_gameplay/resupply_locker.mdl");
}

stock void TF2_MoveTo(int client, float flGoal[3], float fVel[3], float fAng[3])
{
	float flPos[3];
	GetClientAbsOrigin(client, flPos);
	
	float newmove[3];
	SubtractVectors(flGoal, flPos, newmove);
	
	newmove[1] = -newmove[1];
	
	float sin = Sine(fAng[1] * FLOAT_PI / 180.0);
	float cos = Cosine(fAng[1] * FLOAT_PI / 180.0);
	
	fVel[0] = cos * newmove[0] - sin * newmove[1];
	fVel[1] = sin * newmove[0] + cos * newmove[1];
	
	NormalizeVector(fVel, fVel);
	
	if(GetVectorDistance(flPos, flGoal, true) > 20.0)
	{
		ScaleVector(fVel, 450.0);
	}
}

stock bool TF2_IsNextToWall(int client)
{
	float flPos[3];
	GetClientAbsOrigin(client, flPos);
	
	float flMaxs[3], flMins[3];
	GetEntPropVector(client, Prop_Send, "m_vecMaxs", flMaxs);
	GetEntPropVector(client, Prop_Send, "m_vecMins", flMins);
	
	flMaxs[0] += 1.5;
	flMaxs[1] += 1.5;
	flMins[0] -= 1.5;
	flMins[1] -= 1.5;
	
	flPos[2] += 18.0;
	
	//Perform a wall check to see if we are near any obstacles we should try jump over
	Handle TraceRay = TR_TraceHullFilterEx(flPos, flPos, flMins, flMaxs, MASK_PLAYERSOLID, ExcludeFilter, client);
	
	bool bHit = TR_DidHit(TraceRay);	
	
	delete TraceRay;
	
	return bHit;
}

public bool ExcludeFilter(int entity, int contentsMask, any iExclude)
{
    return !(entity == iExclude);
}

stock bool AttackToSpy(int client)
{
	if(!IsValidClient(client))
		return false;
	
	if(TF2_IsPlayerInCondition(client, TFCond_CritOnWin) || TF2_IsPlayerInCondition(client, TFCond_CritOnFlagCapture))
	{
		return true;
	}
	
	return false;
}

stock bool ScreetSpy(int client)
{
	if(!IsValidClient(client))
		return false;
	
	if(TF2_IsPlayerInCondition(client, TFCond_DeadRingered))
	{
		return true;
	}
	
	if(TF2_IsPlayerInCondition(client, TFCond_Disguising) || 
	TF2_IsPlayerInCondition(client, TFCond_CloakFlicker) || 
	TF2_IsPlayerInCondition(client, TFCond_Bonked) || 
	TF2_IsPlayerInCondition(client, TFCond_OnFire) || 
	TF2_IsPlayerInCondition(client, TFCond_Jarated) || 
	TF2_IsPlayerInCondition(client, TFCond_Bleeding) || 
	TF2_IsPlayerInCondition(client, TFCond_Milked) || 
	TF2_IsPlayerInCondition(client, TFCond_Sapped) || 
	TF2_IsPlayerInCondition(client, TFCond_Gas))
	{
		return false;
	}
	
	if(TF2_IsPlayerInCondition(client, TFCond_Disguised) || TF2_IsPlayerInCondition(client, TFCond_Cloaked) || TF2_IsPlayerInCondition(client, TFCond_Stealthed))
	{
		return true;
	}
	
	return false;
}

stock int TF2_GetObject(int client, TFObjectType type, TFObjectMode mode)
{
	int iObject = INVALID_ENT_REFERENCE;
	while ((iObject = FindEntityByClassname(iObject, "obj_*")) != -1)
	{
		TFObjectType iObjType = TF2_GetObjectType(iObject);
		TFObjectMode iObjMode = TF2_GetObjectMode(iObject);
		
		if(GetEntPropEnt(iObject, Prop_Send, "m_hBuilder") == client && iObjType == type && iObjMode == mode 
		&& !GetEntProp(iObject, Prop_Send, "m_bPlacing")
		&& !GetEntProp(iObject, Prop_Send, "m_bDisposableBuilding"))
		{			
			return iObject;
		}
	}
	
	return iObject;
}

stock Client_GetClosest(int client)
{
	float distance = -1.0;
	int closestEdict = -1;
	for(int i=1;i<=MaxClients;i++)
	{
		if (!IsClientInGame(i) || !IsPlayerAlive(i) || (i == client))
			continue;
		if(GetClientTeam(i) != GetClientTeam(client))
		{
			float edict_distance = GetVectorDistance(GetEyePosition(client), GetEyePosition(i), true);
			if((edict_distance < distance) || (distance == -1.0))
			{
				distance = edict_distance;
				closestEdict = i;
			}
		}
	}
	
	return closestEdict;
}

stock GetNearestPossibleEnemy(int client, float range = 999999.0)
{
	float distance = -1.0;
	int NearestTarget = -1;
	
	for(int i = 1; i <= MaxClients; i++)
	{
		if(!IsValidClient(i))
			continue;
		
		if(!IsPlayerAlive(i))
			continue;
		
		if(i == client)
			continue;
		
		if(GetClientTeam(i) == GetClientTeam(client))
			continue;
		
		if(PF_Exists(client) && PF_IsPathToEntityPossible(client, i, range))
			continue;
		
		if(TF2_GetPlayerClass(i) == TFClass_Spy && !IsRevealedSpyByTarget(i, client))
			continue;
		
		float edict_distance = GetVectorDistance(GetEyePosition(client), GetEyePosition(i), true);
		
		if(edict_distance < distance || distance == -1.0)
		{
			distance = edict_distance;
			NearestTarget = i;
		}
	}
	
	return NearestTarget;
}

stock int TF2_GetHealingTarget(int client)
{
	int index = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
	return GetEntPropEnt(index, Prop_Send, "m_hHealingTarget");
}

stock int GetClientMaxHealth(int client)
{
	return GetEntProp(client, Prop_Data, "m_iMaxHealth");
}

stock bool ClientIsMoving(int client)
{
	if(!IsValidClient(client))
		return false;
	
	float buffer[3];
	GetEntPropVector(client, Prop_Data, "m_vecAbsVelocity", buffer);
	return (GetVectorLength(buffer) > 10.0);
}

stock bool TF2_IsBountyMode()
{
	return view_as<bool>(GameRules_GetProp("m_bBountyModeEnabled"));
}

stock bool IsClientAimingToMe(int client, int target)
{
	if(!IsValidClient(client))
		return false;
	
	if(!IsValidClient(target))
		return false;
	
	if(ClientViews(target, client, 0.9))
		return true;
		
	return false;
}

stock bool IsClientVisibleInScreen(int client, int target)
{
	if(!IsValidClient(client))
		return false;
	
	if(!IsValidClient(target))
		return false;
	
	if(ClientViews(client, target, 0.6))
		return true;
		
	return false;
}

stock bool IsClientAimingToPosition(int client, float targetposition[3])
{
	if(!IsValidClient(client))
		return false;
	
	if(ClientViewsOrigin(client, targetposition, 0.95))
		return true;
		
	return false;
}

stock TFTeam GetEnemyTFTeam(int client)
{
	if(GetClientTeam(client) == 3)
	{
		return TFTeam_Red;
	}
	else if(GetClientTeam(client) == 2)
	{
		return TFTeam_Blue;
	}
	
	return TFTeam_Unassigned;
}

stock int GetEnemyTeam(int client)
{
	if(GetClientTeam(client) == 3)
	{
		return 2;
	}
	else if(GetClientTeam(client) == 2)
	{
		return 3;
	}
	
	return 1;
}

stock bool IsStealthed(int client)
{
	if(!IsValidClient(client))
		return false;
	
	if(TF2_IsPlayerInCondition(client, TFCond_Cloaked))
		return true;
	
	if(TF2_IsPlayerInCondition(client, TFCond_DeadRingered))
		return true;
	
	if(TF2_IsPlayerInCondition(client, TFCond_Stealthed))
		return true;
		
	return TF2_IsPlayerInCondition(client, TFCond_StealthedUserBuffFade);
}

stock float GetPercentInvisible(int client)
{
	int offset = FindSendPropInfo("CTFPlayer", "m_flInvisChangeCompleteTime") - 8;
	return GetEntDataFloat(client, offset);
}

stock bool IsRevealedSpy(int client)
{
	if(!IsValidClient(client))
		return false;
	
	if(IsStealthed(client) && TF2_IsPlayerInCondition(client, TFCond_CloakFlicker))
		return true;
	
	if(TF2_IsPlayerInCondition(client, TFCond_Bleeding))
		return true;
	
	if(TF2_IsPlayerInCondition(client, TFCond_Bonked))
		return true;
	
	if(TF2_IsPlayerInCondition(client, TFCond_Dazed))
		return true;
	
	if(TF2_IsPlayerInCondition(client, TFCond_Gas))
		return true;
	
	if(TF2_IsPlayerInCondition(client, TFCond_Jarated))
		return true;
	
	if(TF2_IsPlayerInCondition(client, TFCond_Milked))
		return true;
	
	return false;
}

stock bool IsRevealedSpyByTarget(int client, target)
{
	if(!IsValidClient(client))
		return false;
	
	if(!IsValidClient(target))
		return false;
	
	if(GetVectorDistance(GetOrigin(client), GetOrigin(target)) <= 80.0)
		return true;
	
	if(IsStealthed(client))
		return (GetPercentInvisible(client) <= 0.75);
	
	if(!TF2_IsPlayerInCondition(client, TFCond_Disguised))
		return true;
	
	if(IsStealthed(client) && TF2_IsPlayerInCondition(client, TFCond_CloakFlicker))
		return true;
	
	if(!IsStealthed(client) && TargetSpyClient[target] == client)
		return true;
	
	if(!TF2_IsPlayerInCondition(client, TFCond_CritOnFirstBlood) && TF2_IsPlayerInCondition(target, TFCond_CritOnFirstBlood)) // what? why this guy don't have a crit? wait... it's a spy!
		return true;
	
	if(!TF2_IsPlayerInCondition(client, TFCond_CritOnWin) && TF2_IsPlayerInCondition(target, TFCond_CritOnWin)) // what? why this guy don't have a crit? wait... it's a spy!
		return true;
	
	if(!TF2_IsPlayerInCondition(client, TFCond_CritOnFlagCapture) && TF2_IsPlayerInCondition(target, TFCond_CritOnFlagCapture)) // what? why this guy don't have a crit? wait... it's a spy!
		return true;
	
	if(TF2_IsPlayerInCondition(client, TFCond_Bleeding))
		return true;
	
	if(TF2_IsPlayerInCondition(client, TFCond_Bonked))
		return true;
	
	if(TF2_IsPlayerInCondition(client, TFCond_Dazed))
		return true;
	
	if(TF2_IsPlayerInCondition(client, TFCond_Gas))
		return true;
	
	if(TF2_IsPlayerInCondition(client, TFCond_Jarated))
		return true;
	
	if(TF2_IsPlayerInCondition(client, TFCond_Milked))
		return true;
	
	return false;
}

stock int GetUsableSniperHintsCount(int client)
{
	int SniperHintsCount = 0;
	int iEntity = -1;
	
	while((iEntity = FindEntityByClassname(iEntity, "func_tfbot_hint")) != -1)
	{
		if(!IsValidEntity(iEntity))
			continue;
		
		if(!IsValidEdict(iEntity))
			continue;
			
		if(GetEntProp(iEntity, Prop_Data, "m_hint") != 0)
			continue;
		
		if(GetEntProp(iEntity, Prop_Data, "m_isDisabled") == 1)
			continue;
			
		if(GetEnemyTeam(client) == GetTeamNumber(iEntity)) // don't use enemy hints. but still can use UNDEFINED TEAM hints. (GetClientTeam(client) != GetTeamNumber(iEntity) is team only hints.)
			continue;
		
		SniperHintsCount++;
	}
	
	return SniperHintsCount;
}

stock int GetUsableSentryHintsCount(int client)
{
	int SentryHintsCount = 0;
	int iEntity = -1;
	
	while((iEntity = FindEntityByClassname(iEntity, "bot_hint_sentrygun")) != -1)
	{
		if(!IsValidEntity(iEntity))
			continue;
		
		if(!IsValidEdict(iEntity))
			continue;
			
		//if(GetEntProp(iEntity, Prop_Data, "m_hOwnerEntity") != -1)
		//	continue;
		
		if(GetEntProp(iEntity, Prop_Data, "m_isDisabled") == 1)
			continue;
			
		if(GetEnemyTeam(client) == GetTeamNumber(iEntity)) // don't use enemy hints. but still can use UNDEFINED TEAM hints. (GetClientTeam(client) != GetTeamNumber(iEntity) is team only hints.)
			continue;
		
		SentryHintsCount++;
	}
	
	return SentryHintsCount;
}

stock int GetUsableTeleporterExitHintsCount(int client)
{
	int TeleporterExitHintsCount = 0;
	int iEntity = -1;
	
	while((iEntity = FindEntityByClassname(iEntity, "bot_hint_teleporter_exit")) != -1)
	{
		if(!IsValidEntity(iEntity))
			continue;
		
		if(!IsValidEdict(iEntity))
			continue;
		
		if(GetEntProp(iEntity, Prop_Data, "m_isDisabled") == 1)
			continue;
			
		if(GetEnemyTeam(client) == GetTeamNumber(iEntity)) // don't use enemy hints. but still can use UNDEFINED TEAM hints. (GetClientTeam(client) != GetTeamNumber(iEntity) is team only hints.)
			continue;
		
		TeleporterExitHintsCount++;
	}
	
	return TeleporterExitHintsCount;
}

stock GetNearestPayload(int client, int team)
{
	int NearestPayload = -1;
	
	float distance = -1.0; 
	float nearestDistance = -1.0;
	
	int iPayload = -1;
	
	while ((iPayload = FindEntityByClassname(iPayload, "mapobj_cart_dispenser")) != -1)
	{
		if(!IsValidEntity(iPayload))
			continue;
		
		if(!IsValidEdict(iPayload))
			continue;
		
		if(GetTeamNumber(iPayload) != team)
			continue;
		
		distance = GetVectorDistance(GetOrigin(client), GetOrigin(iPayload), true);
        
		if (distance < nearestDistance || nearestDistance == -1.0)
		{
			NearestPayload = iPayload;
			nearestDistance = distance;
		}
	}
	
	return NearestPayload;
}

stock GetNearestEnemyControlPoint(int client)
{
	int NearestControlPoint = -1;
	
	float distance = -1.0; 
	float nearestDistance = -1.0;
	
	int iControlPoint = -1;
	
	while ((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
	{
		if(!IsValidEntity(iControlPoint))
			continue;
		
		if(!IsValidEdict(iControlPoint))
			continue;
		
		if(GetEntProp(iControlPoint, Prop_Data, "m_bLocked") != 0)
			continue;
		
		if(GetClientTeam(client) == GetTeamNumber(iControlPoint))
			continue;
		
		distance = GetVectorDistance(GetOrigin(client), GetOrigin(iControlPoint));
        
		if (distance < nearestDistance || nearestDistance == -1.0)
		{
			NearestControlPoint = iControlPoint;
			nearestDistance = distance;
		}
	}
	
	return NearestControlPoint;
}

stock int GetEnemyControlPointCount(int client)
{
	int EnemyControlPointCount = 0;
	
	int iControlPoint = -1;
	
	while((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
	{
		if(!IsValidEntity(iControlPoint))
			continue;
		
		if(!IsValidEdict(iControlPoint))
			continue;
		
		if(GetEntProp(iControlPoint, Prop_Data, "m_bLocked") != 0)
			continue;
		
		if(GetClientTeam(client) == GetTeamNumber(iControlPoint))
			continue;
		
		EnemyControlPointCount++;
	}
	
	return EnemyControlPointCount;
}

stock GetNearestMyControlPoint(int client)
{
	int NearestControlPoint = -1;
	
	float distance = -1.0; 
	float nearestDistance = -1.0;
	
	int iControlPoint = -1;
	
	while((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
	{
		if(!IsValidEntity(iControlPoint))
			continue;
		
		if(!IsValidEdict(iControlPoint))
			continue;
		
		if(GetEntProp(iControlPoint, Prop_Data, "m_bLocked") != 0)
			continue;
		
		if(GetClientTeam(client) != GetTeamNumber(iControlPoint))
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
	
	while((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
	{
		if(!IsValidEntity(iControlPoint))
			continue;
		
		if(!IsValidEdict(iControlPoint))
			continue;
		
		if(GetEntProp(iControlPoint, Prop_Data, "m_bLocked") != 0)
			continue;
		
		if(GetClientTeam(client) != GetTeamNumber(iControlPoint))
			continue;
		
		MyControlPointCount++;
	}
	
	return MyControlPointCount;
}

stock GetNearestBluControlPoint()
{
	int iControlPoint = -1;
	
	while((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
	{
		if(!IsValidEntity(iControlPoint))
			continue;
		
		if(!IsValidEdict(iControlPoint))
			continue;
		
		if(GetTeamNumber(iControlPoint) != 3)
			continue;
	}
	
	return iControlPoint;
}

stock GetNearestDefendableControlPoint(int client)
{
	int NearestControlPoint = -1;
	
	float distance = -1.0; 
	float nearestDistance = -1.0;
	
	int iControlPoint = -1;
	
	if(GetNearestBluControlPoint() != -1)
	{
		while((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
		{
			if(!IsValidEntity(iControlPoint))
				continue;
			
			if(!IsValidEdict(iControlPoint))
				continue;
			
			if(GetEntProp(iControlPoint, Prop_Data, "m_bLocked") != 0)
				continue;
			
			if(GetClientTeam(client) != GetTeamNumber(iControlPoint))
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
		while((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
		{
			if(!IsValidEntity(iControlPoint))
				continue;
			
			if(!IsValidEdict(iControlPoint))
				continue;
			
			if(GetEntProp(iControlPoint, Prop_Data, "m_bLocked") != 0)
				continue;
			
			if(GetClientTeam(client) != GetTeamNumber(iControlPoint))
				continue;
			
			NearestControlPoint = iControlPoint;
		}
	}
	
	return NearestControlPoint;
}

stock bool IsCapturingControlPoint(int client)
{
	char currentMap[PLATFORM_MAX_PATH];
	GetCurrentMap(currentMap, sizeof(currentMap));
	if(StrContains(currentMap, "cp_" , false) != -1)
	{
		int capturepoint = GetNearestEnemyControlPoint(client);
		if(capturepoint != -1 && GetVectorDistance(GetOrigin(client), GetOrigin(capturepoint), true) < 300.0)
		{
			return true;
		}
	}
	
	return false;
}

stock bool IsPushingPayload(int client)
{
	char currentMap[PLATFORM_MAX_PATH];
	GetCurrentMap(currentMap, sizeof(currentMap));
	if(StrContains(currentMap, "pl_" , false) != -1)
	{
		int capturepoint = GetNearestPayload(client);
		if(capturepoint != -1 && GetVectorDistance(GetOrigin(client), GetOrigin(capturepoint), true) < 300.0)
		{
			return true;
		}
	}
	
	return false;
}

stock bool IsInRange(float start[3], float end[3], float range)
{
	return GetVectorDistance(start, end, true) <= range;
}

stock bool IsReloading(int client)
{
	return GetEntProp(GetEntPropEnt(client, Prop_Data, "m_hActiveWeapon"), Prop_Data, "m_bInReload") != 0;
}

stock bool IsLocked(int entity)
{
	return GetEntProp(entity, Prop_Data, "m_bLocked", 1) != 0;
}
stock bool IsDisabled(int entity)
{
	return GetEntProp(entity, Prop_Data, "m_bDisabled", 1) != 0;
}
