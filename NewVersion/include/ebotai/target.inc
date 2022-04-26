public void FindFriendsAndEnemiens(int client)
{
	m_enemyDistance[client] = 99999999.0;
	m_friendDistance[client] = 99999999.0;
	m_enemiesNearCount[client] = 0;
	m_friendsNearCount[client] = 0;
	m_hasEnemiesNear[client] = false;
	m_hasFriendsNear[client] = false;
	float medicdist = 99999999.0;
	
	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsValidClient(i))
			continue;
		
		if (!IsPlayerAlive(i))
			continue;
		
		if (i == client)
			continue;
		
		if (GetEntProp(i, Prop_Data, "m_takedamage") != 2)
			continue;
		
		if (!IsVisible(GetEyePosition(client), GetEyePosition(i)))
			continue;
		
		if (GetClientTeam(i) == GetClientTeam(client))
		{
			if (TF2_GetPlayerClass(client) == TFClass_Medic)
			{
				if (TF2_IsPlayerInCondition(i, TFCond_Cloaked))
					continue;
		
				if (TF2_IsPlayerInCondition(i, TFCond_Disguised) && GetClientHealth(i) >= GetMaxHealth(i))
					continue;
		
				if (TF2_GetPlayerClass(i) == TFClass_Engineer && IsWeaponSlotActive(i, 2) && GetClientHealth(i) >= GetMaxHealth(i))
					continue;
		
				if (TF2_GetPlayerClass(i) == TFClass_Medic && GetClientHealth(i) >= GetMaxHealth(i))
					continue;
		
				if (TF2_GetPlayerClass(i) == TFClass_Sniper && GetClientHealth(i) >= GetMaxHealth(i))
					continue;
		
				if (TF2_GetPlayerClass(i) == TFClass_Engineer && IsWeaponSlotActive(i, 5) && GetClientHealth(i) >= GetMaxHealth(i))
					continue;
		
				m_friendsNearCount[client]++;
				m_friendDistance[client] = GetVectorDistance(GetEyePosition(client), GetEyePosition(i), true);
				float realdist = m_friendDistance[client];
				float edict_distance = realdist * GetClientHealth(i);
				if (edict_distance < medicdist)
				{
					medicdist = edict_distance;
					m_nearestFriend[client] = i;
				}
			}
			else
			{
				m_friendsNearCount[client]++;
				float edict_distance = GetVectorDistance(GetOrigin(client), GetOrigin(i), true);
				if (edict_distance < m_friendDistance[client])
				{
					m_friendDistance[client] = edict_distance;
					m_nearestFriend[client] = i;
				}
			}
		}
		else
		{
			if (!ClientViews(client, i) && !IsAttacking(i) && TF2_GetPlayerClass(client) != TFClass_Medic && !TF2_IsPlayerInCondition(client, TFCond_Zoomed) && !TF2_IsPlayerInCondition(client, TFCond_Disguised) && !TF2_IsPlayerInCondition(client, TFCond_Taunting))
				continue;

			if (TF2_GetPlayerClass(i) == TFClass_Spy && !IsRevealedSpyByTarget(i, client))
				continue;
			
			m_enemiesNearCount[client]++;
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

public void FindEnemyEntities(int client)
{
	if (!m_isSlowThink[client])
		return;
	
	m_hasEntitiesNear[client] = false;
	int m_hasEntitiesNearCount = 0;
	float nearestDistance = -1.0;
	for (int i = 1; i <= GetEntityCount(); i++)
	{
		if (!IsValidEntity(i))
			continue;
		
		if (i <= MaxClients)
			continue;

		if (!HasEntProp(i, Prop_Data, "m_iHealth"))
			continue;
		
		if (GetEntProp(i, Prop_Data, "m_iHealth") <= 0)
			continue;
		
		if (!HasEntProp(i, Prop_Data, "m_iTeamNum"))
			continue;
		
		if (GetTeamNumber(i) == GetClientTeam(client))
			continue;

		if (HasEntProp(i, Prop_Send, "m_bHasSapper") && GetEntProp(i, Prop_Send, "m_bHasSapper") != 0)
			continue;
		
		if (!HasEntProp(i, Prop_Data, "m_takedamage"))
			continue;
		
		char class[64];
		GetEntityClassname(i, class, sizeof(class));
		if (StrContains(class, "obj_", false) == -1 && !StrEqual(class, "tf_zombie", false) && !StrEqual(class, "tank_boss", false) && !StrEqual(class, "merasmus", false) && !StrEqual(class, "eyeball_boss", false) && !StrEqual(class, "headless_hatman", false) && !StrEqual(class, "tf_robot_destruction_robot", false))
			continue;
		
		float eyeposition[3];
		float center[3];
		eyeposition = GetEyePosition(client);
		center = GetCenter(i);
		if (!IsVisible(eyeposition, center))
			continue;

		m_hasEntitiesNearCount++;
		float distance = GetFastDistance(eyeposition, center);
		if (distance < nearestDistance || nearestDistance == -1.0)
		{
			m_nearestEntity[client] = i;
			nearestDistance = distance;
		}
	}
	
	m_hasEntitiesNear[client] = m_hasEntitiesNearCount > 0;
}