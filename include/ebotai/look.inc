stock void Look(int client)
{
	if(HasEnemiesNear[client] && TF2_IsPlayerInCondition(client, TFCond_Taunting))
	{
		FakeClientCommandThrottled(client, "taunt");
		
		return;
	}
	
	if(TF2_IsPlayerInCondition(client, TFCond_Taunting))
	{
		return;
	}
	
	if(TF2_GetPlayerClass(client) == TFClass_Pyro && HasFriendsNear[client] && IsValidClient(NearestFriend[client]) && TF2_IsPlayerInCondition(NearestFriend[client], TFCond_OnFire) && TF2_GetPlayerClass(NearestFriend[client]) != TFClass_Pyro)
	{
		int i = NearestFriend[client];
		
		if(HasEnemiesNear[client] && GetVectorDistance(GetEyePosition(client), GetEyePosition(i)) < 250.0)
		{
			TF2_LookAtPos(client, GetEyePosition(i), EBotAimSpeed[client]);
			
			if(IsWeaponSlotActive(client, 0))
			{
				Attack2Timer[client] = GetGameTime() + 0.2;
			}
			else
			{
				EquipWeaponSlot(client, 0);
			}
			
			return;
		}
		else if(!HasEnemiesNear[client])
		{
			TF2_LookAtPos(client, GetEyePosition(i), EBotAimSpeed[client]);
			
			if(IsWeaponSlotActive(client, 0))
			{
				Attack2Timer[client] = GetGameTime() + 0.2;
			}
			else
			{
				EquipWeaponSlot(client, 0);
			}
			
			return;
		}
	}
	
	if(!HasEnemiesNear[client] && HasFriendsNear[client] && IsValidClient(NearestFriend[client]) && TF2_IsPlayerInCondition(NearestFriend[client], TFCond_Taunting))
	{
		TF2_LookAtPos(client, GetEyePosition(NearestFriend[client]), EBotAimSpeed[client]);
		
		return;
	}
	
	if(!TF2_IsPlayerInCondition(client, TFCond_CritOnWin) && HasEnemiesNear[client] && HasFriendsNear[client] && TF2_GetPlayerClass(client) == TFClass_Medic)
	{
		int i = NearestFriend[client];
		int x = NearestEnemy[client];
		
		if(!IsValidClient(i) || !IsValidClient(x))
			return;
		
		if(GetVectorDistance(GetEyePosition(client), GetEyePosition(i)) < GetVectorDistance(GetEyePosition(client), GetEyePosition(x)))
		{
			TF2_LookAtPos(client, GetEyePosition(i), EBotAimSpeed[client]);
			
			if(!IsWeaponSlotActive(client, 1))
			{
				EquipWeaponSlot(client, 1);
			}
			else if(TF2_GetHealingTarget(client) == i)
			{
				AttackTimer[client] = GetGameTime() + 0.5;
			}
			
			CanAttack[client] = false;
		}
		else
		{
			LookAtEnemiens(client);
			CanAttack[client] = true;
		}
		
		return;
	}
	
	/*if(HasEyeBallBossNear[client])
	{
		if(IsValidEntity(NearestEyeBallBoss[client]))
		{
			TF2_LookAtPos(client, WorldSpaceCenter(NearestEyeBallBoss[client]), EBotAimSpeed[client]);
			SelectBestCombatWeaponForVector(client, GetOrigin(NearestEyeBallBoss[client]));
			AutoAttack(client);
		}
		
		return;
	}*/
	
	if(TF2_GetPlayerClass(client) == TFClass_Spy && HasEnemyBuildingsNear[client] && !TF2_IsPlayerInCondition(client, TFCond_Cloaked)) // spy must prefer the sap building
	{
		if(IsValidEntity(NearestEnemyBuilding[client]) && GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemyBuilding[client])) < 200.0)
		{
			TF2_LookAtPos(client, WorldSpaceCenter(NearestEnemyBuilding[client]), EBotAimSpeed[client]);
			FakeClientCommandThrottled(client, "build 3 0");
			if(IsWeaponSlotActive(client, 1))
			{
				AttackTimer[client] = GetGameTime() + 0.2;
			}
			else
			{
				EquipWeaponSlot(client, 5);
			}
			
			return;
		}
	}
	
	if(HasEnemiesNear[client] && TF2_GetPlayerClass(client) == TFClass_Engineer)
	{
		EngineerAimLogic(client);
		
		return;
	}
	
	if(HasEnemiesNear[client])
	{
		if(TF2_GetPlayerClass(client) == TFClass_Spy)
		{
			SpyAimLogic(client);
		}
		else
		{
			LookAtEnemiens(client);
			SelectBestCombatWeapon(client);
		}
		
		CanAttack[client] = true;
		
		return;
	}
	
	if(TF2_GetPlayerClass(client) != TFClass_Spy && HasEnemyBuildingsNear[client] && !TF2_IsPlayerInCondition(client, TFCond_Cloaked))
	{
		if(IsValidEntity(NearestEnemyBuilding[client]))
		{
			TF2_LookAtPos(client, WorldSpaceCenter(NearestEnemyBuilding[client]), EBotAimSpeed[client]);
			SelectBestCombatWeaponForVector(client, GetOrigin(NearestEnemyBuilding[client]));
			AutoAttack(client);
			
			return;
		}
	}
	
	if(HasFriendsNear[client] && TF2_GetPlayerClass(client) == TFClass_Medic)
	{
		int i = NearestFriend[client];
		
		if(!IsValidClient(i))
			return;
		
		TF2_LookAtPos(client, GetEyePosition(i), EBotAimSpeed[client]);
		
		if(!IsWeaponSlotActive(client, 1))
		{
			EquipWeaponSlot(client, 1);
		}
		else
		{
			AttackTimer[client] = GetGameTime() + 0.2;
		}
		
		CanAttack[client] = false;
		
		return;
	}
	
	if(!HasEnemiesNear[client] && TF2_HasTheFlag(client))
	{
		return;
	}
	
	if(TF2_GetPlayerClass(client) == TFClass_Engineer)
	{
		EngineerAimLogic(client);
		
		return;
	}
	
	if(TF2_GetPlayerClass(client) == TFClass_DemoMan && DefendMode[client])
	{
		DemomanAim(client);
		
		return;
	}
	
	if(HasFriendsNear[client] && IsValidClient(NearestFriend[client]) && IsAttacking(NearestFriend[client]) && IsWeaponSlotActive(NearestFriend[client], 2) && GetVectorDistance(GetOrigin(client), GetOrigin(NearestFriend[client])) <= 100.0)
	{
		EquipWeaponSlot(client, 2);
		TF2_LookAtPos(client, GetEyePosition(NearestFriend[client]), EBotAimSpeed[client]);
		AttackTimer[client] = GetGameTime() + 0.5;
		
		return;
	}
	
	TF2_LookAround(client);
}