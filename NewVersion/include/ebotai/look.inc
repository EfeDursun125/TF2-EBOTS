stock void LookAtEnemiens(const int client)
{
	if (TF2_IsPlayerInCondition(client, TFCond_Charging))
	{
		m_lookAt[client] = m_pathAhead[client];
		return;
	}

	static float BestAimPosition[3];
	static float EntVec[3];
	static float delta;

	if (IsSniper(client))
	{
		if (TF2_IsPlayerInCondition(client, TFCond_Zoomed))
			m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
		else
			m_lookAt[client] = GetCenter(m_nearestEnemy[client]);
	}
	else if (m_primaryID[client] == 56 || m_primaryID[client] == 1005 || m_primaryID[client] == 1092)
	{
		if (m_difficulty[client] >= 2)
		{
			GetEntPropVector(m_nearestEnemy[client], Prop_Data, "m_vecVelocity", EntVec);	
			delta = GetVectorDistance(GetEyePosition(client), GetEyePosition(m_nearestEnemy[client])) / 1875.0;

			BestAimPosition = GetEyePosition(m_nearestEnemy[client]);
			BestAimPosition[0] += (EntVec[0] * delta);
			BestAimPosition[1] += (EntVec[1] * delta);
			BestAimPosition[2] += (EntVec[2] * delta);

			if (IsVisible(GetEyePosition(client), BestAimPosition))
				m_lookAt[client] = BestAimPosition;
			else
				m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
		}
		else
			m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
	}
	else if (m_class[client] == TFClass_Soldier && IsWeaponSlotActive(client, 0))
	{
		if (m_difficulty[client] < 2)
			m_lookAt[client] = GetOrigin(m_nearestEnemy[client]);
		else if (IsVisible(GetEyePosition(client), GetOrigin(m_nearestEnemy[client])))
		{
			GetEntPropVector(m_nearestEnemy[client], Prop_Data, "m_vecVelocity", EntVec);
			delta = GetVectorDistance(GetEyePosition(client), GetOrigin(m_nearestEnemy[client])) / 1100.0;

			BestAimPosition = GetOrigin(m_nearestEnemy[client]);
			BestAimPosition[0] += (EntVec[0] * delta);
			BestAimPosition[1] += (EntVec[1] * delta);
			BestAimPosition[2] += (EntVec[2] * delta);

			if (IsVisible(GetEyePosition(client), BestAimPosition))
				m_lookAt[client] = BestAimPosition;
			else
				m_lookAt[client] = GetOrigin(m_nearestEnemy[client]);
		}
		else
			m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
	}
	else if (m_class[client] == TFClass_DemoMan && !IsWeaponSlotActive(client, 2))
	{
		GetClientEyePosition(m_nearestEnemy[client], BestAimPosition);
		if (TF2_GetPlayerWeaponID(client, 0) == 996)
		{
			BestAimPosition[2] += (GetVectorDistance(GetEyePosition(client), BestAimPosition) * 0.14285714285) - 5.0;
			if (IsVisible(GetEyePosition(client), BestAimPosition))
				m_lookAt[client] = BestAimPosition;
			else
				m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
		}
		else
		{
			float myOrigin[3];
			myOrigin = GetEyePosition(client);

			float enemyOrigin[3];
			enemyOrigin = GetEyePosition(m_nearestEnemy[client]);

			GetEntPropVector(m_nearestEnemy[client], Prop_Data, "m_vecVelocity", EntVec);
			delta = GetVectorDistance(myOrigin, enemyOrigin) / 1216.0;
			enemyOrigin[0] += (EntVec[0] * delta);
			enemyOrigin[1] += (EntVec[1] * delta);
			enemyOrigin[2] += (EntVec[2] * delta);

			BestAimPosition = enemyOrigin;
			float timeToReach = SquareRoot(Squaredf(enemyOrigin[0] - myOrigin[0]) + Squaredf(enemyOrigin[1] - myOrigin[1]) + Squaredf(enemyOrigin[2] - myOrigin[2])) / 1216.0;
			BestAimPosition[0] += (enemyOrigin[0] - myOrigin[0]) / timeToReach;
			BestAimPosition[1] += (enemyOrigin[1] - myOrigin[1]) / timeToReach;
			BestAimPosition[2] += ((enemyOrigin[2] + myOrigin[2]) * Squaredf(timeToReach)) / timeToReach;

			if (IsVisible(myOrigin, BestAimPosition))
				m_lookAt[client] = BestAimPosition;
			else
				m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
		}
	}
	else if ((IsMedic(client) || HasCrossbow(client)) && IsWeaponSlotActive(client, 0))
	{
		if (m_primaryID[client] == 305 || m_primaryID[client] == 1079)
		{
			if (IsVisible(GetEyePosition(client), GetEyePosition(m_nearestEnemy[client])))
				m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
			else
				m_lookAt[client] = GetOrigin(m_nearestEnemy[client]);
		}
		else
		{
			GetClientEyePosition(m_nearestEnemy[client], BestAimPosition);
			GetEntPropVector(m_nearestEnemy[client], Prop_Data, "m_vecVelocity", EntVec);
			delta = GetVectorDistance(GetEyePosition(client), BestAimPosition) / 990.0;

			BestAimPosition[0] += (EntVec[0] * delta);
			BestAimPosition[1] += (EntVec[1] * delta);

			if (IsVisible(GetEyePosition(client), BestAimPosition))
				m_lookAt[client] = BestAimPosition;
			else
				m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
		}
	}
	else if (m_class[client] == TFClass_Spy && (TF2_GetPlayerWeaponID(client, 0) == 61 || TF2_GetPlayerWeaponID(client, 0) == 1006))
		m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
	else
	{
		if (IsWeaponSlotActive(client, 2))
			m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
		else
			m_lookAt[client] = GetCenter(m_nearestEnemy[client]);
	}
}

