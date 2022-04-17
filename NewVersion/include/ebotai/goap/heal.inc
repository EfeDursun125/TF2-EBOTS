stock bool HealRequirements(int client)
{
	if (TF2_GetPlayerClass(client) != TFClass_Medic)
		return false;
	return true;
}

stock void HealStart(int client)
{
	m_goalEntity[client] = -1;
}

stock void HealUpdate(int client)
{
	if (m_hasEnemiesNear[client] && TF2_IsPlayerInCondition(client, TFCond_CritOnWin))
	{
		LookUpdate(client);
		FollowPath(client, GetOrigin(m_nearestEnemy[client]));
		FindFriendsAndEnemiens(client);
		return;
	}

	FindFriendsAndEnemiens(client);

	if (m_hasEnemiesNear[client] && m_hasFriendsNear[client] && m_friendDistance[client] > m_enemyDistance[client])
		SetProcess(client, PRO_ATTACK, true, 180.0, "", true);

	//CheckHealth(client); not ready now
	CheckAmmo(client);

	if (m_hasFriendsNear[client])
	{
		CurrentProcessTime[client] = GetGameTime() + 6.0; // no timeout if we have friends nearby
		if (m_friendDistance[client] < Squared(GetConVarInt(m_eBotMedicFollowRange)))
		{
			if (IsWeaponSlotActive(client, 1))
			{
				if (GetClientHealth(client) <= 60)
					m_buttons[client] |= IN_ATTACK2;
				m_lookAt[client] = GetEyePosition(m_nearestFriend[client]);
				if (TF2_GetHealingTarget(client) == m_nearestFriend[client])
					m_attackTimer[client] = GetGameTime() + 0.4;
				
				if (m_hasEnemiesNear[client] && !ClientViews(client, m_nearestEnemy[client]))
					BackstabMove(client, m_nearestFriend[client], false);
			}
			else
				EquipWeaponSlot(client, 1);
		}
		else
		{
			FollowPath(client, GetOrigin(m_nearestFriend[client]));
			LookUpdate(client);
		}
	}
	else
		LookUpdate(client);
}

stock void HealEnd(int client)
{
	m_goalEntity[client] = -1;
}