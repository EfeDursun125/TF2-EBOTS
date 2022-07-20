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
	
	if (TF2_IsPlayerInCondition(client, TFCond_Cloaked))
		m_buttons[client] |= IN_ATTACK2;
}

public void SpyLurkUpdate(int client)
{
	CurrentProcessTime[client] = GetGameTime() + 9999.0; // no timeout

	if (m_hasEnemiesNear[client] && TF2_IsPlayerInCondition(client, TFCond_Cloaked))
	{
		SetProcess(client, PRO_HIDE, 60.0, "", true);
		return;
	}
	
	if (IsValidEntity(m_knownSentry[client]))
	{
		if (HasEntProp(m_nearestEntity[client], Prop_Send, "m_bHasSapper") && GetEntProp(m_knownSentry[client], Prop_Send, "m_bHasSapper") == 0)
		{
			SetProcess(client, PRO_SPYSAP, GetRandomFloat(40.0, 80.0), "| process started -> try to sap buildings");
			return;
		}
		else
			m_knownSentry[client] = -1;
	}

	

	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	CheckHealth(client);
	CheckAmmo(client);
	SelectBestCombatWeapon(client);

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
		if (IsValidClient(enemy) && IsPlayerAlive(enemy) && m_lastFailedEntity[client] != enemy)
		{
			m_goalEntity[client] = enemy;
			m_goalPosition[client] = GetOrigin(enemy);
		}
		else
			SelectObjective(client);
	}
	
	if (m_hasEnemiesNear[client] && TF2_GetPlayerClass(m_nearestEnemy[client]) != TFClass_Spy)
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