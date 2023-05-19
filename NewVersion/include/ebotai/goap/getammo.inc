stock bool GetAmmoRequirements(int client)
{
	if (ammopacks <= 0)
		return false;
	
	if (m_lowHealth[client])
		return false;
	
	if (m_hasWaypoints && !m_hasAmmoWaypoints)
		return false;
	
	if (CurrentProcess[client] == PRO_HIDE)
		return false;
	
	if (isVSH && GetClientTeam(client) == 3)
		return false;
		
	if (TF2_HasTheFlag(client))
		return false;
	
	return true;
}

stock void GetAmmoStart(int client)
{
	m_goalEntity[client] = -1;
	m_goalIndex[client] = -1;
	m_goalPosition[client] = NULL_VECTOR;
	DeletePathNodes(client);
}

stock void GetAmmoUpdate(int client)
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

	FollowPath(client, m_goalPosition[client]);
}

stock void GetAmmoEnd(int client)
{
	m_goalEntity[client] = -1;
	m_goalIndex[client] = -1;
	m_goalPosition[client] = NULL_VECTOR;
	SelectObjective(client);
}