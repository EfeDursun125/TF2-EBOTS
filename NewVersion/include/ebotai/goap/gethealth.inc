stock bool GetHealthRequirements(int client)
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
	
	return true;
}

stock void GetHealthStart(int client)
{
	m_goalEntity[client] = -1;
	m_goalIndex[client] = -1;
	m_goalPosition[client] = NULL_VECTOR;
	DeletePathNodes(client);
}

stock void GetHealthUpdate(int client)
{
	if (GetClientHealth(client) >= GetMaxHealth(client))
	{
		FinishCurrentProcess(client);
		return;
	}
	else
		FollowPath(client, m_goalPosition[client]);

	CheckHideFromDanger(client);	
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	LookUpdate(client);
}

stock void GetHealthEnd(int client)
{
	m_goalEntity[client] = -1;
	m_goalIndex[client] = -1;
	m_goalPosition[client] = NULL_VECTOR;
	SelectObjective(client);
}