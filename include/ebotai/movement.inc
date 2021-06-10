float HidingPosition[MAXPLAYERS + 1][3];
bool HidingSpotIsReady[MAXPLAYERS + 1];

stock void Movement(int client)
{
	if(TF2_IsPlayerInCondition(client, TFCond_CritOnWin)) // chase losers
	{
		if(IsSlowThink[client])
		{
			int Enemy = GetNearestPossibleEnemy(client);
			
			if(Enemy != -1)
			{
				TF2_FindPath(client, GetOrigin(Enemy));
				
				return;
			}
		}
	}
	
	if(TryUnStuck[client])
	{
		return;
	}
	
	if(StopTime[client] > GetGameTime() && !HasEnemiesNear[client] && !HasEnemyBuildingsNear[client])
	{
		return;
	}
	
	if(TF2_HasTheFlag(client))
	{
		ObjectiveMovement(client);
		
		return;
	}
	
	if((HasEnemiesNear[client] && IsValidClient(NearestEnemy[client]) || HasEnemyBuildingsNear[client] && IsValidEntity(NearestEnemyBuilding[client])) && !TF2_IsPlayerInCondition(client, TFCond_CritOnWin))
	{
		if(TF2_IsPlayerInCondition(client, TFCond_Cloaked))
		{
			if(HasEnemiesNear[client])
			{
				HideFromTheTarget(client, GetEyePosition(NearestEnemy[client]));
				
				TF2_FindPath(client, HidingPosition[client]);
				
				StopTime[client] = GetGameTime() + GetRandomFloat(1.0, 3.0);
				
				return;
			}
			else if(HasEnemyBuildingsNear[client])
			{
				HideFromTheTarget(client, WorldSpaceCenter(NearestEnemyBuilding[client]));
				
				TF2_FindPath(client, HidingPosition[client]);
				
				StopTime[client] = GetGameTime() + GetRandomFloat(1.0, 3.0);
				
				return;
			}
		}
		else if(TF2_IsPlayerInCondition(client, TFCond_Disguised) && !TF2_IsPlayerInCondition(client, TFCond_Cloaked))
		{
			if(GetClientHealth(client) < (GetClientHealth(NearestEnemy[client]) / 2))
			{
				HideFromTheTarget(client, GetEyePosition(NearestEnemy[client]));
				
				TF2_FindPath(client, HidingPosition[client]);
				
				StopTime[client] = GetGameTime() + GetRandomFloat(1.0, 3.0);
				
				return;
			}
		}
		else if(HasEnemiesNear[client] && 
		(TF2_IsPlayerInCondition(client, TFCond_Cloaked) 
		|| TF2_IsPlayerInCondition(client, TFCond_Disguising) 
		|| TF2_IsPlayerInCondition(client, TFCond_TeleportedGlow) 
		|| TF2_IsPlayerInCondition(NearestEnemy[client], TFCond_CritOnFlagCapture) 
		|| TF2_IsPlayerInCondition(NearestEnemy[client], TFCond_CritOnWin) 
		|| g_bAmmoIsLow[client] && TF2_GetPlayerClass(client) != TFClass_Engineer
		|| g_bHealthIsLow[client] 
		|| (GetClientHealth(client) < (GetClientHealth(NearestEnemy[client]) / 2) && IsAttacking(NearestEnemy[client]) && IsClientVisibleInScreen(NearestEnemy[client], client) && GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client])) < 768.0)
		|| !TF2_IsPlayerInCondition(client, TFCond_Disguised))) // hiding is important
		{
			if(!IsValidClient(NearestEnemy[client]))
				return;
			
			HideFromTheTarget(client, GetEyePosition(NearestEnemy[client]));
			
			TF2_FindPath(client, HidingPosition[client]);
			
			StopTime[client] = GetGameTime() + GetRandomFloat(2.0, 4.0);
			
			return;
		}
		else if(HasEnemyBuildingsNear[client] && IsValidEdict(NearestEnemyBuilding[client]) && IsValidEntity(NearestEnemyBuilding[client]) && GetClientHealth(client) < GetClientMaxHealth(client))
		{
			HideFromTheTarget(client, WorldSpaceCenter(NearestEnemyBuilding[client]));
			
			TF2_FindPath(client, HidingPosition[client]);
			
			StopTime[client] = GetGameTime() + GetRandomFloat(1.0, 3.0);
			
			return;
		}
		else
		{
			HidingSpotIsReady[client] = false;
		}
	}
	
	if(HasEnemiesNear[client] && IsValidClient(NearestEnemy[client]) && IsCapturingControlPoint(NearestEnemy[client]) && !IsCapturingControlPoint(client)) // block capturing.
	{
		TF2_FindPath(client, GetOrigin(NearestEnemy[client]));
		
		return;
	}
	
	if(HasFriendsNear[client] && IsValidClient(NearestFriend[client]) && IsCapturingControlPoint(NearestFriend[client]) && !IsCapturingControlPoint(client)) // help capturing.
	{
		TF2_FindPath(client, GetOrigin(NearestFriend[client]));
		
		return;
	}
	
	if(TF2_GetPlayerClass(client) == TFClass_Pyro && !HasEnemiesNear[client] && HasFriendsNear[client] && IsValidClient(NearestFriend[client]) && TF2_IsPlayerInCondition(NearestFriend[client], TFCond_OnFire) && TF2_GetPlayerClass(NearestFriend[client]) != TFClass_Pyro)
	{
		TF2_FindPath(client, GetOrigin(NearestFriend[client]));
		
		return;
	}
	
	if(!HasEnemiesNear[client] && HasFriendsNear[client] && IsValidClient(NearestFriend[client]) && !IsFakeClient(NearestFriend[client]) && !TF2_IsPlayerInCondition(client, TFCond_Cloaked) && !TF2_IsPlayerInCondition(client, TFCond_Disguised) && !TF2_IsPlayerInCondition(client, TFCond_Taunting))
	{
		if(TF2_IsPlayerInCondition(NearestFriend[client], TFCond_Taunting))
		{
			TF2_FindPath(client, GetOrigin(NearestFriend[client]));
			
			if(GetVectorDistance(GetOrigin(client), GetOrigin(NearestFriend[client])) < 150.0 && IsSlowThink[client])
			{
				FakeClientCommandThrottled(client, "taunt");
			}
			
			return;
		}
	}
	
	if(!TF2_IsPlayerInCondition(client, TFCond_CritOnWin) && HasFriendsNear[client] && IsValidClient(NearestFriend[client]) && !CanAttack[client] && TF2_GetPlayerClass(client) == TFClass_Medic)
	{
		if((!HasEnemiesNear[client] && GetVectorDistance(GetOrigin(client), GetOrigin(NearestFriend[client])) > 200.0) || (HasEnemiesNear[client] && GetVectorDistance(GetOrigin(client), GetOrigin(NearestFriend[client])) > 400.0))
		{
			if(GetConVarInt(EBotPerformance) != 1)
			{
				NavArea movearea = TheNavMesh.GetNearestNavArea_Vec(GetOrigin(NearestFriend[client]), true, 25000.0, true, true, GetClientTeam(client));
				if(movearea != NavArea_Null)
				{
					float RandomPosition[3];
					
					movearea.GetRandomPoint(RandomPosition);
					
					TF2_FindPath(client, RandomPosition);
				}
				else
				{
					TF2_FindPath(client, GetOrigin(NearestFriend[client]));
				}
			}
			else
			{
				TF2_FindPath(client, GetOrigin(NearestFriend[client]));
			}
			
			HidingSpotIsReady[client] = false;
		}
		else if(HasEnemiesNear[client])
		{
			HideFromTheTarget(client, GetEyePosition(NearestEnemy[client]));
			
			TF2_FindPath(client, HidingPosition[client]);
			
			StopTime[client] = GetGameTime() + GetRandomFloat(1.0, 3.0);
		}
		else
		{
			GetClientAbsOrigin(client, TargetGoal[client]);
		}
		
		return;
	}
	
	if(HasEnemiesNear[client] && IsWeaponSlotActive(client, 2) && TF2_GetPlayerClass(client) != TFClass_Spy && TF2_GetPlayerClass(client) != TFClass_Engineer)
	{
		MeleeCombat(client);
		
		return;
	}
	
	if(g_bHealthIsLow[client])
	{
		FindNearestHealthPack(client);
		GoToHealthPack(client);
		
		return;
	}
	
	if(g_bAmmoIsLow[client] && !GetEntProp(client, Prop_Send, "m_bCarryingObject"))
	{
		FindNearestAmmoPack(client);
		GoToAmmoPack(client);
		
		return;
	}
	
	if(HasEnemiesNear[client] && IsValidClient(NearestEnemy[client]) && (TF2_GetPlayerClass(client) == TFClass_Pyro || TF2_GetPlayerClass(client) == TFClass_Scout))
	{
		if(GetConVarInt(EBotPerformance) != 1)
		{
			float g_flSelectedMoveArea[3];
			
			NavArea movearea = TheNavMesh.GetNearestNavArea_Vec(GetOrigin(NearestEnemy[client]), true, 25000.0, true, true, GetClientTeam(client));
			if(movearea != NavArea_Null)
			{
				movearea.GetRandomPoint(g_flSelectedMoveArea);
				
				TF2_FindPath(client, g_flSelectedMoveArea);
			}
			else
			{
				TF2_FindPath(client, GetOrigin(NearestEnemy[client]));
			}
		}
		else
		{
			float g_flSelectedMoveArea[3];
			
			g_flSelectedMoveArea = GetOrigin(NearestEnemy[client]);
			
			g_flSelectedMoveArea[0] += GetRandomFloat(-300.0, 300.0);
			g_flSelectedMoveArea[1] += GetRandomFloat(-300.0, 300.0);
			
			TF2_FindPath(client, g_flSelectedMoveArea);
		}
		
		return;
	}
	else if(HasEnemiesNear[client] && IsValidClient(NearestEnemy[client]) && !TF2_IsPlayerInCondition(client, TFCond_Disguised))
	{
		if(TF2_HasTheFlag(NearestEnemy[client]))
		{
			TF2_FindPath(client, GetOrigin(NearestEnemy[client]));
		}
		else if(TF2_GetPlayerClass(client) != TFClass_Engineer)
		{
			if(GetConVarInt(EBotPerformance) != 1)
			{
				float RandomPosition[3];
				float g_flSelectedMoveArea[3];
				
				RandomPosition = GetOrigin(client);
				
				RandomPosition[0] += GetRandomFloat(-300.0, 300.0);
				RandomPosition[1] += GetRandomFloat(-300.0, 300.0);
				
				NavArea movearea = TheNavMesh.GetNearestNavArea_Vec(RandomPosition, true, 25000.0, true, true, GetClientTeam(client));
				if(movearea != NavArea_Null)
				{
					movearea.GetRandomPoint(g_flSelectedMoveArea);
					
					TF2_FindPath(client, g_flSelectedMoveArea);
				}
				else
				{
					TF2_FindPath(client, RandomPosition);
				}
			}
			else
			{
				float RandomPosition[3];
				
				RandomPosition = GetOrigin(client);
				
				RandomPosition[0] += GetRandomFloat(-500.0, 500.0);
				RandomPosition[1] += GetRandomFloat(-500.0, 500.0);
				
				TF2_FindPath(client, RandomPosition);
			}
		}
		
		if(IsSlowThink[client] && TF2_GetPlayerClass(NearestEnemy[client]) == TFClass_Spy)
		{
			StopTime[client] = GetGameTime() + GetRandomFloat(4.0, 8.0);
		}
		
		return;
	}
	
	if(UseTeleporter[client] && HasTeleportersNear[client] && IsValidEntity(NearestTeleporter[client]))
	{
		TF2_FindPath(client, GetOrigin(NearestTeleporter[client]));
		
		return;
	}
	
	if(TF2_GetPlayerClass(client) == TFClass_Spy && !TF2_IsPlayerInCondition(client, TFCond_Cloaked))
	{
		if(HasEnemiesNear[client] && IsValidClient(NearestEnemy[client]) && GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client])) < 300.0 && IsPointVisible(GetOrigin(client), GetOrigin(NearestEnemy[client])))
		{
			MoveTargetsBehind[client] = true;
		}
		else
		{
			SpyMoveLogic(client);
			MoveTargetsBehind[client] = false;
		}
		
		return;
	}
	
	if (ProtectTeammateTime[client] > GetGameTime())
	{
		if(HasFriendsNear[client] && IsValidClient(NearestFriend[client]) && GetVectorDistance(GetEyePosition(client), GetEyePosition(NearestFriend[client])) > 300.0)
		{
			TF2_FindPath(client, GetOrigin(NearestFriend[client]));
		}
		
		if(HasFriendsNear[client])
		{
			return;
		}
	}
	
	if(TF2_GetPlayerClass(client) == TFClass_Sniper)
	{
		SniperMoveLogic(client);
		
		return;
	}
	
	if(TF2_GetPlayerClass(client) == TFClass_Engineer)
	{
		EngineerMoveLogic(client);
		
		return;
	}
	
	if(TF2_GetPlayerClass(client) == TFClass_DemoMan && DefendMode[client])
	{
		DemomanMove(client);
		
		return;
	}
	
	ObjectiveMovement(client);
}

