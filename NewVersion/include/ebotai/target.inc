stock void FindFriendsAndEnemiens(int client, bool noteammates = false)
{
	m_enemyDistance[client] = -1.0;
	m_friendDistance[client] = -1.0;
	m_enemiesNearCount[client] = 0;
	m_friendsNearCount[client] = 0;
	m_hasEnemiesNear[client] = false;
	m_hasFriendsNear[client] = false;
	
	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientInGame(i))
			continue;
		
		if (!IsPlayerAlive(i))
			continue;
		
		if (i == client)
			continue;
		
		if (!IsVisible(GetEyePosition(client), GetEyePosition(i)))
			continue;
		
		if (GetClientTeam(i) == GetClientTeam(client))
		{
			if (noteammates)
				return;
			
			m_friendsNearCount[client] += 1;
			float edict_distance = GetVectorDistance(GetOrigin(client), GetOrigin(i), true);
			if (edict_distance < m_friendDistance[client] || m_friendDistance[client] == -1.0)
			{
				m_friendDistance[client] = edict_distance;
				m_nearestFriend[client] = i;
			}
		}
		else
		{
			if (!ClientViews(client, i) && !IsAttacking(i) && TF2_GetPlayerClass(client) != TFClass_Medic && !TF2_IsPlayerInCondition(client, TFCond_Zoomed) && !TF2_IsPlayerInCondition(client, TFCond_Disguised) && !TF2_IsPlayerInCondition(client, TFCond_Taunting))
				continue;

			if (TF2_GetPlayerClass(i) == TFClass_Spy && !IsRevealedSpyByTarget(i, client))
				continue;
			
			m_enemiesNearCount[client] += 1;
			float edict_distance = GetVectorDistance(GetOrigin(client), GetOrigin(i), true);
			if (edict_distance < m_enemyDistance[client] || m_enemyDistance[client] == -1.0)
			{
				m_enemyDistance[client] = edict_distance;
				m_nearestEnemy[client] = i;
			}
		}
	}
	
	m_hasEnemiesNear[client] = m_enemiesNearCount[client] > 0;
	m_hasFriendsNear[client] = m_friendsNearCount[client] > 0;
}

stock void FindFriendsForHeal(int client)
{
	m_friendDistance[client] = -1.0;
	m_friendsNearCount[client] = 0;
	m_hasFriendsNear[client] = false;
	
	for (int i = 1; i <= MaxClients; i++)
	{
		if (i == client)
			continue;
		
		if (!IsValidClient(i))
			continue;
		
		if (!IsPlayerAlive(i))
			continue;
		
		if (GetClientTeam(i) != GetClientTeam(client))
			continue;
		
		if (!IsPointVisible(GetEyePosition(client), GetEyePosition(i)))
			continue;
		
		if (TF2_IsPlayerInCondition(i, TFCond_Cloaked))
			continue;
		
		if (TF2_IsPlayerInCondition(i, TFCond_Disguised) && TF2_IsPlayerInCondition(i, TFCond_Overhealed))
			continue;
		
		if (TF2_GetPlayerClass(i) == TFClass_Engineer && IsFakeClient(i) && IsWeaponSlotActive(i, 2) && TF2_IsPlayerInCondition(i, TFCond_Overhealed))
			continue;
		
		if (TF2_GetPlayerClass(i) == TFClass_Medic && TF2_IsPlayerInCondition(i, TFCond_Overhealed))
			continue;
		
		if (TF2_IsPlayerInCondition(i, TFCond_Zoomed) && TF2_IsPlayerInCondition(i, TFCond_Overhealed))
			continue;
		
		if (TF2_GetPlayerClass(i) == TFClass_Engineer && IsFakeClient(i) && IsWeaponSlotActive(i, 5))
			continue;
		
		m_friendsNearCount[client] += 1;
		m_friendDistance[client] = (GetClientHealth(i) * GetVectorDistance(GetEyePosition(client), GetEyePosition(i), true)); // prefer lower health targets
		if (m_friendDistance[client] < distance || distance == -1.0)
		{
			distance = m_friendDistance[client];
			m_nearestFriend[client] = i;
		}
	}
	
	m_hasFriendsNear[client] = m_friendsNearCount[client] > 0;
}