float m_searchTime[TFMaxPlayers];
stock void LookAround(const int client)
{
	if (m_pauseTime[client] > GetGameTime())
		return;
	
	static float cdamage;
	static float damage;
	static float fw[3];
	static float ClientEye[3];
	GetClientEyePosition(client, ClientEye);

	static int search;
	static int index;
	static CNavArea area;

	if (m_hasFriendsNear[client] && m_class[m_nearestFriend[client]] != TFClass_Medic && IsAttacking(m_nearestFriend[client]) && !IsWeaponSlotActive(client, 2) && !IsWeaponSlotActive(m_nearestFriend[client], 2))
	{
		index = m_nearestFriend[client];
		if (IsEBot(index) || m_isAFK[index])
			m_lookAt[client] = m_lookAt[index];
		else
			GetAimOrigin(index, m_lookAt[client]);
		
		m_pauseTime[client] = GetGameTime() + GetRandomFloat(1.5, 2.5);
	}
	else if (m_isSlowThink[client] && m_ignoreEnemies[client] < GetGameTime() && m_class[client] != TFClass_Medic && ChanceOf(m_eBotSenseChance[client])) // no any friends near, who's footsteps is this or fire sounds?
	{
		static float PlayerArea[3];
		for (search = 1; search <= MaxClients; search++)
		{
			if (!IsValidClient(search))
				continue;
			
			if (!m_isAlive[search])
				continue;

			if (m_team[client] == m_team[search])
				continue;
			
			if (TF2_IsPlayerInCondition(search, TFCond_HalloweenGhostMode))
				continue;
			
			if (!IsAttacking(search) && !IsMoving(search))
				continue;
			
			if (IsWeaponSlotActive(search, 2))
				continue;
			
			if (GetVectorDistance(GetOrigin(client), GetOrigin(search), true) > Squaredf(768.0))
				continue;
			
			if (TF2_HasTheFlag(search))
				PlayerArea = GetOrigin(search);
			else if (m_hasWaypoints)
			{
				index = FindNearestWaypoint(GetEyePosition(search), 9999999999.0, client);
				if (index != -1)
				{
					PlayerArea[0] = m_paths[index].origin[0] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius) + ((PlayerArea[0] - ClientEye[0]) * 1024.0);
					PlayerArea[1] = m_paths[index].origin[1] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius) + ((PlayerArea[1] - ClientEye[1]) * 1024.0);
					PlayerArea[2] = m_eyeOrigin[client][2];
				}
			}
			else
			{
				area = NavMesh.GetNearestNavArea(GetEyePosition(search), true, 9999999999.0, false, false, m_team[client]);
				if (area != NULL_AREA)
				{
					GetRandomPoint(area, PlayerArea);
					EyeVectors(client, fw);
					PlayerArea[0] += (fw[0] * 256.0);
					PlayerArea[1] += (fw[0] * 256.0);
					PlayerArea[2] = m_eyeOrigin[client][2];
				}
			}

			if (m_difficulty[client] > 0 && m_class[client] == TFClass_Heavy && IsWeaponSlotActive(client, 0))
			{
				if (IsValidEntity(GetPlayerWeaponSlot(client, TFWeaponSlot_Primary)))
				{
					if (IsMoving(client))
						m_buttons[client] |= IN_JUMP;

					if (m_lowHealth[client] || m_lowAmmo[client] || m_difficulty[client] == 1)
						m_attack2Timer[client] = GetGameTime() + 2.0;
					else
						m_attack2Timer[client] = GetGameTime() + GetRandomFloat(1.0, m_difficulty[client] + 2.0);
				}
				else
					m_attack2Timer[client] = 0.0;
			}
			
			m_lookAt[client][0] = PlayerArea[0];
			m_lookAt[client][1] = PlayerArea[1];
			m_lookAt[client][2] = PlayerArea[2];
			m_pauseTime[client] = GetGameTime() + GetRandomFloat(1.5, 2.5);
			
			return;
		}
	}

	static float BestLookPos[3];
	static float SelectRandom[3];

	if (m_pauseTime[client] > GetGameTime())
		return;

	if (m_searchTime[client] > GetGameTime())
		return;

	GetClientEyePosition(client, BestLookPos);

	BestLookPos[0] += GetRandomFloat(-1024.0, 1024.0);
	BestLookPos[1] += GetRandomFloat(-1024.0, 1024.0);
	BestLookPos[2] += GetRandomFloat(-256.0, 256.0);

	if (m_hasWaypoints)
	{
		index = FindNearestWaypoint(BestLookPos, 9999999999.0, client);
		if (index != -1)
		{
			damage = 0.0;
			for (search = 0; search < MaxPathIndex; search++)
			{
				if (m_paths[index].pathIndex[search] == -1)
                	continue;
				
				cdamage = GetDamage(client, m_paths[index].pathIndex[search]);
				if (cdamage > damage)
				{
					damage = cdamage;
					index = m_paths[index].pathIndex[search];
				}
			}

			SelectRandom[0] = m_paths[index].origin[0] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius);
			SelectRandom[1] = m_paths[index].origin[1] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius);
			SelectRandom[2] = m_paths[index].origin[2];
		}
	}
	else
	{
		area = NavMesh.GetNearestNavArea(BestLookPos, true, 9999999999.0, false, false, m_team[client]);
		if (area != NULL_AREA)
			GetRandomPoint(area, SelectRandom);
	}

	if (GetVectorDistance(GetOrigin(client), SelectRandom, true) > Squaredf(512.0) && IsVisible(GetEyePosition(client), SelectRandom))
	{
		m_lookAt[client][0] = SelectRandom[0] + ((SelectRandom[0] - ClientEye[0]) * 256.0);
		m_lookAt[client][1] = SelectRandom[1] + ((SelectRandom[1] - ClientEye[1]) * 256.0);
		m_lookAt[client][2] = SelectRandom[2] + GetHeight(client);
		m_pauseTime[client] = GetGameTime() + GetRandomFloat(0.75, 1.75);
	}
	else if (m_isSlowThink[client] && IsMoving(client))
	{
		if (m_hasWaypoints)
		{
			index = FindNearestWaypoint(m_pathAhead[client], 9999999999.0, client);
			if (index != -1)
			{
				damage = 0.0;
				for (search = 0; search < MaxPathIndex; search++)
				{
					if (m_paths[index].pathIndex[search] == -1)
            	    	continue;

					cdamage = GetDamage(client, m_paths[index].pathIndex[search]);
					if (cdamage > damage)
					{
						damage = cdamage;
						index = m_paths[index].pathIndex[search];
					}
				}

				m_lookAt[client][0] = m_paths[index].origin[0] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius) + ((m_pathAhead[client][0] - ClientEye[0]) * 1024.0);
				m_lookAt[client][1] = m_paths[index].origin[1] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius) + ((m_pathAhead[client][1] - ClientEye[1]) * 1024.0);
				m_lookAt[client][2] = m_paths[index].origin[2] + GetHeight(client);
				m_pauseTime[client] = GetGameTime() + GetRandomFloat(0.75, 1.75);
			}
		}
		else
		{
			area = NavMesh.GetNearestNavArea(m_pathAhead[client], true, 9999999999.0, false, false, m_team[client]);
			if (area != NULL_AREA)
			{
				EyeVectors(client, fw);
				GetRandomPoint(area, m_lookAt[client]);
				m_lookAt[client][0] += fw[0] * 256.0;
				m_lookAt[client][1] += fw[0] * 256.0;
				m_lookAt[client][2] += GetHeight(client);
				m_pauseTime[client] = GetGameTime() + GetRandomFloat(1.5, 2.25);
			}
		}
	}

	m_searchTime[client] = GetGameTime() + 0.2;
}
