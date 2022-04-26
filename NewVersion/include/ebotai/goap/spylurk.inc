public bool SpyLurkRequirements(int client)
{
	if (TF2_HasTheFlag(client))
		return false;
	
	return true;
}

public void SpyLurkStart(int client)
{
	if (!IsValidClient(m_goalEntity[client]) || !IsValidEntity(m_goalEntity[client]))
		SelectObjective(client);
}

public void SpyLurkUpdate(int client)
{
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	CheckHealth(client);
	CheckAmmo(client);

	if (m_hasEnemiesNear[client])
		LookAtEnemiens(client);
	else
		LookAround(client);
	
	if (IsValidClient(m_goalEntity[client]) && IsPlayerAlive(m_goalEntity[client]))
		FollowPath(client, GetOrigin(m_goalEntity[client]));
	else if (IsValidEntity(m_goalEntity[client]))
		ObjectiveMovement(client);
	else
	{
		int enemy = FindNearestEnemy(client);
		if (IsValidClient(enemy) && IsPlayerAlive(enemy) && IsPathPossible(GetOrigin(client), GetOrigin(enemy)))
		{
			m_goalEntity[client] = enemy;
			m_goalPosition[client] = GetOrigin(enemy);
		}
		else
			SelectObjective(client);
	}
	
	if (m_hasEnemiesNear[client])
	{
		float distance = GetVectorDistance(GetOrigin(client), GetOrigin(m_nearestEnemy[client]), true);
		int range = Squared(256);
		if (distance <= range)
		{
			BackstabMove(client, m_nearestEnemy[client]);
			return;
		}
		m_goalEntity[client] = m_nearestEnemy[client];
		if (IsValidClient(m_goalEntity[client]) && IsPlayerAlive(m_goalEntity[client]))
		{
			SetProcess(client, PRO_SPYHUNT, GetRandomFloat(10.0, 20.0), "| process started -> try to hunt down enemies");
			return;
		}
	}
}

public void SpyLurkEnd(int client)
{
	if (!IsValidClient(m_goalEntity[client]) || !IsValidEntity(m_goalEntity[client]))
		SelectObjective(client);
}