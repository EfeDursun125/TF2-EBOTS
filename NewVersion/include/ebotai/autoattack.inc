float m_fireTimer[MAXPLAYERS + 1];

stock void Attack(int client)
{
	if (TF2_GetPlayerClass(client) == TFClass_DemoMan && IsWeaponSlotActive(client, 0) && TF2_GetPlayerWeaponID(client, 0) == 996)
	{
		if (!m_isSlowThink[client])
			AutoAttack(client);
	}
	else
	{
		if (TF2_GetPlayerClass(client) != TFClass_Medic && !m_canAttack[client]) // its medic only, currently disabled for other classes.
		{
			m_canAttack[client] = true;
			return;
		}
		
		if (m_hasEnemiesNear[client] && TF2_GetPlayerClass(client) == TFClass_Spy)
		{
			SpyAttack(client);
			return;
		}
		
		if (m_hasEnemiesNear[client] && TF2_GetPlayerClass(client) != TFClass_Spy && m_canAttack[client])
		{
			AutoAttack(client);
			return;
		}
	}
}

stock void AutoAttack(int client)
{
	if (IsReloading(client))
		return;
	
	if (m_hasEnemiesNear[client] && TF2_GetPlayerClass(client) == TFClass_Engineer)
	{
		m_attackTimer[client] = GetGameTime() + 0.4;
		return;
	}
	
	if (TF2_GetPlayerClass(client) == TFClass_Heavy)
	{
		if (IsWeaponSlotActive(client, 0))
		{
			if (GetAmmo(client, 0) > 0)
			{
				if (m_lowHealth[client])
					m_attack2Timer[client] = GetGameTime() + 2.0;
				else
					m_attack2Timer[client] = GetGameTime() + 4.0;
				m_attackTimer[client] = GetGameTime() + 0.4;
			}
			else
			{
				m_attackTimer[client] = 0.0;
				m_attack2Timer[client] = 0.0;
			}
		}
		else
			m_attackTimer[client] = GetGameTime() + 0.4;
		return;
	}
	
	if (TF2_GetPlayerClass(client) == TFClass_Pyro && IsWeaponSlotActive(client, 0))
	{
		m_attackTimer[client] = GetGameTime() + 0.4;
		return;
	}
	
	if (TF2_GetPlayerClass(client) == TFClass_DemoMan)
	{
		if (IsWeaponSlotActive(client, 0) && TF2_GetPlayerWeaponID(client, 0) == 996)
		{
			if (m_fireTimer[client] < GetGameTime())
				m_fireTimer[client] = GetGameTime() + 0.1;
			else if (m_fireTimer[client] > GetGameTime())
				m_buttons[client] |= IN_ATTACK;
		}
		else
			m_attackTimer[client] = GetGameTime() + 0.4;
		return;
	}
	
	if (TF2_GetPlayerClass(client) == TFClass_Soldier)
	{
		if (IsWeaponSlotActive(client, 0) && TF2_GetPlayerWeaponID(client, 0) == 730)
		{
			if (GetEntProp(GetPlayerWeaponSlot(client, 0), Prop_Send, "m_iClip1") < 3)
				m_attackTimer[client] = GetGameTime() + 0.4;
		}
		else
			m_attackTimer[client] = GetGameTime() + 0.4;
		return;
	}
	
	if (m_hasEnemiesNear[client] && !TF2_IsPlayerInCondition(m_nearestEnemy[client], TFCond_Ubercharged))
	{
		if (!TF2_IsPlayerInCondition(client, TFCond_Zoomed))
			m_attackTimer[client] = GetGameTime() + 0.4;
	}
	else
		m_attackTimer[client] = GetGameTime() + 0.45;
}

stock void SpyAttack(int client)
{
	if (IsReadyForAttack(client))
	{
		m_attackTimer[client] = GetGameTime() + 0.4;
		return;
	}
	
	if (m_hasEnemiesNear[client] && ChanceOf(m_eBotSenseChance[client]) && TF2_IsPlayerInCondition(m_nearestEnemy[client], TFCond_Taunting) && m_hasFriendsNear[client])
	{
		m_attackTimer[client] = GetGameTime() + 0.4;
		return;
	}
	
	if (m_hasEnemiesNear[client] && !ChanceOf(m_eBotSenseChance[client]) && (TF2_IsPlayerInCondition(m_nearestEnemy[client], TFCond_Disguised) || GetClientHealth(m_nearestEnemy[client]) <= 65.0))
	{
		m_attackTimer[client] = GetGameTime() + 0.4;
		return;
	}
}

