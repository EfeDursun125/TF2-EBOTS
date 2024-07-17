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

	if (!m_hasWaypoints)
	{
		m_goalEntity[client] = FindNearestAmmo(client);
		if (!IsValidAmmoPack(m_goalEntity[client]) || !IsPathPossible(client, GetOrigin(client), GetOrigin(m_goalEntity[client])))
			return false;
	}

	if (m_class[client] == TFClass_Engineer && !IsMetalLow(client))
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

	if (m_class[client] == TFClass_Spy)
		SpyAimLogic(client);
	else
		LookUpdate(client);

	if (m_hasWaypoints)
		FollowPath(client, m_goalPosition[client]);
	else if (IsValidAmmoPack(m_goalEntity[client]))
		FollowPath(client, GetOrigin(m_goalEntity[client]));
	else if (!IsValidAmmoPack(m_goalEntity[client]) || !IsPathPossible(client, GetOrigin(client), GetOrigin(m_goalEntity[client])))
		FinishCurrentProcess(client);
}

stock void GetAmmoEnd(const int client)
{
	m_goalIndex[client] = -1;
	m_goalPosition[client] = NULL_VECTOR;
}

stock int FindNearestAmmo(const int client)
{
	float distance = 99999999999999999.0;
	float edict_distance;
	int nearestEntity = -1;
	float origin[3];
	origin = GetOrigin(client);

	int x;
	for (x = 0; x <= GetMaxEntities(); x++)
	{
		if (!IsValidAmmoPack(x))
			continue;

		edict_distance = GetFastDistance(origin, GetOrigin(x));
		if (edict_distance < distance)
		{
			distance = edict_distance;
			nearestEntity = x;
		}
	}

	return nearestEntity;
}
