stock void LookAtEnemiens(int client)
{
	if (TF2_IsPlayerInCondition(client, TFCond_Charging))
	{
		m_lookAt[client] = m_pathAhead[client];
		return;
	}

	m_tackEntity[client] = m_nearestEnemy[client];

	float distance = GetVectorDistance(GetEyePosition(client), GetEyePosition(m_nearestEnemy[client]), true);
	if (distance <= Squaredf(128.0))
	{
		m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
		return;
	}

	float BestAimPosition[3];

	if (IsSniper(client) && IsWeaponSlotActive(client, 0))
	{
		int iBone = LookupBone(m_nearestEnemy[client], "bip_head");
		if (iBone != -1)
		{
			float AimPositon[3];
			float Null[3];
			GetBonePosition(m_nearestEnemy[client], iBone, AimPositon, Null);
			AimPositon[2] += 3.0;
			m_lookAt[client] = AimPositon;
		}
		else
			m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
	}
	else if (m_primaryID[client] == 56 || m_primaryID[client] == 1005 || m_primaryID[client] == 1092)
	{
		float EntVec[3];
		GetEntPropVector(m_nearestEnemy[client], Prop_Data, "m_vecVelocity", EntVec);
		float AimPositon[3];
		AimPositon = GetEyePosition(m_nearestEnemy[client]);
		if (GetEntityFlags(m_nearestEnemy[client]) & FL_ONGROUND)
		{
			float AimPositon2[3];
			AimPositon2[2] = AimPositon[2] + (EntVec[2] * 0.4);
			AimPositon2[1] = AimPositon[1] + (EntVec[1] * 0.4);
			AimPositon2[0] = AimPositon[0] + (EntVec[0] * 0.4);
			
			if (IsVisible(GetEyePosition(client), AimPositon2))
				m_lookAt[client] = AimPositon2;
			else
				m_lookAt[client] = AimPositon;
		}
		else
		{
			float AimPositon2[3];
			AimPositon2[2] = AimPositon[2] + (EntVec[2] * 0.5);
			AimPositon2[1] = AimPositon[1] + (EntVec[1] * 0.5);
			AimPositon2[0] = AimPositon[0] + (EntVec[0] * 0.5);
			
			if (IsVisible(GetEyePosition(client), AimPositon2))
				m_lookAt[client] = AimPositon2;
			else
				m_lookAt[client] = AimPositon;
		}
	}
	else if (m_class[client] == TFClass_Soldier && IsWeaponSlotActive(client, 0))
	{
		if (m_difficulty[client] >= 2 && IsVisible(GetEyePosition(client), GetOrigin(m_nearestEnemy[client])))
		{
			float EntVec[3];
			GetEntPropVector(m_nearestEnemy[client], Prop_Data, "m_vecVelocity", EntVec);	

			float dist = SquareRoot(distance);

			m_lookAt[client] = GetOrigin(m_nearestEnemy[client]);
			m_lookAt[client][0] += (EntVec[0] * (dist / 1100));
			m_lookAt[client][1] += (EntVec[1] * (dist / 1100));
			m_lookAt[client][2] += (EntVec[2] * (dist / 1100));
		}
		else
			m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
	}
	else if (m_class[client] == TFClass_DemoMan && !IsWeaponSlotActive(client, 2))
	{
		GetClientEyePosition(m_nearestEnemy[client], BestAimPosition);
		if (TF2_GetPlayerWeaponID(client, 0) == 996)
		{
			BestAimPosition[2] += (SquareRoot(distance) * 0.14285714285) - 5;
			if (IsVisible(GetEyePosition(client), BestAimPosition))
				m_lookAt[client] = BestAimPosition;
			else
				m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
		}
		else
		{
			if (m_enemyDistance[client] <= Squaredf(500.0))
				BestAimPosition[2] -= 15.0;
			else if (m_enemyDistance[client] <= Squaredf(750.0))
				BestAimPosition[2] += 50.0;
			else if (m_enemyDistance[client] <= Squaredf(1000.0))
				BestAimPosition[2] += 100.0;
			else if	(m_enemyDistance[client] <= Squaredf(1250.0))
				BestAimPosition[2] += 150.0;
			else if	(m_enemyDistance[client] <= Squaredf(1500.0))
				BestAimPosition[2] += 200.0;
			else if	(m_enemyDistance[client] > Squaredf(1500.0))
				BestAimPosition[2] += 250.0;
			
			if (IsVisible(GetEyePosition(client), BestAimPosition))
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
			float EntVec[3];
			GetEntPropVector(m_nearestEnemy[client], Prop_Data, "m_vecVelocity", EntVec);	

			float dist = SquareRoot(distance);

			BestAimPosition[0] += (EntVec[0] * (dist / 990));
			BestAimPosition[1] += (EntVec[1] * (dist / 990));
			BestAimPosition[2] += (dist / 13.784048752090222);
			//BestAimPosition[2] += (distance / 990 + 800);

			if (IsVisible(GetEyePosition(client), BestAimPosition))
				m_lookAt[client] = BestAimPosition;
			else
				m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
		}
	}
	else if (m_class[client] == TFClass_Spy && (TF2_GetPlayerWeaponID(client, 0) == 61 || TF2_GetPlayerWeaponID(client, 0) == 1006))
	{
		int iBone = LookupBone(m_nearestEnemy[client], "bip_head");
		if (iBone != -1)
		{
			float AimPositon[3];
			float Null[3];
			GetBonePosition(m_nearestEnemy[client], iBone, AimPositon, Null);
			AimPositon[2] += 3.0;
			m_lookAt[client] = AimPositon;
		}
		else
			m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
	}
	else
	{
		if (IsWeaponSlotActive(client, 2))
			m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
		else
			m_lookAt[client] = GetCenter(m_nearestEnemy[client]);
	}
}