stock bool IsReadyForAttack(int client) // 1 core cpu killer... 
{
	if (TF2_IsPlayerInCondition(client, TFCond_Ubercharged) 
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
	|| TF2_HasTheFlag(m_nearestEnemy[client]))
		return true;
	return false;
}

stock void SelectBestCombatWeapon(int client)
{
	float distance = m_hasEnemiesNear[client] ? m_enemyDistance[client] : GetVectorDistance(GetOrigin(client), m_lookAt[client], true);
	float meleerange = Squaredf(GetConVarFloat(EBotMelee));
	
	if (TF2_GetPlayerClass(client) == TFClass_Pyro)
		meleerange *= 0.5;

	if (TF2_IsPlayerInCondition(client, TFCond_Disguised))
		meleerange *= 2.0;

	if (distance <= meleerange)
		EquipWeaponSlot(client, 2);
	else
	{
		int clip0 = GetPrimaryClipN(client, 0);
		int clip1 = GetPrimaryClipN(client, 1);
		if (TF2_GetPlayerClass(client) == TFClass_Sniper)
		{
			if (clip1 > 0 && distance <= Squared(512))
				EquipWeaponSlot(client, 1);
			else if (clip0 > 0)
				EquipWeaponSlot(client, 0);
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Heavy)
		{
			if (clip1 > 0 && distance <= Squared(256))
				EquipWeaponSlot(client, 1);
			else if (clip0 > 0)
				EquipWeaponSlot(client, 0);
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Pyro)
		{
			if (clip0 > 0 && distance <= Squared(400))
				EquipWeaponSlot(client, 0);
			else if (clip1 > 0)
				EquipWeaponSlot(client, 1);
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Soldier)
		{
			if (clip1 > 0 && distance <= Squared(256))
				EquipWeaponSlot(client, 1);
			else if (clip1 > 0 && IsValidClient(m_nearestEnemy[client]) && TF2_GetPlayerClass(m_nearestEnemy[client]) == TFClass_Pyro && distance <= Squared(512))
				EquipWeaponSlot(client, 1);
			else if (clip0 > 0)
				EquipWeaponSlot(client, 0);
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Medic)
		{
			if (clip0 > 0)
				EquipWeaponSlot(client, 0);
		}
		else if(TF2_GetPlayerClass(client) == TFClass_DemoMan)
		{
			if (clip0 > 0)
				EquipWeaponSlot(client, 0);
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Scout)
		{
			if (clip1 > 0 && distance >= Squared(1024))
				EquipWeaponSlot(client, 1);
			else if (clip0 > 0)
				EquipWeaponSlot(client, 0);
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Engineer)
		{
			if (clip1 > 0 && distance >= Squared(1024))
				EquipWeaponSlot(client, 1);
			else if (clip0 > 0)
				EquipWeaponSlot(client, 0);
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Spy)
		{
			if (clip0 > 0)
				EquipWeaponSlot(client, 0);
		}

		if (IsWeaponSlotActive(client, 2) && distance > meleerange)
		{
			clip0 = GetPrimaryClip(client, 0);
			clip1 = GetPrimaryClip(client, 1);
			if (TF2_GetPlayerClass(client) == TFClass_Medic)
				EquipWeaponSlot(client, 0);
			else if (TF2_GetPlayerClass(client) == TFClass_Pyro || TF2_GetPlayerClass(client) == TFClass_Heavy)
			{
				if (clip1 <= 0)
					EquipWeaponSlot(client, 1);
				else
					EquipWeaponSlot(client, 0);
			}
			else if (clip0 <= 0)
				EquipWeaponSlot(client, 0);
			else if (clip1 <= 0)
				EquipWeaponSlot(client, 1);
			else
				EquipWeaponSlot(client, 0);
		}
	}
}