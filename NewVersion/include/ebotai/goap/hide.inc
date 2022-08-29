int m_coverIndex[MAXPLAYERS + 1];

stock bool HideRequirements(int client)
{
	if (!m_hasWaypoints)
		return false;

	if (m_currentIndex[client] == -1)
		return false;
	
	if (TF2_HasTheFlag(client))
		return false;
	
	int index = FindCoverWaypoint(client);
	if (index != -1 && m_currentIndex[client] != index)
	{
		m_coverIndex[client] = index;
		return true;
	}

	return false;
}

stock void HideStart(int client)
{
	m_goalPosition[client] = m_paths[m_coverIndex[client]].origin;
	DeletePathNodes(client);
	AStarFindPath(m_currentIndex[client], m_coverIndex[client], client, m_goalPosition[client]);
}

stock void HideUpdate(int client)
{
	if (m_coverIndex[client] == -1)
	{
		FinishCurrentProcess(client, false, "impossible to find cover waypoint");
		return;
	}

	if (m_currentIndex[client] == -1)
	{
		FinishCurrentProcess(client, false, "unknown error!");
		return;
	}

	if (m_positions[client] != null && m_positions[client].Length > 12)
	{
		m_lastFailedWaypoint[client] = m_coverIndex[client];
		FinishCurrentProcess(client, false, "too far from cover waypoint");
		return;
	}

	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	LookUpdate(client);

	FollowPath(client, m_goalPosition[client]);

	if (m_hasEnemiesNear[client])
	{
		float distance = GetVectorDistance(GetOrigin(client), GetOrigin(m_nearestEnemy[client]), true);
		int range = Squared(256);
		if (distance <= range)
			BackstabMove(client, m_nearestEnemy[client], false);
	}
	else if (m_isSlowThink[client] && !IsMoving(client)) // same
	{
		DeletePathNodes(client);
		FinishCurrentProcess(client, true);
		SetProcess(client, PRO_IDLE, GetRandomFloat(2.0, 4.0), "", true);
	}

	if (m_currentIndex[client] == m_coverIndex[client])
	{
		if (m_hasEnemiesNear[client]) // enemy on cover index?
			HideStart(client);
		else
		{
			DeletePathNodes(client);
			FinishCurrentProcess(client, true);
			SetProcess(client, PRO_IDLE, GetRandomFloat(2.0, 4.0), "", true);
		}
	}
}

stock void HideEnd(int client)
{
	// nothing to do
}