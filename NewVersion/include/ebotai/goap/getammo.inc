stock bool GetAmmoRequirements(const int client)
{
	if (ammopacks <= 0)
		return false;
	
	if (m_lowHealth[client])
		return false;
	
	if (m_hasWaypoints && !m_hasAmmoWaypoints)
		return false;
	
	if (CurrentProcess[client] == PRO_HIDE)
		return false;
	
	if (isVSH && m_team[client] == 3)
		return false;
		
	if (TF2_HasTheFlag(client))
		return false;
	
	m_goalEntity[client] = FindNearestAmmo(client);
	if (!IsValidAmmoPack(m_goalEntity[client]) || !IsPathPossible(client, GetOrigin(client), GetOrigin(m_goalEntity[client])))
		return false;
	
	return true;
}

stock void GetAmmoStart(const int client)
{
	m_goalIndex[client] = -1;
	m_goalPosition[client] = NULL_VECTOR;
	DeletePathNodes(client);
}

stock void GetAmmoUpdate(const int client)
{
	if (m_class[client] == TFClass_Engineer)
	{
		if (!IsMetalLow(client))
		{
			m_lowAmmo[client] = false;
			FinishCurrentProcess(client, true);
			return;
		}
	}
	else
	{
		if (GetAmmo(client, 0) > 3 && GetAmmo(client, 1) > 6)
		{
			m_lowAmmo[client] = false;
			FinishCurrentProcess(client, true);
			return;
		}
	}

	CheckHideFromDanger(client);
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	LookUpdate(client);

	if (m_hasWaypoints)
		FollowPath(client, m_goalPosition[client]);
	else
	{
		static int ammoPack;
		if (!IsValidAmmoPack(ammoPack))
		{
			if (IsValidAmmoPack(m_goalEntity[client]))
				ammoPack = m_goalEntity[client];
			else
			{
				ammoPack = FindNearestAmmo(client);
				m_goalEntity[client] = ammoPack;
				if (!IsValidAmmoPack(ammoPack) || !IsPathPossible(client, GetOrigin(client), GetOrigin(ammoPack)))
					FinishCurrentProcess(client);
			}
		}

		FollowPath(client, GetOrigin(ammoPack));
	}
}

stock void GetAmmoEnd(const int client)
{
	m_goalIndex[client] = -1;
	m_goalPosition[client] = NULL_VECTOR;
	SelectObjective(client);
}

stock int FindNearestAmmo(const int client)
{
	float distance = 99999999999999999.0;
	int nearestEntity = -1;
	float origin[3];
	origin = GetOrigin(client);

	for (int x = TFMaxPlayers; x <= GetMaxEntities(); x++)
	{
		if (!IsValidAmmoPack(x))
			continue;

		float edict_distance = GetFastDistance(origin, GetOrigin(x));
		if (edict_distance < distance)
		{
			distance = edict_distance;
			nearestEntity = x;
		}
	}
	
	return nearestEntity;
}