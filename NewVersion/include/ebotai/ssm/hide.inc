float m_hidingSpot[TFMaxPlayers][3];

stock bool HideRequirements(const int client)
{
	if (!m_hasWaypoints)
	{
		m_hidingSpot[client] = FindBestHidingSpot(client);
		if (IsNullVector(m_hidingSpot[client]))
			return false;
	}
	else if (m_class[client] == TFClass_Spy)
		return false;
	
	if (TF2_HasTheFlag(client))
		return false;

	return true;
}

stock void HideStart(const int client)
{
	DeletePathNodes(client);
}

stock void HideUpdate(const int client)
{
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	LookUpdate(client);

	if (m_hasWaypoints)
	{
		if (IsValidClient(m_nearestEnemy[client]))
			FollowPath(client, GetOrigin(m_nearestEnemy[client]));
		else if (IsValidEntity(m_nearestEntity[client]))
			FollowPath(client, GetOrigin(m_nearestEntity[client]));
		else
			FinishCurrentProcess(client, false, "Nothing to hide");
	}
	else if (GetVectorDistance(GetOrigin(client), m_hidingSpot[client], true) > Squaredf(24.0))
		FollowPath(client, m_hidingSpot[client]);

	if (m_isSlowThink[client] && m_lastEnemySeen[client] + 4.0 < GetGameTime() && m_lastEntitySeen[client] + 4.0 < GetGameTime())
	{
		CurrentProcess[client] = PRO_DEFAULT;
		CheckHealth(client);
		if (CurrentProcess[client] != PRO_GETHEALTH)
		{
			CurrentProcess[client] = PRO_HIDE;
			if (!m_lowHealth[client] && CurrentProcess[client] != PRO_GETHEALTH)
				FinishCurrentProcess(client, false, "I found a safe place");
		}
	}
}

stock void HideEnd(const int client)
{
	m_hidingSpot[client] = NULL_VECTOR;
	DeletePathNodes(client);
}

stock float[] FindBestHidingSpot(const int client)
{
	CNavArea area = NavMesh.GetNearestNavArea(GetOrigin(client), true, 9999999999.0, true, false, m_team[client]);
	if (!area)
		return NULL_VECTOR;

	AreasCollector areas = NavMesh.CollectSurroundingAreas(area, 2048.0);

	int i;
	float height;
	float point[3];
	height = GetHeight(client);
	int count = areas.Count();
	for (i = 0; i < count; i++)
	{
		area = areas.Get(i);
		GetRandomPoint(area, point);

		if (IsVisible(GetEyePosition(client), point))
			continue;

		if (IsDangerous(client, point, height))
			continue;

		delete areas;
		return point;
	}

	delete areas;
	return NULL_VECTOR;
}