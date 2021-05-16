bool HasEnemiesNear[MAXPLAYERS + 1];
bool HasFriendsNear[MAXPLAYERS + 1];
bool HasEnemyBuildingsNear[MAXPLAYERS + 1];
bool HasTeleportersNear[MAXPLAYERS + 1];
bool HasEyeBallBossNear[MAXPLAYERS + 1];

int NearestEnemy[MAXPLAYERS + 1];
int NearestFriend[MAXPLAYERS + 1];
int NearestEnemyBuilding[MAXPLAYERS + 1];
int NearestTeleporter[MAXPLAYERS + 1];
int NearestEyeBallBoss[MAXPLAYERS + 1];

stock FindEnemiens(int client)
{
	float distance = -1.0;
	int EnemiesNearCount;
	
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
		
		if(!IsPointVisible(GetEyePosition(client), GetEyePosition(i)) && !IsPointVisible(GetEyePosition(client), GetOrigin(i)))
			continue;
		
		if(!ClientViews(client, i) && !IsAttacking(i) && TF2_GetPlayerClass(client) != TFClass_Medic && !TF2_IsPlayerInCondition(client, TFCond_Zoomed) && !TF2_IsPlayerInCondition(client, TFCond_Disguised) && !IsCapturingControlPoint(client) && !IsCapturingControlPoint(i) && !TF2_IsPlayerInCondition(client, TFCond_Taunting))
			continue;
		
		if(TF2_GetPlayerClass(i) == TFClass_Spy && !IsRevealedSpyByTarget(i, client))
			continue;
		
		EnemiesNearCount = EnemiesNearCount + 1;
		float edict_distance = (GetClientHealth(i) * GetVectorDistance(GetEyePosition(client), GetEyePosition(i), true)); // prefer lower health targets
		
		if(edict_distance < distance || distance == -1.0)
		{
			distance = edict_distance;
			NearestEnemy[client] = i;
		}
	}
	
	HasEnemiesNear[client] = EnemiesNearCount > 0;
}

stock FindFriends(int client)
{
	float distance = -1.0;
	int FriendsNearCount;
	
	for(int i = 1; i <= MaxClients; i++)
	{
		if(!IsValidClient(i))
			continue;
		
		if(!IsPlayerAlive(i))
			continue;
		
		if(i == client)
			continue;
		
		if(GetClientTeam(i) != GetClientTeam(client))
			continue;
		
		if(!IsPointVisible(GetEyePosition(client), GetEyePosition(i)))
			continue;
		
		FriendsNearCount = FriendsNearCount + 1;
		float edict_distance = GetVectorDistance(GetEyePosition(client), GetEyePosition(i), true);
		
		if(edict_distance < distance || distance == -1.0)
		{
			distance = edict_distance;
			NearestFriend[client] = i;
		}
	}
	
	HasFriendsNear[client] = FriendsNearCount > 0;
}

stock FindFriendsWithLowHealth(int client)
{
	float distance = -1.0;
	int FriendsNearCount;
	
	for(int i = 1; i <= MaxClients; i++)
	{
		if(!IsValidClient(i))
			continue;
		
		if(!IsPlayerAlive(i))
			continue;
		
		if(i == client)
			continue;
		
		if(GetClientTeam(i) != GetClientTeam(client))
			continue;
		
		if(!IsPointVisible(GetEyePosition(client), GetEyePosition(i)))
			continue;
		
		FriendsNearCount = FriendsNearCount + 1;
		float edict_distance = (GetClientHealth(i) * GetVectorDistance(GetEyePosition(client), GetEyePosition(i), true)); // prefer lower health friends
		
		if(edict_distance < distance || distance == -1.0)
		{
			distance = edict_distance;
			NearestFriend[client] = i;
		}
	}
	
	HasFriendsNear[client] = FriendsNearCount > 0;
}

stock FindFriendsForHeal(int client)
{
	float distance = -1.0;
	int FriendsNearCount;
	
	for(int i = 1; i <= MaxClients; i++)
	{
		if(!IsValidClient(i))
			continue;
		
		if(!IsPlayerAlive(i))
			continue;
		
		if(i == client)
			continue;
		
		if(GetClientTeam(i) != GetClientTeam(client))
			continue;
		
		if(!IsPointVisible(GetEyePosition(client), GetEyePosition(i)))
			continue;
		
		if(TF2_IsPlayerInCondition(i, TFCond_Cloaked))
			continue;
		
		if(TF2_IsPlayerInCondition(i, TFCond_Disguised) && TF2_IsPlayerInCondition(i, TFCond_Overhealed))
			continue;
		
		if(TF2_GetPlayerClass(i) == TFClass_Engineer && IsFakeClient(i) && IsWeaponSlotActive(i, 2) && TF2_IsPlayerInCondition(i, TFCond_Overhealed))
			continue;
		
		if(TF2_GetPlayerClass(i) == TFClass_Medic && TF2_IsPlayerInCondition(i, TFCond_Overhealed))
			continue;
		
		if(TF2_IsPlayerInCondition(i, TFCond_Zoomed) && TF2_IsPlayerInCondition(i, TFCond_Overhealed))
			continue;
		
		if(TF2_GetPlayerClass(i) == TFClass_Engineer && IsFakeClient(i) && IsWeaponSlotActive(i, 5))
			continue;
		
		FriendsNearCount = FriendsNearCount + 1;
		float edict_distance = (GetClientHealth(i) * GetVectorDistance(GetEyePosition(client), GetEyePosition(i), true)); // prefer lower health targets
		
		if(edict_distance < distance || distance == -1.0)
		{
			distance = edict_distance;
			NearestFriend[client] = i;
		}
	}
	
	HasFriendsNear[client] = FriendsNearCount > 0;
}

