stock bool GetHealthRequirements(const int client)
{
	if (healthpacks <= 0)
		return false;

	if (!m_lowHealth[client])
		return false;

	if (m_hasWaypoints && !m_hasHealthWaypoints)
		return false;
	
	if (CurrentProcess[client] == PRO_HIDE)
		return false;
	
	if (m_hasEnemiesNear[client])
		return false;
	
	if (isVSH && GetClientTeam(client) == 3)
		return false;

	if (TF2_HasTheFlag(client))
		return false;
	
	if (!m_hasWaypoints)
	{
		m_goalEntity[client] = FindNearestHealth(client);
		if (!IsValidHealthPack(m_goalEntity[client]) || !IsPathPossible(client, GetOrigin(client), GetOrigin(m_goalEntity[client])))
			return false;
	}

	return true;
}

stock void GetHealthStart(const int client)
{
	m_goalIndex[client] = -1;
	m_goalPosition[client] = NULL_VECTOR;
	DeletePathNodes(client);
}

stock void GetHealthUpdate(const int client)
{
	if (GetClientHealth(client) > (GetMaxHealth(client) / 1.4))
	{
		FinishCurrentProcess(client);
		DeletePathNodes(client);
		return;
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
	else if (IsValidHealthPack(m_goalEntity[client]))
		FollowPath(client, GetOrigin(m_goalEntity[client]));
	else
	{
		m_goalEntity[client] = FindNearestHealth(client);
		if (!IsValidHealthPack(m_goalEntity[client]) || !IsPathPossible(client, GetOrigin(client), GetOrigin(m_goalEntity[client])))
			FinishCurrentProcess(client);
	}
}

stock void GetHealthEnd(const int client)
{
	m_goalIndex[client] = -1;
	m_goalPosition[client] = NULL_VECTOR;
}

stock int FindNearestHealth(const int client)
{
	float distance = 99999999999999999.0;
	float edict_distance;
	int nearestEntity = -1;
	float origin[3];
	origin = GetOrigin(client);

	int x;
	for (x = 0; x <= GetMaxEntities(); x++)
	{
		if (!IsValidHealthPack(x))
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