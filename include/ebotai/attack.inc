float FireTimer[MAXPLAYERS + 1];

stock void Attack(int client)
{
	if(TF2_GetPlayerClass(client) == TFClass_DemoMan && IsWeaponSlotActive(client, 0) && PrimaryID[client] == 996)
	{
		if(!IsSlowThink[client])
		{
			AutoAttack(client);
		}
	}
	else if(IsSlowThink[client])
	{
		if(TF2_GetPlayerClass(client) != TFClass_Medic && !CanAttack[client]) // its medic only, disabled for other classes.
		{
			CanAttack[client] = true;
			
			return;
		}
		
		if(HasEnemiesNear[client] && TF2_GetPlayerClass(client) == TFClass_Spy)
		{
			SpyAttack(client);
			SelectBestCombatWeapon(client);
			
			return;
		}
		
		if(HasEnemiesNear[client] && TF2_GetPlayerClass(client) != TFClass_Spy && CanAttack[client])
		{
			AutoAttack(client);
			SelectBestCombatWeapon(client);
			
			return;
		}
	}
}

stock void AutoAttack(int client)
{
	int i = NearestEnemy[client];
	
	if(HasEnemiesNear[client] && TF2_GetPlayerClass(client) == TFClass_Engineer)
	{
		AttackTimer[client] = GetGameTime() + 1.1;
		
		return;
	}
	
	if (TF2_GetPlayerClass(client) == TFClass_Heavy)
	{
		if(IsWeaponSlotActive(client, 0))
		{
			int Primary = GetPlayerWeaponSlot(client, TFWeaponSlot_Primary);
			
			if(IsValidEntity(Primary) && HasAmmo(Primary))
			{
				if(g_bHealthIsLow[client])
				{
					Attack2Timer[client] = GetGameTime() + 2.0;
				}
				else
				{
					Attack2Timer[client] = GetGameTime() + 4.0;
				}
				
				AttackTimer[client] = GetGameTime() + 1.1;
			}
			else
			{
				AttackTimer[client] = 0.0;
				Attack2Timer[client] = 0.0;
			}
		}
		else
		{
			AttackTimer[client] = GetGameTime() + 1.1;
		}
		
		return;
	}
	
	if(TF2_GetPlayerClass(client) == TFClass_Pyro && IsWeaponSlotActive(client, 0))
	{
		AttackTimer[client] = GetGameTime() + 1.1;
		
		return;
	}
	
	if(TF2_GetPlayerClass(client) == TFClass_DemoMan)
	{
		if(IsWeaponSlotActive(client, 0) && PrimaryID[client] == 996)
		{
			if(FireTimer[client] < GetGameTime())
			{
				FireTimer[client] = GetGameTime() + 0.1;
			}
			else if(FireTimer[client] > GetGameTime())
			{
				ForcePressButton[client] |= IN_ATTACK;
			}
		}
		else
		{
			AttackTimer[client] = GetGameTime() + 1.1;
		}
		
		return;
	}
	
	if(TF2_GetPlayerClass(client) == TFClass_Soldier)
	{
		if(IsWeaponSlotActive(client, 0) && PrimaryID[client] == 730)
		{
			int ClipAmmo = GetEntProp(GetPlayerWeaponSlot(client, 0), Prop_Send, "m_iClip1");
			if(ClipAmmo < 3)
			{
				AttackTimer[client] = GetGameTime() + 1.1;
			}
		}
		else
		{
			AttackTimer[client] = GetGameTime() + 1.1;
		}
		
		return;
	}
	
	if (!HasEnemyBuildingsNear[client] && TF2_GetPlayerClass(client) == TFClass_Sniper)
	{
		return; // Sniper Is Buggy (DON'T REMOVE THIS!)
	}
	
	if(HasEnemiesNear[client] && IsValidClient(i) && !TF2_IsPlayerInCondition(i, TFCond_Ubercharged))
	{
		if(!TF2_IsPlayerInCondition(client, TFCond_Zoomed))
		{
			AttackTimer[client] = GetGameTime() + 1.1;
		}
	}
	else
	{
		AttackTimer[client] = GetGameTime() + 1.1;
	}
}

