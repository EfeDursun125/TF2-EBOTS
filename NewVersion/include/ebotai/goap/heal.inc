public bool HealRequirements(int client)
{
	if (TF2_HasTheFlag(client))
		return false;
	return true;
}

public void HealStart(int client)
{
	m_goalEntity[client] = -1;
}

public void HealUpdate(int client)
{
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);

	if (m_hasEnemiesNear[client] && m_hasFriendsNear[client] && m_friendDistance[client] > m_enemyDistance[client])
		SetProcess(client, PRO_ATTACK, 180.0, "", true);
	
	if (m_hasFriendsNear[client])
	{
		m_healTarget[client] = m_nearestFriend[client];
		CurrentProcessTime[client] = GetGameTime() + 10.0; // no timeout if we have friends nearby
	}
	
	if (IsValidClient(m_healTarget[client]) && IsPlayerAlive(m_healTarget[client]) && GetVectorDistance(GetOrigin(client), GetOrigin(m_healTarget[client]), true) <= Squared(GetConVarInt(m_eBotMedicFollowRange)))
	{
		if (IsWeaponSlotActive(client, 1))
		{
			if (GetClientHealth(client) <= 60)
				m_buttons[client] |= IN_ATTACK2;
			m_lookAt[client] = GetEyePosition(m_healTarget[client]);
			m_attackTimer[client] = GetGameTime() + 0.4;
			if (m_isSlowThink[client] && IsValidClient(TF2_GetHealingTarget(client)) && TF2_GetHealingTarget(client) != m_healTarget[client])
			{
				m_attackTimer[client] = 0.0;
				BackstabMove(client, m_healTarget[client], false);
			}
			if (m_hasEnemiesNear[client] && !ClientViews(client, m_nearestEnemy[client]))
				BackstabMove(client, m_healTarget[client], false);
		}
		else
			EquipWeaponSlot(client, 1);
	}
	else
	{
		if (IsValidClient(m_healTarget[client]) && IsPlayerAlive(m_healTarget[client]))
		{
			m_lookAt[client] = GetEyePosition(m_healTarget[client]);
			FollowPath(client, GetOrigin(m_healTarget[client]));
		}
	}
}

public void HealEnd(int client)
{
	m_goalEntity[client] = -1;
	EquipWeaponSlot(client, 0);
}