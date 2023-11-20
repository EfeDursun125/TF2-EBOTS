stock bool DefendRequirements(int client)
{
	if (!m_hasWaypoints)
		return false;

	if (m_goalIndex[client] == -1)
		return false;

	if (!(m_paths[m_goalIndex[client]].activeArea & currentActiveArea))
		return false;

	if (TF2_HasTheFlag(client))
		return false;

	return true;
}

stock void DefendStart(int client)
{
	m_goalEntity[client] = -1;
}

stock void DefendUpdate(int client)
{
	if (m_goalIndex[client] == -1 || !(m_paths[m_goalIndex[client]].activeArea & currentActiveArea))
	{
		FinishCurrentProcess(client, true);
		return;
	}
	
	if (m_hasEnemiesNear[client] && ClientViews(m_nearestEnemy[client], client, 0.7) && IsAttacking(m_nearestEnemy[client]))
		SetProcess(client, PRO_ATTACK, 180.0, "", true);

	if (GetVectorDistance(GetOrigin(client), m_goalPosition[client], true) <= Squaredf(24.0))
	{
		m_moveVel[client][0] = 0.0;
		m_moveVel[client][1] = 0.0;
		m_moveVel[client][2] = 0.0;
		if (m_hasEnemiesNear[client])
			LookUpdate(client, false);
	}
	else
	{
		FollowPath(client, m_goalPosition[client]);
		LookUpdate(client);
	}
	
	FindFriendsAndEnemiens(client);
	CheckHealth(client);
	CheckAmmo(client);
}

stock void DefendEnd(int client)
{
	m_goalIndex[client] = -1;
}