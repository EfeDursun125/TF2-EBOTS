stock bool DefendRequirements(const int client)
{
	if (m_spawnTime[client] + 2.0 > GetGameTime())
		return false;

	return true;
}

stock void DefendStart(const int client)
{

}

stock void DefendUpdate(const int client)
{
	if (m_hasEnemiesNear[client])
	{
		if (SetProcess(client, PRO_ATTACK, 180.0, "", true))
			return;
		else if (!IsMedic(client) && TF2_IsPlayerInCondition(client, TFCond_Ubercharged))
		{
			int enemy = FindNearestEnemy(client);
			if (IsValidClient(enemy) && IsPlayerAlive(enemy) && m_lastFailedEntity[client] != enemy)
			{
				m_goalIndex[client] = -1;
				m_goalEntity[client] = enemy;
				m_goalPosition[client] = GetOrigin(enemy);
				if (SetProcess(client, PRO_HUNTENEMY, 15.0, "", true))
					return;
			}
		}
	}

	if (m_goalIndex[client] != -1)
	{
		if (m_paths[m_goalIndex[client]].activeArea != 0 && !(m_paths[m_goalIndex[client]].activeArea & currentActiveArea))
		{
			FinishCurrentProcess(client, true);
			return;
		}

		if (m_paths[m_goalIndex[client]].flags & WAYPOINT_DEMOMANCAMP)
		{
			if (m_stickyCount[client] < 4)
				m_lookAt[client] = m_paths[m_goalIndex[client]].campStart;
			else if (m_stickyCount[client] < 8)
				m_lookAt[client] = m_paths[m_goalIndex[client]].campEnd;

			if (m_stickyCount[client] < 8)
			{
				if (IsWeaponSlotActive(client, 1))
				{
					if (m_attackTimer[client] + 0.125 < GetGameTime())
						m_attackTimer[client] = GetGameTime() + 0.125;
				}
				else if (!m_hasEnemiesNear[client])
					EquipWeaponSlot(client, 1);

				return;
			}
			else if (!m_hasEnemiesNear[client])
				EquipWeaponSlot(client, 0);

			FindFriendsAndEnemiens(client);
			CheckHealth(client);
			CheckAmmo(client);
			return;
		}

		if (m_paths[m_goalIndex[client]].flags & WAYPOINT_DEFEND || m_paths[m_goalIndex[client]].flags & WAYPOINT_SNIPER)
		{
			if (m_pauseTime[client] < GetGameTime())
			{
				if (crandomint(1, 2) == 1 && IsVisible(GetEyePosition(client), m_paths[m_goalIndex[client]].campStart))
				{
					m_lookAt[client] = m_paths[m_goalIndex[client]].campStart;
					m_pauseTime[client] = GetGameTime() + crandomfloat(2.0, 7.0);
				}
				else if (IsVisible(GetEyePosition(client), m_paths[m_goalIndex[client]].campEnd))
				{
					m_lookAt[client] = m_paths[m_goalIndex[client]].campEnd;
					m_pauseTime[client] = GetGameTime() + crandomfloat(2.0, 7.0);
				}
				else if (IsVisible(GetEyePosition(client), m_paths[m_goalIndex[client]].campStart))
				{
					m_lookAt[client] = m_paths[m_goalIndex[client]].campStart;
					m_pauseTime[client] = GetGameTime() + crandomfloat(2.0, 7.0);
				}
				else
				{
				    if (m_paths[m_goalIndex[client]].radius > 0.0)
				    {
					    m_goalPosition[client][0] = m_paths[m_goalIndex[client]].origin[0] + crandomfloat(-m_paths[m_goalIndex[client]].radius, m_paths[m_goalIndex[client]].radius);
					    m_goalPosition[client][1] = m_paths[m_goalIndex[client]].origin[1] + crandomfloat(-m_paths[m_goalIndex[client]].radius, m_paths[m_goalIndex[client]].radius);
					    m_goalPosition[client][2] = m_paths[m_goalIndex[client]].origin[2];
				    }
				    else
					    m_goalPosition[client] = m_paths[m_goalIndex[client]].origin;

				    FindPath(client, m_goalPosition[client], -1, m_goalIndex[client]);
	        	    if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
					    return;
				}
			}
			else if (IsSniper(client) && m_primaryID[client] != 1098 && !TF2_IsPlayerInCondition(client, TFCond_Zoomed))
			{
				if (!IsWeaponSlotActive(client, 0) && !m_hasEnemiesNear[client])
					EquipWeaponSlot(client, 0);
				else
				    m_buttons[client] |= IN_ATTACK2;
			}

			FindFriendsAndEnemiens(client);
			CheckHealth(client);
			CheckAmmo(client);
			return;
		}
	}

	if (IsSniper(client) && m_primaryID[client] != 1098 && !TF2_IsPlayerInCondition(client, TFCond_Zoomed))
	{
		if (!IsWeaponSlotActive(client, 0))
			EquipWeaponSlot(client, 0);
		else
			m_buttons[client] |= IN_ATTACK2;
	}

	LookUpdate(client);
	FindFriendsAndEnemiens(client);
	CheckHealth(client);
	CheckAmmo(client);
}

stock void DefendEnd(const int client)
{

}
