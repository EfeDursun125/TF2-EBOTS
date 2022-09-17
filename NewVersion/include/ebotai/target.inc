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
		
		if (TF2_IsPlayerInCondition(i, TFCond_HalloweenGhostMode))
			continue;
		
		if (GetEntProp(i, Prop_Data, "m_takedamage") != 2)
			continue;
		
		if (!IsVisible(GetEyePosition(client), GetEyePosition(i)))
			continue;
		
		if (GetClientTeam(i) == GetClientTeam(client))
		{
			if (IsMedic(client) || HasCrossbow(client))
			{
				// no medigun no pocket
				if (HasCrossbow(client) && GetClientHealth(i) >= GetMaxHealth(i))
					continue;

				if (GameRules_GetProp("m_bPlayingMedieval") && GetClientHealth(i) >= GetMaxHealth(i))
					continue;
				
				if (TF2_IsPlayerInCondition(i, TFCond_Cloaked))
					continue;
		
				if (TF2_IsPlayerInCondition(i, TFCond_Disguised) && GetClientHealth(i) >= GetMaxHealth(i))
					continue;
		
				if (TF2_GetPlayerClass(i) == TFClass_Engineer && IsWeaponSlotActive(i, 2) && GetClientHealth(i) >= GetMaxHealth(i))
					continue;
		
				if (IsMedic(i) && GetClientHealth(i) >= GetMaxHealth(i))
					continue;
		
				if (IsSniper(i) && GetClientHealth(i) >= GetMaxHealth(i))
					continue;
		
				if (TF2_GetPlayerClass(i) == TFClass_Engineer && IsWeaponSlotActive(i, 5) && GetClientHealth(i) >= GetMaxHealth(i))
					continue;
		
				m_friendsNearCount[client]++;
				m_friendDistance[client] = GetVectorDistance(GetEyePosition(client), GetEyePosition(i), true);
				float realdist = m_friendDistance[client];
				float pri = 1.0; // lowest = highest priority

				if (TF2_GetPlayerClass(i) == TFClass_Heavy)
					pri = 0.5;
				else if (TF2_GetPlayerClass(i) == TFClass_Soldier)
					pri = 0.6;
				else if (TF2_GetPlayerClass(i) == TFClass_Pyro)
					pri = 0.7;
				else if (TF2_GetPlayerClass(i) == TFClass_DemoMan)
					pri = 0.8;
				else if (TF2_GetPlayerClass(i) == TFClass_Scout)
					pri = 0.9;
				else if (IsMedic(i))
					pri = 1.1;

				float edict_distance = ((realdist * GetClientHealth(i)) * (TF2_GetNumberOfHealers(i) + 1)) * pri;

				if (edict_distance < medicdist)
				{
					medicdist = edict_distance;
					m_nearestFriend[client] = i;
				}
			}
			else if (HasSandwich(client))
			{
				// no medigun no pocket
				if (HasSandwich(client) && GetClientHealth(i) >= GetMaxHealth(i))
					continue;

				if (GameRules_GetProp("m_bPlayingMedieval") && GetClientHealth(i) >= GetMaxHealth(i))
					continue;
				
				if (TF2_IsPlayerInCondition(i, TFCond_Cloaked))
					continue;
		
				if (TF2_IsPlayerInCondition(i, TFCond_Disguised) && GetClientHealth(i) >= GetMaxHealth(i))
					continue;
		
				if (TF2_GetPlayerClass(i) == TFClass_Engineer && IsWeaponSlotActive(i, 2) && GetClientHealth(i) >= GetMaxHealth(i))
					continue;
		
				if (IsMedic(i) && GetClientHealth(i) >= GetMaxHealth(i))
					continue;
		
				if (IsSniper(i) && GetClientHealth(i) >= GetMaxHealth(i))
					continue;
		
				if (TF2_GetPlayerClass(i) == TFClass_Engineer && IsWeaponSlotActive(i, 5) && GetClientHealth(i) >= GetMaxHealth(i))
					continue;
		
				m_friendsNearCount[client]++;
				m_friendDistance[client] = GetVectorDistance(GetEyePosition(client), GetEyePosition(i), true);
				float realdist = m_friendDistance[client];

				if (realdist < 500)
				{
					MoveTo(client, GetOrigin(m_nearestFriend[client]));
					EquipWeaponSlot(client, 1);
					m_lookAt[client] = GetEyePosition(m_nearestFriend[client]);
					if (IsWeaponSlotActive(client, 1))
						m_buttons[client] |= IN_ATTACK2;
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
			// avoid fighting if we active uber
			if (IsMedic(client) && TF2_IsPlayerInCondition(client, TFCond_Ubercharged) && IsWeaponSlotActive(client, 1))
			{
				m_enemiesNearCount[client] = 0;
				m_enemyDistance[client] = 0.0;
				m_nearestEnemy[client] = -1;
				m_hasEnemiesNear[client] = false;
				return;
			}
			
			// avoid fighting if we active bonk
			if (TF2_IsPlayerInCondition(client, TFCond_Bonked))
			{
				m_enemiesNearCount[client] = 0;
				m_enemyDistance[client] = 0.0;
				m_nearestEnemy[client] = -1;
				m_hasEnemiesNear[client] = false;
				return;
			}

			float edict_distance = GetVectorDistance(GetOrigin(client), GetOrigin(i), true);
			if (TF2_HasTheFlag(i))
			{
				m_enemiesNearCount[client] = 1;
				m_enemyDistance[client] = edict_distance;
				m_nearestEnemy[client] = i;
				m_hasEnemiesNear[client] = false;

				if (m_hasEnemiesNear[client])
					m_lastEnemyOrigin[client] = GetOrigin(m_nearestEnemy[client]);
				
				return;
			}
			
			if (edict_distance > Squared(80))
			{
				if (!IsAttacking(i) && !TF2_IsPlayerInCondition(client, TFCond_Disguised) && !ClientViews(client, i))
					continue;
				
				if (TF2_GetPlayerClass(i) == TFClass_Spy && !IsRevealedSpyByTarget(i, client))
					continue;
			}

			if (TF2_IsPlayerInCondition(client, TFCond_Zoomed) && edict_distance > Squared(256))
			{
				if (m_difficulty[client] >= 1 && IsSniper(client))
				{
					m_enemiesNearCount[client] = 1;
					m_enemyDistance[client] = edict_distance;
					m_nearestEnemy[client] = i;
					m_hasEnemiesNear[client] = m_enemiesNearCount[client] > 0;
					m_hasFriendsNear[client] = m_friendsNearCount[client] > 0;

					if (m_hasEnemiesNear[client])
						m_lastEnemyOrigin[client] = GetOrigin(m_nearestEnemy[client]);
					
					return;
				}
				else if (m_difficulty[client] >= 2 && !IsMoving(i))
				{
					m_enemiesNearCount[client] = 1;
					m_enemyDistance[client] = edict_distance;
					m_nearestEnemy[client] = i;
					m_hasEnemiesNear[client] = m_enemiesNearCount[client] > 0;
					m_hasFriendsNear[client] = m_friendsNearCount[client] > 0;

					if (m_hasEnemiesNear[client])
						m_lastEnemyOrigin[client] = GetOrigin(m_nearestEnemy[client]);
					
					return;
				}
				else if (m_difficulty[client] >= 3 && !IsClientAimingToMe(client, i))
				{
					m_enemiesNearCount[client] = 1;
					m_enemyDistance[client] = edict_distance;
					m_nearestEnemy[client] = i;
					m_hasEnemiesNear[client] = m_enemiesNearCount[client] > 0;
					m_hasFriendsNear[client] = m_friendsNearCount[client] > 0;

					if (m_hasEnemiesNear[client])
						m_lastEnemyOrigin[client] = GetOrigin(m_nearestEnemy[client]);
					
					return;
				}
			}
			
			m_enemiesNearCount[client]++;
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
	{
		m_lastEnemy[client] = m_nearestEnemy[client];
		m_lastEnemyOrigin[client] = GetOrigin(m_nearestEnemy[client]);
	}
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
		
		char class[64];
		GetEntityClassname(i, class, sizeof(class));
		if (StrContains(class, "obj_", false) == -1 && !StrEqual(class, "tf_zombie", false) && !StrEqual(class, "tank_boss", false) && !StrEqual(class, "merasmus", false) && !StrEqual(class, "eyeball_boss", false) && !StrEqual(class, "headless_hatman", false) && !StrEqual(class, "tf_robot_destruction_robot", false))
			continue;
		
		if (GetEntProp(i, Prop_Data, "m_iHealth") <= 0)
			continue;
		
		if (GetTeamNumber(i) == GetClientTeam(client))
			continue;

		if (HasEntProp(i, Prop_Send, "m_bHasSapper") && GetEntProp(i, Prop_Send, "m_bHasSapper") != 0)
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
