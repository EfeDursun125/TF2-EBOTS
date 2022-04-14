stock bool SpyLurkRequirements(int client)
{
	if (TF2_HasTheFlag(client))
		return false;
	
	return true;
}

stock void SpyLurkStart(int client)
{
	if (!IsValidClient(m_goalEntity[client]) || !IsValidEntity(m_goalEntity[client]))
		SelectObjective(client);
}

stock void SpyLurkUpdate(int client)
{
	if (TF2_IsPlayerInCondition(client, TFCond_CritOnWin) && m_hasEnemiesNear[client])
	{
		FollowPath(client, GetOrigin(m_nearestEnemy[client]));
		FindFriendsAndEnemiens(client);
		LookUpdate(client);
		return;
	}

	if (IsValidClient(m_goalEntity[client]))
		FollowPath(client, GetOrigin(m_goalEntity[client]));
	else if (IsValidEntity(m_goalEntity[client]))
		ObjectiveMovement(client);
	else
	{
		int enemy = FindNearestEnemy(client);
		if (IsPathPossible(GetOrigin(client), GetOrigin(enemy)))
		{
			m_goalEntity[client] = enemy;
			m_goalPosition[client] = GetOrigin(enemy);
		}
		else
			SelectObjective(client);
	}
	
	if (m_hasEnemiesNear[client])
	{
		m_goalEntity[client] = m_nearestEnemy[client];
		if (IsValidClient(m_goalEntity[client]))
		{
			SetProcess(client, PRO_SPYHUNT, false, GetRandomFloat(10.0, 20.0), "| process started -> try to hunt down enemies");
			return;
		}
	}
	
	FindFriendsAndEnemiens(client);
	CheckHealth(client);
	CheckAmmo(client);
	if (m_hasEnemiesNear[client])
		LookAtEnemiens(client);
	else
		LookAround(client);
}

stock void SpyLurkEnd(int client)
{
	if (!IsValidClient(m_goalEntity[client]) || !IsValidEntity(m_goalEntity[client]))
		SelectObjective(client);
}