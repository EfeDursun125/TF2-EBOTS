stock void LookAtEnemiens(int client)
{
	if (TF2_IsPlayerInCondition(client, TFCond_Charging))
	{
		m_lookAt[client] = m_pathAhead[client];
		return;
	}

	float BestAimPosition[3];
	if (TF2_GetPlayerClass(client) == TFClass_Soldier && IsWeaponSlotActive(client, 0))
	{
		if (IsVisible(GetEyePosition(client), GetOrigin(m_nearestEnemy[client])))
		{
			if (TF2_GetPlayerWeaponID(client, 0) == 127)
			{
				float EntVec[3];
				GetEntPropVector(m_nearestEnemy[client], Prop_Data, "m_vecVelocity", EntVec);
				m_lookAt[client] = GetOrigin(m_nearestEnemy[client]);
				m_lookAt[client][2] += EntVec[2];
				m_lookAt[client][1] += EntVec[1];
				m_lookAt[client][0] += EntVec[0];
			}
			else
			{
				float EntVec[3];
				GetEntPropVector(m_nearestEnemy[client], Prop_Data, "m_vecVelocity", EntVec);	
				m_lookAt[client] = GetOrigin(m_nearestEnemy[client]);
				m_lookAt[client][2] += (EntVec[2] / 2);
				m_lookAt[client][1] += (EntVec[1] / 2);
				m_lookAt[client][0] += (EntVec[0] / 2);
			}
		}
		else
			m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
	}
	else if (TF2_GetPlayerClass(client) == TFClass_DemoMan && !IsWeaponSlotActive(client, 2))
	{
		GetClientEyePosition(m_nearestEnemy[client], BestAimPosition);
		if (TF2_GetPlayerWeaponID(client, 0) == 996)
		{
			float distance = GetVectorDistance(GetEyePosition(client), GetEyePosition(m_nearestEnemy[client]));
			BestAimPosition[2] += (distance / 7) - 5;
			if (IsVisible(GetEyePosition(client), BestAimPosition))
				m_lookAt[client] = BestAimPosition;
			else
				m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
		}
		else
		{
			float distance = GetVectorDistance(GetEyePosition(client), GetEyePosition(m_nearestEnemy[client]), true);
			if (distance <= Squared(512))
				BestAimPosition[2] -= 15.0;
			else if (distance <= Squared(768))
				BestAimPosition[2] += 50.0;
			else if (distance <= Squared(1024))
				BestAimPosition[2] += 100.0;
			else if (distance <= Squared(1280))
				BestAimPosition[2] += 150.0;
			else if (distance <= Squared(1536))
				BestAimPosition[2] += 200.0;
			else if (distance > Squared(1536))
				BestAimPosition[2] += 250.0;
			
			if (IsVisible(GetEyePosition(client), BestAimPosition))
				m_lookAt[client] = BestAimPosition;
			else
				m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
		}
	}
	else if (TF2_GetPlayerClass(client) == TFClass_Medic && IsWeaponSlotActive(client, 0))
	{
		int primaryid = TF2_GetPlayerWeaponID(client, 0);
		if (primaryid == 305 || primaryid == 1079)
		{
			if (IsVisible(GetEyePosition(client), GetEyePosition(m_nearestEnemy[client])))
				m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
			else
				m_lookAt[client] = GetOrigin(m_nearestEnemy[client]);
		}
		else
		{
			GetClientEyePosition(m_nearestEnemy[client], BestAimPosition);
			float distance = GetVectorDistance(GetEyePosition(client), GetEyePosition(m_nearestEnemy[client]));
			BestAimPosition[2] += (distance / 17) - 2;
			if (IsVisible(GetEyePosition(client), BestAimPosition))
				m_lookAt[client] = BestAimPosition;
			else
				m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
		}
	}
	else if (TF2_GetPlayerClass(client) == TFClass_Sniper && IsWeaponSlotActive(client, 0))
	{
		int primaryid = TF2_GetPlayerWeaponID(client, 0);
		if (primaryid == 56 || primaryid == 1005 || primaryid == 1092)
		{
			float EntVec[3];
			GetEntPropVector(m_nearestEnemy[client], Prop_Data, "m_vecVelocity", EntVec);
			float AimPositon[3];
			AimPositon = GetEyePosition(m_nearestEnemy[client]);
			if (GetEntityFlags(m_nearestEnemy[client]) & FL_ONGROUND)
			{
				float AimPositon2[3];
				AimPositon2[2] = AimPositon[2] + (EntVec[2] / 2.5);
				AimPositon2[1] = AimPositon[1] + (EntVec[1] / 2.5);
				AimPositon2[0] = AimPositon[0] + (EntVec[0] / 2.5);
				
				if (IsVisible(GetEyePosition(client), AimPositon2))
					m_lookAt[client] = AimPositon2;
				else
					m_lookAt[client] = AimPositon;
			}
			else
			{
				float AimPositon2[3];
				AimPositon2[2] = AimPositon[2] + (EntVec[2] / 2);
				AimPositon2[1] = AimPositon[1] + (EntVec[1] / 2);
				AimPositon2[0] = AimPositon[0] + (EntVec[0] / 2);
				
				if (IsVisible(GetEyePosition(client), AimPositon2))
					m_lookAt[client] = AimPositon2;
				else
					m_lookAt[client] = AimPositon;
			}
		}
		else
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
	}
	else if (TF2_GetPlayerClass(client) == TFClass_Spy && (TF2_GetPlayerWeaponID(client, 0) == 61 || TF2_GetPlayerWeaponID(client, 0) == 1006))
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

float m_searchTime[MAXPLAYERS + 1];
stock void LookAround(int client)
{
	if (m_pauseTime[client] >= GetGameTime())
		return;
	
	if (m_hasFriendsNear[client] && IsAttacking(m_nearestFriend[client]))
	{
		int friend = m_nearestFriend[client];
		if (IsFakeClient(friend))
			m_lookAt[client] = m_lookAt[friend];
		else
			m_lookAt[client] = GetEyePosition(friend);
		m_pauseTime[client] = GetGameTime() + GetRandomFloat(1.5, 2.5);
	}
	else if (m_isSlowThink[client] && ChanceOf(m_eBotSenseChance[client])) // no any friends near, who's footsteps is this or fire sounds?
	{
		for (int search = 1; search <= MaxClients; search++)
		{
			if (!IsValidClient(search))
				continue;
			
			if (!IsPlayerAlive(search))
				continue;
			
			if (GetClientTeam(client) == GetClientTeam(search))
				continue;
			
			if (search == client)
				continue;
			
			if (!IsMoving(search) && !IsAttacking(search))
				continue;
			
			if (GetVectorDistance(GetOrigin(client), GetOrigin(search), true) > Squared(768))
				continue;
			
			float PlayerArea[3];
			if (TF2_HasTheFlag(search))
				PlayerArea = GetOrigin(search);
			else if (m_hasWaypoints)
			{
				int index = FindNearestWaypoint(GetEyePosition(search), 999999.0, client);
				if (index != -1)
				{
					PlayerArea[0] = float(m_paths[index].origin[0] + GetRandomInt(-m_paths[index].radius, m_paths[index].radius));
					PlayerArea[1] = float(m_paths[index].origin[1] + GetRandomInt(-m_paths[index].radius, m_paths[index].radius));
					PlayerArea[2] = float(m_paths[index].origin[2]);
				}
			}
			else
			{
				CNavArea area = NavMesh_GetNearestArea(GetEyePosition(search), true, 768.0, false, false);
				if (area != INVALID_NAV_AREA)
					area.GetRandomPoint(PlayerArea);
			}
			
			if (TF2_GetPlayerClass(client) == TFClass_Heavy && IsWeaponSlotActive(client, 0))
			{
				if (IsValidEntity(GetPlayerWeaponSlot(client, TFWeaponSlot_Primary)))
				{
					if(m_lowHealth[client])
						m_attack2Timer[client] = GetGameTime() + 2.0;
					else
						m_attack2Timer[client] = GetGameTime() + 4.0;
				}
				else
					m_attack2Timer[client] = 0.0;
			}
			
			m_lookAt[client][0] = PlayerArea[0];
			m_lookAt[client][1] = PlayerArea[1];
			m_lookAt[client][2] = PlayerArea[2] + GetHeight(client);
			m_pauseTime[client] = GetGameTime() + GetRandomFloat(1.5, 2.5);
			
			return;
		}
	}

	float BestLookPos[3];
	float SelectRandom[3];
	
	if (m_searchTime[client] < GetGameTime())
	{
		GetClientEyePosition(client, BestLookPos);
		
		BestLookPos[0] += GetRandomFloat(-768.0, 768.0);
		BestLookPos[1] += GetRandomFloat(-768.0, 768.0);
		BestLookPos[2] += GetRandomFloat(-256.0, 256.0);
		
		if (m_hasWaypoints)
		{
			int index = FindNearestWaypoint(BestLookPos, 999999.0, client);
			if (index != -1)
			{
				SelectRandom[0] = float(m_paths[index].origin[0] + GetRandomInt(-m_paths[index].radius, m_paths[index].radius));
				SelectRandom[1] = float(m_paths[index].origin[1] + GetRandomInt(-m_paths[index].radius, m_paths[index].radius));
				SelectRandom[2] = float(m_paths[index].origin[2]);
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
	
	if (IsVisible(GetEyePosition(client), SelectRandom) && GetVectorDistance(GetOrigin(client), SelectRandom, true) > Squared(256))
	{
		if (m_pauseTime[client] < GetGameTime())
		{
			m_lookAt[client][0] = SelectRandom[0];
			m_lookAt[client][1] = SelectRandom[1];
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
				m_lookAt[client][0] = float(m_paths[index].origin[0] + GetRandomInt(-m_paths[index].radius, m_paths[index].radius));
				m_lookAt[client][1] = float(m_paths[index].origin[1] + GetRandomInt(-m_paths[index].radius, m_paths[index].radius));
				m_lookAt[client][2] = float(m_paths[index].origin[2]) + GetHeight(client);
			}
		}
		else
		{
			CNavArea area = NavMesh_GetNearestArea(m_pathAhead[client], true, 512.0, false, false);
			if (area != INVALID_NAV_AREA)
			{
				area.GetRandomPoint(m_lookAt[client]);
				m_lookAt[client][2] += GetHeight(client);
			}
		}
	}
}