stock int MeleeCombat(int client)
{
	if(HasEnemiesNear[client] && IsWeaponSlotActive(client, 2))
	{
		if(GetConVarInt(EBotPerformance) != 1 && GetClientHealth(client) < (GetEntProp(client, Prop_Data, "m_iMaxHealth") / 1.6))
		{
			NavArea randomarea = TheNavMesh.GetNearestNavArea_Vec(GetOrigin(NearestEnemy[client]), true, 250.0, false, false, GetClientTeam(client));
			if(randomarea != NavArea_Null)
			{
				float RandomPosition[3];
				
				randomarea.GetRandomPoint(RandomPosition);
				
				TF2_FindPath(client, RandomPosition);
			}
		}
		else
		{
			TF2_FindPath(client, GetOrigin(NearestEnemy[client]));
		}
	}
}

stock int ObjectiveMovement(int client)
{
	char currentMap[PLATFORM_MAX_PATH];
	GetCurrentMap(currentMap, sizeof(currentMap));
	
	if(StrContains(currentMap, "koth_" , false) != -1)
	{
		GameModeKoth(client);
	}
	else if(StrContains(currentMap, "ctf_" , false) != -1)
	{
		GameModeCTF(client);
	}
	else if(StrContains(currentMap, "plr_" , false) != -1)
	{
		GameModePayloadRace(client);
	}
	else if(StrContains(currentMap, "cp_" , false) != -1)
	{
		GameModeControlPoint(client);
	}
	else if(StrContains(currentMap, "tc_" , false) != -1)
	{
		GameModeControlPoint(client);
	}
	else if(StrContains(currentMap, "pl_" , false) != -1)
	{
		GameModePayload(client);
	}
	else
	{
		GameModeDeathmatch(client);
	}
}

