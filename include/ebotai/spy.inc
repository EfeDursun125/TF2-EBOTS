int TargetEnemy[MAXPLAYERS + 1];

stock void SpyMoveLogic(int client)
{
	// TODO: Go To Enemy Spawn (%50 Chance) For Behind Attack & Sap Teleporters
	
	if(SapBuildings[client])
	{
		if(TargetBuilding[client] != -1 && IsValidEntity(TargetBuilding[client]) && IsValidEdict(TargetBuilding[client]))
		{
			TF2_FindPath(client, GetOrigin(TargetBuilding[client]));
			
			return;
		}
		else // target building is destroyed or already sapped by someone?
		{
			SapBuildings[client] = false;
			
			TargetBuilding[client] = -1;
		}
	}
	
	if(IsValidClient(TargetEnemy[client]) && IsPlayerAlive(TargetEnemy[client]) && TargetEnemy[client] != -1 && !IsStealthed(TargetEnemy[client]) && PF_IsPathToEntityPossible(client, TargetEnemy[client]))
	{
		TF2_FindPath(client, GetOrigin(TargetEnemy[client]));
		
		return;
	}
	else
	{
		if(HasEnemiesNear[client])
		{
			TargetEnemy[client] = NearestEnemy[client];
		}
		else
		{
			TargetEnemy[client] = -1;
		}
	}
	
	ObjectiveMovement(client);
}

stock void SpyAimLogic(int client)
{
	if(TF2_IsPlayerInCondition(client, TFCond_Disguised) && HasEnemiesNear[client] && IsClientAimingToMe(client, NearestEnemy[client]) && !IsAttacking(NearestEnemy[client])) // enemy looking at me.
	{
		TF2_LookAround(client);
		
		return;
	}
	
	if(TF2_IsPlayerInCondition(client, TFCond_Disguised) && HasEnemiesNear[client] && IsClientAimingToMe(client, NearestEnemy[client]) && IsAttacking(NearestEnemy[client])) // enemy looking at me.
	{
		LookAtEnemiens(client);
		
		return;
	}
	
	if(TF2_IsPlayerInCondition(client, TFCond_Disguised) && HasEnemiesNear[client] && GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client]), true) > 300 && IsClientVisibleInScreen(NearestEnemy[client], client)) // don't look at enemy.
	{
		if(HasFriendsNear[client])
		{
			TF2_LookAtPos(client, GetEyePosition(NearestFriend[client]), EBotAimSpeed[client]);
		}
		else
		{
			TF2_LookAround(client);
		}
		
		return;
	}
	
	LookAtEnemiens(client);
}

stock void SpyReactChecker(int client) 
{
	if(HasEnemiesNear[client])
	{
		if(!IsValidClient(NearestEnemy[client]))
			return;
		
		if(g_bHealthIsLow[client] && !TF2_IsPlayerInCondition(client, TFCond_Disguised) && !TF2_IsPlayerInCondition(client, TFCond_Cloaked))
		{
			ForcePressButton[client] |= IN_ATTACK2;
		}
		
		if(IsClientAimingToMe(client, NearestEnemy[client]) && IsAttacking(NearestEnemy[client]) && !TF2_IsPlayerInCondition(client, TFCond_Cloaked))
		{
			if(!g_bHealthIsLow[client] && GetClientHealth(NearestEnemy[client]) < 175.0) // maybe i can kill him?
			{
				ForcePressButton[client] |= IN_ATTACK;
			}
			else // maybe not.
			{
				ForcePressButton[client] |= IN_ATTACK2;
			}
		}
	}
	
	if(HasEnemyBuildingsNear[client])
	{
		if(!IsValidEntity(NearestEnemyBuilding[client]))
			return;
		
		if(!TF2_IsPlayerInCondition(client, TFCond_Cloaked) && !TF2_IsPlayerInCondition(client, TFCond_Disguised))
		{
			ForcePressButton[client] |= IN_ATTACK2;
		}
	}
	
	if(!HasEnemiesNear[client] && !HasEnemyBuildingsNear[client] && TF2_IsPlayerInCondition(client, TFCond_Disguised))
	{
		if(TF2_IsPlayerInCondition(client, TFCond_Cloaked) && TF2_IsPlayerInCondition(client, TFCond_Disguised))
		{
			ForcePressButton[client] |= IN_ATTACK2;
		}
	}
	
	if(IsWeaponSlotActive(client, 5) && !HasEnemiesNear[client] && !HasEnemyBuildingsNear[client])
	{
		EquipWeaponSlot(client, 2);
	}
}