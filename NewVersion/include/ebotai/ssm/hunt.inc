stock bool HuntRequirements(const int client)
{
	if (!m_hasEnemiesNear[client])
		return false;

	if (m_lowHealth[client])
		return false;

	if (m_lowAmmo[client])
		return false;

	if (TF2_HasTheFlag(client))
		return false;

	if (!IsValidClient(m_goalEntity[client]) || !IsPlayerAlive(m_goalEntity[client]))
		return false;

	return true;
}

stock void HuntStart(const int client)
{
	DeletePathNodes(client);
}

stock void HuntUpdate(const int client)
{
	if (!IsValidClient(m_goalEntity[client]) || !IsPlayerAlive(m_goalEntity[client]))
	{
		FinishCurrentProcess(client, false, "victim is not valid or already dead");
		return;
	}
	else
		FollowPath(client, GetOrigin(m_goalEntity[client]));

	if (m_hasEnemiesNear[client] && m_nearestEnemy[client] == m_goalEntity[client] && !TF2_IsPlayerInCondition(client, TFCond_Ubercharged))
		SetProcess(client, PRO_ATTACK, 180.0, "", true);

	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	LookUpdate(client);
	CheckHideFromDanger(client);
	CheckHealth(client);
	CheckAmmo(client);
}

stock void HuntEnd(const int client)
{
	m_goalEntity[client] = -1;
	RememberedProcess[client] = PRO_DEFAULT;
	RememberedProcessTime[client] = 0.0;
}