stock bool NeedEscape(int client)
{
	if(TF2_IsPlayerInCondition(client, TFCond_TeleportedGlow) // enemies can find position of the our teleporter, we don't like this!
	|| TF2_IsPlayerInCondition(client, TFCond_CloakFlicker) 
	|| TF2_IsPlayerInCondition(client, TFCond_Bonked) 
	|| TF2_IsPlayerInCondition(client, TFCond_Dazed) 
	|| TF2_IsPlayerInCondition(client, TFCond_OnFire) 
	|| TF2_IsPlayerInCondition(client, TFCond_Jarated) 
	|| TF2_IsPlayerInCondition(client, TFCond_Bleeding) 
	|| TF2_IsPlayerInCondition(client, TFCond_Milked) 
	|| TF2_IsPlayerInCondition(client, TFCond_Parachute) // slow :/
	|| TF2_IsPlayerInCondition(client, TFCond_KnockedIntoAir) 
	|| TF2_IsPlayerInCondition(client, TFCond_ParachuteDeployed) // slow :/
	|| TF2_IsPlayerInCondition(client, TFCond_BurningPyro) 
	|| TF2_IsPlayerInCondition(client, TFCond_Gas) 
	|| TF2_IsPlayerInCondition(client, TFCond_ObscuredSmoke))
	{
		return true;
	}
	
	return false;
}

