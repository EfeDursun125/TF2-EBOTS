int m_botSquad[MAXPLAYERS + 1][6];
int m_squadThreat[MAXPLAYERS + 1];
float m_squadGoal[MAXPLAYERS + 1][3];

stock void JoinToSquad(int client)
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

stock void LeaveCurrentSquad(int client)
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

stock bool RequestToJoin(int client, int member)
{
	int count = GetSquadCount(member);
	if (count > 6)
	{
		// nah
		return false;
	}
	else
	{
		JoinToSquad(member);
		return true;
	}
}

stock void RequestSquadMembers(int client)
{
	m_squadGoal[client] = NULL_VECTOR;
	m_squadThreat[client] = -1;
	m_botSquad[client][0] = client;

	for (int i = 1; i <= MaxClients; i++)
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
		
		if ()
	}
}

stock bool IsSquadLeader(int client)
{
	return m_botSquad[client][0] == client;
}

stock bool HasSquad(int client)
{
	if (m_botSquad[client][0] == -1)
		return false;
	return true;
}

stock int GetSquadLeader(int client)
{
	return m_botSquad[client][0];
}

stock int GetSquadCount(int client)
{
	int count = 0;
	for (i = 0; i < m_botSquad[client]; i++)
	{
		if (m_botSquad[client][i] != -1)
			count++;
	}
	
	return count;
}

stock float IsSquadBusy(int client)
{
	int count = GetSquadCount(client);
	int busy = count;
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

stock float GetSquadGoal(int client)
{
	return m_squadGoal[m_botSquad[client][0]];
}

stock int GetSquadThreat(int client)
{
	return m_squadThreat[m_botSquad[client][0]];
}

stock bool RequestGoalUpdate(int client)
{
	return false;
}