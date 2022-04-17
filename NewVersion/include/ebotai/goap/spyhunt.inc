stock bool SpyHuntRequirements(int client)
{
	if (m_hasEnemiesNear[client] && TF2_IsPlayerInCondition(client, TFCond_Cloaked) && IsVisible(GetEyePosition(client), GetEyePosition(m_nearestEnemy[client])))
		return false;
	
	if (!TF2_IsPlayerInCondition(client, TFCond_Disguised))
		return false;
	
	if (!IsValidClient(m_goalEntity[client]) || !IsPlayerAlive(m_goalEntity[client]))
		return false;
	
	if (TF2_HasTheFlag(client))
		return false;
	
	return true;
}

stock void SpyHuntStart(int client)
{
	EquipWeaponSlot(client, 2);
}

stock void SpyHuntUpdate(int client)
{
	if (TF2_IsPlayerInCondition(client, TFCond_CritOnWin) && m_hasEnemiesNear[client])
	{
		FollowPath(client, GetOrigin(m_nearestEnemy[client]));
		FindFriendsAndEnemiens(client);
		LookUpdate(client);
		return;
	}

	if (!IsValidClient(m_goalEntity[client]) || !IsPlayerAlive(m_goalEntity[client]))
	{
		FinishCurrentProcess(client);
		return;
	}

	SpyReactChecker(client);
	SpyAimLogic(client);

	float distance = GetVectorDistance(GetOrigin(client), GetOrigin(m_goalEntity[client]), true);
	int range = Squared(256);
	if (TF2_IsPlayerInCondition(client, TFCond_Cloaked) && distance <= range)
	{
		BackstabMove(client, m_goalEntity[client]);
		if (distance <= Squared(128))
			m_moveVel[client][0] = -500.0;
		return;
	}
	else
	{
		if (m_hasEnemiesNear[client] && m_enemiesNearCount[client] > 1)
		{
			float distance2 = GetVectorDistance(GetOrigin(client), GetOrigin(m_nearestEnemy[client]), true);
			int range2 = Squared(256);
			if (distance2 <= range2)
			{
				BackstabMove(client, m_nearestEnemy[client]);
				if (distance2 <= Squared(128))
					m_moveVel[client][0] = -500.0;
				return;
			}
			else if (distance >= range)
				FollowPath(client, GetOrigin(m_goalEntity[client]));
			else if (IsClientAimingToMe(client, m_nearestEnemy[client]) && IsAttacking(m_nearestEnemy[client]) && GetRandomInt(1, 2) == 1)
				SetProcess(client, PRO_ATTACK, true, 180.0, "", true);
		}
		else
		{
			if (distance <= range)
			{
				EquipWeaponSlot(client, 2);
				BackstabMove(client, m_goalEntity[client]);
			}
			else
				FollowPath(client, GetOrigin(m_goalEntity[client]));
		}
	}

	FindFriendsAndEnemiens(client);
	CheckHealth(client);
	CheckAmmo(client);
}

stock void SpyHuntEnd(int client)
{
	if (IsValidClient(m_goalEntity[client]) && ChanceOf(m_eBotSenseChance[client]))
		FakeClientCommandThrottled(client, "lastdisguise");
}

stock void SpyReactChecker(int client) 
{
	if (m_hasEnemiesNear[client])
	{
		if (m_lowHealth[client] && !TF2_IsPlayerInCondition(client, TFCond_Disguised) && !TF2_IsPlayerInCondition(client, TFCond_Cloaked))
			m_buttons[client] |= IN_ATTACK2;
		
		if (IsClientAimingToMe(client, m_nearestEnemy[client]) && IsAttacking(m_nearestEnemy[client]) && !TF2_IsPlayerInCondition(client, TFCond_Cloaked))
		{
			if (!m_lowHealth[client] && GetClientHealth(m_nearestEnemy[client]) <= 175.0) // maybe i can kill him?
			{
				EquipWeaponSlot(client, 0);
				SetProcess(client, PRO_ATTACK, false, 180.0, "", true);
			}
			else // maybe not.
				m_buttons[client] |= IN_ATTACK2;
		}
	}
	
	/*if (HasEnemyBuildingsNear[client])
	{
		if (!IsValidEntity(NearestEnemyBuilding[client]))
			return;
		
		if (!TF2_IsPlayerInCondition(client, TFCond_Cloaked) && !TF2_IsPlayerInCondition(client, TFCond_Disguised))
			m_buttons[client] |= IN_ATTACK2;
	}*/
	
	if (!m_hasEnemiesNear[client])// && (!HasEnemyBuildingsNear[client]) && TF2_IsPlayerInCondition(client, TFCond_Disguised))
	{
		if (TF2_IsPlayerInCondition(client, TFCond_Cloaked) && TF2_IsPlayerInCondition(client, TFCond_Disguised))
			m_buttons[client] |= IN_ATTACK2;
	}
	
	if (IsWeaponSlotActive(client, 5) && !m_hasEnemiesNear[client]) //&& !HasEnemyBuildingsNear[client])
		EquipWeaponSlot(client, 2);
}

stock void SpyAimLogic(int client)
{
	if (TF2_IsPlayerInCondition(client, TFCond_Disguised) && m_hasEnemiesNear[client] && GetClientAimTarget(m_nearestEnemy[client]) == client && IsAttacking(m_nearestEnemy[client])) // enemy looking at me
	{
		LookAtEnemiens(client);
		return;
	}
	
	int sqDist = Squared(300);
	if (TF2_IsPlayerInCondition(client, TFCond_Disguised) && m_hasEnemiesNear[client] && m_enemyDistance[client] > sqDist && GetClientAimTarget(m_nearestEnemy[client]) == client) // don't look at enemy
	{
		if (m_hasFriendsNear[client])
			m_lookAt[client] = GetEyePosition(m_hasFriendsNear[client]);
		else
			LookAround(client);
		return;
	}
	else if (TF2_IsPlayerInCondition(client, TFCond_Disguised) && m_hasEnemiesNear[client] && !m_hasFriendsNear[client])
	{
		if (m_enemyDistance[client] <= sqDist)
			LookAtEnemiens(client);
		else
			LookAround(client);
		return;
	}

	if (m_hasEnemiesNear[client])
		LookAtEnemiens(client);
	else
		LookAround(client);
}
