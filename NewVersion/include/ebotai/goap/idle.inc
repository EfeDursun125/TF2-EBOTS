stock bool IdleRequirements(int client)
{
	if (TF2_HasTheFlag(client))
		return false;
	if (m_hasEnemiesNear[client])
		return false;
	return true;
}

stock void IdleStart(int client)
{
	m_nextStuckCheck[client] = CurrentProcessTime[client] + 5.0;
	m_nextPathUpdate[client] = CurrentProcessTime[client] + 5.0;
	m_moveVel[client][0] = 0.0;
	m_moveVel[client][1] = 0.0;
	m_moveVel[client][2] = 0.0;
}

stock void IdleUpdate(int client)
{
	FindFriendsAndEnemiens(client);
	LookUpdate(client, false);
}

stock void IdleEnd(int client)
{
	m_goalEntity[client] = -1;
}