stock void HideFromTheTarget(int client, float targetposition[3])
{
	if(HidingSpotIsReady[client])
	{
		if(GetVectorDistance(GetOrigin(client), HidingPosition[client]) < 100.0)
			HidingSpotIsReady[client] = false;
		
		return;
	}
	
	float RandomPosition[3];
	
	RandomPosition[0] = g_flClientEyePos[client][0] + GetRandomFloat(-512.0, 512.0);
	RandomPosition[1] = g_flClientEyePos[client][1] + GetRandomFloat(-512.0, 512.0);
	RandomPosition[2] = g_flClientEyePos[client][2];
	
	NavArea area = TheNavMesh.GetNearestNavArea_Vec(RandomPosition, true, 7500.0, false, false, GetClientTeam(client));
	if(area != NavArea_Null)
	{
		area.GetRandomPoint(HidingPosition[client]);
		
		if(HasTFAttributes(area, BLUE_SPAWN_ROOM))
			return;
		
		if(HasTFAttributes(area, RED_SPAWN_ROOM))
			return;
		
		if(HasTFAttributes(area, RED_SENTRY))
			return;
		
		if(HasTFAttributes(area, BLUE_SENTRY))
			return;
		
		HidingPosition[client][2] += (g_flClientEyePos[client][2] - g_flClientOrigin[client][2]);
		
		if(IsPointVisible(HidingPosition[client], targetposition))
			return;
		
		if(PF_Exists(client) && !PF_IsPathToVectorPossible(client, HidingPosition[client]))
			return;
		
		HidingSpotIsReady[client] = true;
	}
}