float m_searchTime[TFMaxPlayers];
stock void LookAround(int client)
{
	m_tackEntity[client] = -1;
	if (m_pauseTime[client] >= GetGameTime())
		return;

	float ClientEye[3];
	GetClientEyePosition(client, ClientEye);

	if (m_hasFriendsNear[client] && m_class[m_nearestFriend[client]] != TFClass_Medic && IsAttacking(m_nearestFriend[client]) && !IsWeaponSlotActive(client, 2) && !IsWeaponSlotActive(m_nearestFriend[client], 2))
	{
		int friend = m_nearestFriend[client];
		if (IsEBot(friend) || m_isAFK[friend])
			m_lookAt[client] = m_lookAt[friend];
		else
			GetAimOrigin(friend, m_lookAt[client]);
		
		m_pauseTime[client] = GetGameTime() + GetRandomFloat(1.5, 2.5);
	}
	else if (m_isSlowThink[client] && m_ignoreEnemies[client] < GetGameTime() && m_class[client] != TFClass_Medic && ChanceOf(m_eBotSenseChance[client])) // no any friends near, who's footsteps is this or fire sounds?
	{
		for (int search = 1; search <= MaxClients; search++)
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
			
			float PlayerArea[3];
			if (TF2_HasTheFlag(search))
				PlayerArea = GetOrigin(search);
			else if (m_hasWaypoints)
			{
				int index = FindNearestWaypoint(GetEyePosition(search), 999999.0, client);
				if (index != -1)
				{
					PlayerArea[0] = m_paths[index].origin[0] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius) + ((PlayerArea[0] - ClientEye[0]) * 1024.0);
					PlayerArea[1] = m_paths[index].origin[1] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius) + ((PlayerArea[1] - ClientEye[1]) * 1024.0);
					PlayerArea[2] = m_eyeOrigin[client][2];
				}
			}
			else
			{
				CNavArea area = NavMesh_GetNearestArea(GetEyePosition(search), true, 768.0, false, false);
				if (area != INVALID_NAV_AREA)
				{
					area.GetRandomPoint(PlayerArea);
					float fw[3];
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

	float BestLookPos[3];
	float SelectRandom[3];
	
	if (m_searchTime[client] < GetGameTime())
	{
		GetClientEyePosition(client, BestLookPos);
		
		BestLookPos[0] += GetRandomFloat(-1024.0, 1024.0);
		BestLookPos[1] += GetRandomFloat(-1024.0, 1024.0);
		BestLookPos[2] += GetRandomFloat(-256.0, 256.0);
		
		if (m_hasWaypoints)
		{
			int index = FindNearestWaypoint(BestLookPos, 999999.0, client);
			if (index != -1)
			{
				SelectRandom[0] = m_paths[index].origin[0] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius);
				SelectRandom[1] = m_paths[index].origin[1] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius);
				SelectRandom[2] = m_paths[index].origin[2];
			}
		}
		else
		{
			CNavArea area = NavMesh_GetNearestArea(BestLookPos, true, 512.0, false, false);
			if (area != INVALID_NAV_AREA)
				area.GetRandomPoint(SelectRandom);
		}
		
		m_searchTime[client] = GetGameTime() + 0.2;
	}
	
	if (IsVisible(GetEyePosition(client), SelectRandom) && GetVectorDistance(GetOrigin(client), SelectRandom, true) > Squared(512))
	{
		if (m_pauseTime[client] < GetGameTime())
		{
			m_lookAt[client][0] = SelectRandom[0] + ((SelectRandom[0] - ClientEye[0]) * 256.0);
			m_lookAt[client][1] = SelectRandom[1] + ((SelectRandom[1] - ClientEye[1]) * 256.0);
			m_lookAt[client][2] = SelectRandom[2] + GetHeight(client);
			m_pauseTime[client] = GetGameTime() + GetRandomFloat(1.5, 2.25);
		}
	}
	else if (m_isSlowThink[client] && IsMoving(client))
	{
		if (m_hasWaypoints)
		{
			int index = FindNearestWaypoint(m_pathAhead[client], 999999.0, client);
			if (index != -1)
			{
				m_lookAt[client][0] = m_paths[index].origin[0] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius) + ((m_pathAhead[client][0] - ClientEye[0]) * 1024.0);
				m_lookAt[client][1] = m_paths[index].origin[1] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius) + ((m_pathAhead[client][1] - ClientEye[1]) * 1024.0);
				m_lookAt[client][2] = m_paths[index].origin[2] + GetHeight(client);
			}
		}
		else
		{
			CNavArea area = NavMesh_GetNearestArea(m_pathAhead[client], true, 512.0, false, false);
			if (area != INVALID_NAV_AREA)
			{
				float fw[3];
				EyeVectors(client, fw);
				area.GetRandomPoint(m_lookAt[client]);
				m_lookAt[client][0] += fw[0] * 256.0;
				m_lookAt[client][1] += fw[0] * 256.0;
				m_lookAt[client][2] += GetHeight(client);
			}
		}
	}

	if (m_cosError[client] > 0.98)
		m_itAimStart[client] = GetGameTime();
}