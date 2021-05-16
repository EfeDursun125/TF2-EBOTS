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
				EquipWeaponSlot(client, 1);
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
	
	if(HasFriendsNear[client] && IsClientAimingToMe(client, NearestFriend[client]) && GetVectorDistance(GetOrigin(client), GetOrigin(NearestFriend[client])) < 200.0 && IsAttacking(NearestFriend[client]) && IsWeaponSlotActive(NearestFriend[client], 2))
	{
		EquipWeaponSlot(client, 2);
		TF2_LookAtPos(client, GetEyePosition(NearestFriend[client]), EBotAimSpeed[client]);
		AttackTimer[client] = GetGameTime() + 0.2;
		
		return;
	}
	
	if(TF2_GetPlayerClass(client) == TFClass_Engineer)
	{
		EngineerAimLogic(client);
		
		return;
	}
	
	if(TF2_GetPlayerClass(client) == TFClass_DemoMan && DefendFlag[client])
	{
		DemomanAim(client);
		
		return;
	}
	
	TF2_LookAround(client);
}


stock int DemomanAim(int client)
{
	char currentMap[PLATFORM_MAX_PATH];
	GetCurrentMap(currentMap, sizeof(currentMap));
	
	if(StrContains(currentMap, "ctf_" , false) != -1)
	{
		DemomanAimToFlagLogic(client);
		
		return;
	}
	
	TF2_LookAround(client);
}

float SelectedStickyArea[MAXPLAYERS + 1][3];

float DemoAttackTimer[MAXPLAYERS + 1];

stock int DemomanAimToFlagLogic(int client)
{
	int flag;
	while((flag = FindEntityByClassname(flag, "item_teamflag")) != INVALID_ENT_REFERENCE)
	{
		int FlagStatus = GetEntProp(flag, Prop_Send, "m_nFlagStatus");
		if(IsValidEntity(flag) && GetClientTeam(client) == GetTeamNumber(flag))
		{
			if(!IsPointVisible(GetEyePosition(client), GetOrigin(flag)))
			{
				TF2_LookAround(client);
				
				return;
			}
			else if(FlagStatus == 1)
			{
				if(IsWeaponSlotActive(client, 1) && GetPlayerStickyCount(client) < 8)
				{
					if(DemoAttackTimer[client] < GetGameTime())
					{
						ForcePressButton[client] |= IN_ATTACK;
						
						DemoAttackTimer[client] = GetGameTime() + 0.2;
					}
				}
				else if(GetPlayerStickyCount(client) < 8)
				{
					EquipWeaponSlot(client, 1);
				}
				
				TF2_LookAtPos(client, GetOrigin(flag), EBotAimSpeed[client]);
			}
			else if(GetPlayerStickyCount(client) < 8)
			{
				if(IsSlowThink[client])
				{
					NavArea lookarea = TheNavMesh.GetNearestNavArea_Vec(GetOrigin(flag), true, 25000.0, true, true, GetClientTeam(client));
					if(lookarea != NavArea_Null)
					{
						lookarea.GetRandomPoint(SelectedStickyArea[client]);
					}
					
					StopTime[client] = GetGameTime() + GetRandomFloat(2.0, 5.0);
				}
				
				if(IsWeaponSlotActive(client, 1))
				{
					if(DemoAttackTimer[client] < GetGameTime())
					{
						ForcePressButton[client] |= IN_ATTACK;
						
						DemoAttackTimer[client] = GetGameTime() + 0.2;
					}
				}
				else
				{
					EquipWeaponSlot(client, 1);
				}
				
				TF2_LookAtPos(client, SelectedStickyArea[client], EBotAimSpeed[client]);
			}
		}
	}
}