stock void SpyAttack(int client)
{
	int i = NearestEnemy[client];
	
	if(GetVectorDistance(GetOrigin(client), GetOrigin(i)) < 300.0)
	{
		EquipWeaponSlot(client, 2);
	}
	else
	{
		EquipWeaponSlot(client, 0);
	}
	
	if(IsWeaponSlotActive(client, 2) && IsValidEntity(GetPlayerWeaponSlot(client, TFWeaponSlot_Melee)) && GetEntProp(GetPlayerWeaponSlot(client, TFWeaponSlot_Melee), Prop_Send, "m_bReadyToBackstab"))
	{
		AttackTimer[client] = GetGameTime() + 1.1;
	}
	
	if(!HasEnemyBuildingsNear[client] && IsReadyForAttack(client))
	{
		AttackTimer[client] = GetGameTime() + 1.1;
		
		return;
	}
	
	if(!HasEnemyBuildingsNear[client] && HasEnemiesNear[client] && IsValidClient(i) && ChanceOf(EBotSenseChance[client]) && TF2_IsPlayerInCondition(NearestEnemy[client], TFCond_Taunting) && HasFriendsNear[client])
	{
		AttackTimer[client] = GetGameTime() + 1.1;
		
		return;
	}
	
	if(!HasEnemyBuildingsNear[client] && HasEnemiesNear[client] && IsValidClient(i) && !ChanceOf(EBotSenseChance[client]) && (TF2_IsPlayerInCondition(NearestEnemy[client], TFCond_Disguised) || GetClientHealth(i) < 60.0))
	{
		AttackTimer[client] = GetGameTime() + 1.1;
		
		return;
	}
}

stock bool IsReadyForAttack(int client) // 1 core cpu killer... 
{
	if(TF2_IsPlayerInCondition(client, TFCond_Ubercharged) 
	|| TF2_IsPlayerInCondition(client, TFCond_TmpDamageBonus) 
	|| TF2_IsPlayerInCondition(client, TFCond_Buffed) 
	|| TF2_IsPlayerInCondition(client, TFCond_CritOnFirstBlood) 
	|| TF2_IsPlayerInCondition(client, TFCond_OnFire) 
	|| TF2_IsPlayerInCondition(client, TFCond_Jarated) 
	|| TF2_IsPlayerInCondition(client, TFCond_Bleeding) 
	|| TF2_IsPlayerInCondition(client, TFCond_Milked) 
	|| TF2_IsPlayerInCondition(client, TFCond_CritCanteen) 
	|| TF2_IsPlayerInCondition(client, TFCond_CritOnWin) 
	|| TF2_IsPlayerInCondition(client, TFCond_CritOnFlagCapture) 
	|| TF2_IsPlayerInCondition(client, TFCond_CritOnKill) 
	|| TF2_IsPlayerInCondition(client, TFCond_CritOnDamage) 
	|| TF2_IsPlayerInCondition(client, TFCond_MiniCritOnKill) 
	|| TF2_IsPlayerInCondition(client, TFCond_CritRuneTemp) 
	|| TF2_IsPlayerInCondition(client, TFCond_Gas) 
	|| TF2_IsPlayerInCondition(client, TFCond_ObscuredSmoke) 
	|| TF2_IsPlayerInCondition(client, TFCond_HalloweenGiant) 
	|| TF2_IsPlayerInCondition(client, TFCond_UberchargedOnTakeDamage) 
	|| TF2_IsPlayerInCondition(client, TFCond_UberchargedHidden) 
	|| TF2_IsPlayerInCondition(client, TFCond_UberchargedCanteen) 
	|| TF2_IsPlayerInCondition(client, TFCond_HalloweenBombHead) 
	|| TF2_IsPlayerInCondition(client, TFCond_NoHealingDamageBuff) 
	|| TF2_IsPlayerInCondition(client, TFCond_RegenBuffed) 
	|| TF2_IsPlayerInCondition(client, TFCond_DefenseBuffed) 
	|| TF2_IsPlayerInCondition(client, TFCond_SpeedBuffAlly) 
	|| TF2_HasTheFlag(NearestEnemy[client]))
	{
		return true;
	}
	
	return false;
}