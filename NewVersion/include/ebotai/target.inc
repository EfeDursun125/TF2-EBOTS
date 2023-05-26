float m_ignoreEnemies[TFMaxPlayers];

public void FindFriendsAndEnemiens(int client)
{
	m_enemyDistance[client] = 999999999999999999999.0;
	m_friendDistance[client] = 999999999999999999999.0;
	m_enemiesNearCount[client] = 0;
	m_friendsNearCount[client] = 0;
	m_hasEnemiesNear[client] = false;
	m_hasFriendsNear[client] = false;
	float medicdist = 999999999999999999999.0;

	if (m_ignoreEnemies[client] > GetGameTime())
		return;
	
	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsValidClient(i))
			continue;

		if (!m_isAlive[i])
			continue;

		if (i == client)
			continue;
		
		if (TF2_IsPlayerInCondition(i, TFCond_HalloweenGhostMode))
			continue;
		
		if (GetEntProp(i, Prop_Data, "m_takedamage") != 2)
			continue;
		
		if (!IsVisible(GetEyePosition(client), GetEyePosition(i)))
			continue;
		
		if (m_team[i] == m_team[client])
		{
			int health = GetClientHealth(i);
			int maxHealth = GetMaxHealth(i);
			
			if (IsMedic(client) || HasCrossbow(client))
			{
				// no medigun no pocket
				if (HasCrossbow(client) && health >= maxHealth)
					continue;

				if (GameRules_GetProp("m_bPlayingMedieval") && health >= maxHealth)
					continue;
				
				if (TF2_IsPlayerInCondition(i, TFCond_Cloaked))
					continue;
		
				if (TF2_IsPlayerInCondition(i, TFCond_Disguised) && health >= maxHealth)
					continue;
		
				if (m_class[i] == TFClass_Engineer && health >= maxHealth)
					continue;
		
				if (IsMedic(i) && health >= maxHealth)
					continue;
		
				if (IsSniper(i) && health >= maxHealth)
					continue;
		
				if (m_class[i] == TFClass_Engineer && IsWeaponSlotActive(i, 5) && health >= maxHealth)
					continue;
		
				m_friendsNearCount[client]++;
				m_friendDistance[client] = GetVectorDistance(GetEyePosition(client), GetEyePosition(i), true);
				float realdist = m_friendDistance[client];
				float pri = 1.0; // lowest = highest priority

				if (m_class[i] == TFClass_Heavy)
					pri = 0.5;
				else if (m_class[i] == TFClass_Soldier)
					pri = 0.6;
				else if (m_class[i] == TFClass_Pyro)
					pri = 0.7;
				else if (m_class[i] == TFClass_DemoMan)
					pri = 0.8;
				else if (m_class[i] == TFClass_Scout)
					pri = 0.9;
				else if (IsMedic(i))
					pri = 1.1;

				float edict_distance = ((realdist * health) * (TF2_GetNumberOfHealers(i) + 1)) * pri;

				if (edict_distance < medicdist)
				{
					medicdist = edict_distance;
					m_nearestFriend[client] = i;
				}
			}
			else if (HasSandwich(client))
			{
				// no medigun no pocket
				if (HasSandwich(client) && health >= maxHealth)
					continue;

				if (GameRules_GetProp("m_bPlayingMedieval") && health >= maxHealth)
					continue;
				
				if (TF2_IsPlayerInCondition(i, TFCond_Cloaked))
					continue;
		
				if (TF2_IsPlayerInCondition(i, TFCond_Disguised) && health >= maxHealth)
					continue;
		
				if (m_class[i] == TFClass_Engineer && IsWeaponSlotActive(i, 2) && health >= maxHealth)
					continue;
		
				if (IsMedic(i) && health >= maxHealth)
					continue;
		
				if (IsSniper(i) && health >= maxHealth)
					continue;
		
				if (m_class[i] == TFClass_Engineer && IsWeaponSlotActive(i, 5) && health >= maxHealth)
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
				m_hasEnemiesNear[client] = true;
				m_lastEnemy[client] = m_nearestEnemy[client];
				m_lastEnemyOrigin[client] = GetOrigin(m_nearestEnemy[client]);
				
				return;
			}

			if (GameRules_GetProp("m_bPlayingMannVsMachine"))
			{
				if (TF2_IsPlayerInCondition(i, TFCond_Ubercharged))
					continue;

				if (TF2_IsPlayerInCondition(i, TFCond_Disguised) && m_class[i] == TFClass_Spy && !IsRevealedSpyByTarget(i, client))
					continue;
			}
			else if (edict_distance > Squaredf(80.0))
			{
				if (!IsAttacking(i) && !TF2_IsPlayerInCondition(client, TFCond_Disguised) && !ClientViews(client, i))
					continue;
				
				if (m_class[i] == TFClass_Spy && !IsRevealedSpyByTarget(i, client))
					continue;
			}

			if (TF2_IsPlayerInCondition(client, TFCond_Zoomed) && edict_distance > Squaredf(256.0))
			{
				if (m_difficulty[client] >= 1 && IsSniper(client))
				{
					m_enemiesNearCount[client] = 1;
					m_enemyDistance[client] = edict_distance;
					m_nearestEnemy[client] = i;
					m_hasEnemiesNear[client] = m_enemiesNearCount[client] > 0;
					m_hasFriendsNear[client] = m_friendsNearCount[client] > 0;
					m_lastEnemy[client] = m_nearestEnemy[client];
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
					m_lastEnemy[client] = m_nearestEnemy[client];
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
					m_lastEnemy[client] = m_nearestEnemy[client];
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
		m_lastEnemySeen[client] = GetGameTime();
	}
}

public void FindEnemyEntities(int client)
{
	if (!m_isSlowThink[client])
		return;

	if (m_ignoreEnemies[client] > GetGameTime())
	{
		m_hasEntitiesNear[client] = false;
		m_nearestEnemy[client] = -1;
		return;
	}

	m_hasEntitiesNear[client] = false;
	int m_hasEntitiesNearCount = 0;
	float nearestDistance = 999999999999999999999.0;
	
	for (int i = MaxClients + 1; i <= GetMaxEntities(); i++)
	{
		if (!IsValidEntity(i))
			continue;

		char class[64];
		GetEntityClassname(i, class, sizeof(class));
		if (!StrEqual(class, "obj_sentrygun", false) && !StrEqual(class, "obj_dispenser", false) && !StrEqual(class, "obj_teleporter", false) && !StrEqual(class, "tf_projectile_pipe_remote", false) && !StrEqual(class, "tf_zombie", false) && !StrEqual(class, "tank_boss", false) && !StrEqual(class, "merasmus", false) && !StrEqual(class, "eyeball_boss", false) && !StrEqual(class, "headless_hatman", false) && !StrEqual(class, "tf_robot_destruction_robot", false))
			continue;

		if (GetTeamNumber(i) == m_team[client])
		{
			if (m_useTeleporter[client])
			{
				char class[64];
				GetEntityClassname(i, class, sizeof(class));
				if (!StrEqual(class, "obj_teleporter", false))
					continue;
				
				if (TF2_GetObjectMode(i) != TFObjectMode_Entrance)
					continue;

				if (!IsVisible(GetEyePosition(client), GetOrigin(i)))
					continue;
				
				if (!HasEntProp(i, Prop_Send, "m_bHasSapper") || GetEntProp(i, Prop_Send, "m_bHasSapper") != 0)
					continue;
				
				if (GetEntProp(i, Prop_Send, "m_iState") == 2 || GetEntProp(i, Prop_Send, "m_iState") == 6 && GetEntProp(i, Prop_Send, "m_iUpgradeLevel") == 3)
					m_teleporterEntity[client] = i;
			}

			continue;
		}
		
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
		if (distance < nearestDistance)
		{
			m_nearestEntity[client] = i;
			nearestDistance = distance;
		}
	}
	
	m_hasEntitiesNear[client] = m_hasEntitiesNearCount > 0;
	if (m_hasEntitiesNear[client])
		m_lastEntitySeen[client] = GetGameTime();
}