stock GetNearestEnemyBuilding(int client)
{
	int EnemysBuildingsNearCount;
	
	float distance = -1.0; 
	float nearestDistance = -1.0;
	
	int iBuilding = -1;
	while ((iBuilding = FindEntityByClassname(iBuilding, "obj_*")) != -1)
	{
		if (!IsValidEntity(iBuilding) || !IsValidEdict(iBuilding) || (iBuilding == -1))
			continue;
			
		if(GetClientTeam(client) != GetTeamNumber(iBuilding))
		{
			if(IsPointVisibleTank(GetEyePosition(client), WorldSpaceCenter(iBuilding)) && HasEntProp(iBuilding, Prop_Send, "m_bHasSapper") && GetEntProp(iBuilding, Prop_Send, "m_bHasSapper") == 0)
			{
				EnemysBuildingsNearCount = EnemysBuildingsNearCount + 1;
				
				distance = GetVectorDistance(GetEyePosition(client), GetOrigin(iBuilding), true);
        		
				if (distance < nearestDistance || nearestDistance == -1.0)
				{
					NearestEnemyBuilding[client] = iBuilding;
					nearestDistance = distance;
				}
			}
		}
	}
	
	HasEnemyBuildingsNear[client] = EnemysBuildingsNearCount > 0;
}

stock GetNearestTeleporter(int client)
{
	int TeleportersNearCount;
	
	float distance = -1.0; 
	float nearestDistance = -1.0;
	
	int iBuilding = -1;
	while ((iBuilding = FindEntityByClassname(iBuilding, "obj_teleporter")) != -1)
	{
		if (!IsValidEntity(iBuilding) || !IsValidEdict(iBuilding) || (iBuilding == -1))
			continue;
		
		if(GetClientTeam(client) == GetTeamNumber(iBuilding) && TF2_GetObjectMode(iBuilding) == TFObjectMode_Entrance)
		{
			if(IsPointVisibleTank(GetEyePosition(client), GetOrigin(iBuilding)) && GetEntProp(iBuilding, Prop_Send, "m_bHasSapper") == 0 && (GetEntProp(iBuilding, Prop_Send, "m_iState") == 2 || GetEntProp(iBuilding, Prop_Send, "m_iState") == 6 && GetEntProp(iBuilding, Prop_Send, "m_iUpgradeLevel") == 3)) // 3 level teleporters is so fast
			{
				TeleportersNearCount = TeleportersNearCount + 1;
				
				distance = GetVectorDistance(GetEyePosition(client), GetOrigin(iBuilding), true);
        		
				if (distance < nearestDistance || nearestDistance == -1.0)
				{
					NearestTeleporter[client] = iBuilding;
					nearestDistance = distance;
				}
			}
		}
	}
	
	HasTeleportersNear[client] = TeleportersNearCount > 0;
}

stock GetNearestEyeBallBoss(int client)
{
	int EyeBallBossNearCount;
	
	float distance = -1.0; 
	float nearestDistance = -1.0;
	
	int iBoss = -1;
	while ((iBoss = FindEntityByClassname(iBoss, "tf_zombie")) != -1)
	{
		if (!IsValidEntity(iBoss) || !IsValidEdict(iBoss) || (iBoss == -1))
			continue;
		
		if(GetClientTeam(client) != GetTeamNumber(iBoss))
		{
			if(IsPointVisibleTank(GetEyePosition(client), WorldSpaceCenter(iBoss)))
			{
				EyeBallBossNearCount = EyeBallBossNearCount + 1;
				
				distance = GetVectorDistance(GetEyePosition(client), GetOrigin(iBoss), true);
        		
				if (distance < nearestDistance || nearestDistance == -1.0)
				{
					NearestEyeBallBoss[client] = iBoss;
					nearestDistance = distance;
				}
			}
		}
	}
	
	HasEyeBallBossNear[client] = EyeBallBossNearCount > 0;
}