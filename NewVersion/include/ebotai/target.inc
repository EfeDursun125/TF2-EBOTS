stock void FindFriendsAndEnemiens(int client)
{
	m_enemyDistance[client] = 99999999.0;
	m_friendDistance[client] = 99999999.0;
	m_enemiesNearCount[client] = 0;
	m_friendsNearCount[client] = 0;
	m_hasEnemiesNear[client] = false;
	m_hasFriendsNear[client] = false;
	
	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsValidClient(i))
			continue;
		
		if (!IsPlayerAlive(i))
			continue;
		
		if (i == client)
			continue;
		
		if (!IsVisible(GetEyePosition(client), GetEyePosition(i)))
			continue;
		
		if (GetClientTeam(i) == GetClientTeam(client))
		{
			if (TF2_GetPlayerClass(client) == TFClass_Medic)
			{
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
				m_friendDistance[client] = GetVectorDistance(GetEyePosition(client), GetEyePosition(i), true);
				float dist = GetClientHealth(i) * m_friendDistance[client]; // prefer lower health
				float frienddistance = 99999999.0;
				if (dist < frienddistance)
				{
					frienddistance = dist;
					m_nearestFriend[client] = i;
				}
				return;
			}
			
			m_friendsNearCount[client] += 1;
			float edict_distance = GetVectorDistance(GetOrigin(client), GetOrigin(i), true);
			if (edict_distance < m_friendDistance[client])
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
			if (edict_distance < m_enemyDistance[client])
			{
				m_enemyDistance[client] = edict_distance;
				m_nearestEnemy[client] = i;
			}
		}
	}
	
	m_hasEnemiesNear[client] = m_enemiesNearCount[client] > 0;
	m_hasFriendsNear[client] = m_friendsNearCount[client] > 0;
	if (m_hasEnemiesNear[client])
		m_lastEnemyOrigin[client] = GetOrigin(m_nearestEnemy[client]);
}