public bool SpyLurkRequirements(const int client)
{
	if (TF2_HasTheFlag(client))
	{
		if (m_hasEnemiesNear[client] || m_hasEntitiesNear[client])
			m_attackTimer[client] = GetGameTime() + 0.2;
		
		return false;
	}
	
	return true;
}

public void SpyLurkStart(const int client)
{
	if (!IsValidClient(m_goalEntity[client]) || !IsValidEntity(m_goalEntity[client]))
		SelectObjective(client);
}

public void SpyLurkUpdate(const int client)
{
	if (m_class[client] != TFClass_Spy)
	{
		SetProcess(client, PRO_DEFAULT, 99999.0, "", true, false);
		CurrentProcess[client] = PRO_DEFAULT;
		return;
	}

	CurrentProcessTime[client] = GetGameTime() + 99999.0; // no timeout

	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);

	if (m_useTeleporter[client] && IsValidEntity(m_teleporterEntity[client]))
	{
		DeletePathNodes(client);
		m_nextStuckCheck[client] = GetGameTime() + 5.0;
		if (GetVectorDistance(GetOrigin(client), GetOrigin(m_teleporterEntity[client]), true) <= Squaredf(24.0))
		{
			m_moveVel[client][0] = 0.0;
			m_moveVel[client][1] = 0.0;
			m_moveVel[client][2] = 0.0;
		}
		else
			MoveToWaypoint(client, GetOrigin(m_teleporterEntity[client]));
		return;
	}

	if (!DeactiveCloak(client))
	{
		LookAround(client);
		if (IsValidClient(m_goalEntity[client]) || IsValidEntity(m_goalEntity[client]))
			FollowPath(client, GetOrigin(m_goalEntity[client]));
	}
	else
	{
		CheckHealth(client);
		CheckAmmo(client);
		SelectBestCombatWeapon(client);
		SpyAimLogic(client);

		if (IsValidEntity(m_knownSentry[client]))
		{
			if (HasEntProp(m_knownSentry[client], Prop_Send, "m_bHasSapper") && GetEntProp(m_knownSentry[client], Prop_Send, "m_bHasSapper") == 0)
			{
				SetProcess(client, PRO_SPYSAP, crandomfloat(40.0, 80.0), "| process started -> try to sap buildings");
				return;
			}
			else
				m_knownSentry[client] = -1;
		}
		
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
	
		if (m_hasEnemiesNear[client] && m_class[m_nearestEnemy[client]] != TFClass_Spy)
		{
			if (GetVectorDistance(GetOrigin(client), GetOrigin(m_nearestEnemy[client]), true) < Squaredf(256.0))
			{
				BackstabMove(client, m_nearestEnemy[client]);
				return;
			}

			m_goalEntity[client] = m_nearestEnemy[client];

			if (IsValidClient(m_goalEntity[client]) && IsPlayerAlive(m_goalEntity[client]))
			{
				SetProcess(client, PRO_SPYHUNT, crandomfloat(10.0, 20.0), "| process started -> try to hunt down enemies");
				return;
			}
		}
	}

	// avoid bumping!!!
	if (m_hasEnemiesNear[client] && IsValidClient(m_nearestEnemy[client]) && GetVectorDistance(GetOrigin(client), GetOrigin(m_nearestEnemy[client]), true) <= Squaredf(200.0))
		BackstabMove(client, m_nearestEnemy[client], false);
}

public void SpyLurkEnd(const int client)
{
	if (!IsValidClient(m_goalEntity[client]) || !IsValidEntity(m_goalEntity[client]))
		SelectObjective(client);
}