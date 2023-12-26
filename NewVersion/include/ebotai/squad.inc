int m_botSquad[TFMaxPlayers][6];
int m_squadThreat[TFMaxPlayers];
float m_squadGoal[MTFMaxPlayers][3];

stock void JoinToSquad(const int client)
{
	for (i = 0; i < m_botSquad[client]; i++)
	{
		if (m_botSquad[client][i] == -1)
		{
			m_botSquad[client][i] = client;
			break;
		}
	}
}

stock void LeaveCurrentSquad(const int client)
{
	for (i = 0; i < m_botSquad[client]; i++)
	{
		if (m_botSquad[client][i] == client)
		{
			m_botSquad[client][i] = client;
			break;
		}
	}
}

stock bool RequestToJoin(const int client, const int member)
{
	int count = GetSquadCount(member);
	if (count > 6)
		return false;
	else
	{
		JoinToSquad(member);
		return true;
	}
}

stock void RequestSquadMembers(const int client)
{
	m_squadGoal[client] = NULL_VECTOR;
	m_squadThreat[client] = -1;
	m_botSquad[client][0] = client;

	int i;
	for (i = 1; i <= MaxClients; i++)
	{
		if (client == i)
			continue;
		
		if (!IsValidClient(i))
			continue;
		
		if (!IsPlayerAlive(i))
			continue;
		
		if (GetClientTeam(i) != GetClientTeam(client))
			continue;
		
		if (IsSquadLeader(i))
			continue;
	}
}

stock bool IsSquadLeader(const int client)
{
	return m_botSquad[client][0] == client;
}

stock bool HasSquad(const int client)
{
	if (m_botSquad[client][0] == -1)
		return false;
	
	return true;
}

stock int GetSquadLeader(const int client)
{
	return m_botSquad[client][0];
}

stock int GetSquadCount(const int client)
{
	int i;
	int count = 0;
	for (i = 0; i < m_botSquad[client]; i++)
	{
		if (m_botSquad[client][i] != -1)
			count++;
	}
	
	return count;
}

stock float IsSquadBusy(const int client)
{
	int count = GetSquadCount(client);
	int busy = count;
	int i;
	for (i = 0; i < m_botSquad[client]; i++)
	{
		if (m_botSquad[client][i] == -1)
			continue;
		
		if (m_hasEnemiesNear[client])
			continue;
		
		if (m_lowHealth[client])
			continue;

		if (m_lowAmmo[client])
			continue;
		
		if (m_attackTimer[i] <= GetGameTime())
			continue;
		
		busy--;
	}

	return busy / count;
}

stock float GetSquadGoal(const int client)
{
	return m_squadGoal[m_botSquad[client][0]];
}

stock int GetSquadThreat(const int client)
{
	return m_squadThreat[m_botSquad[client][0]];
}

stock bool RequestGoalUpdate(const int client)
{
	return false;
}