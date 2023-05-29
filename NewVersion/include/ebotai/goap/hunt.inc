stock bool HuntRequirements(int client)
{
	if (!m_hasEnemiesNear[client])
		return false;
	
	if (m_lowHealth[client])
		return false;
	
	if (m_lowAmmo[client])
		return false;

	if (TF2_HasTheFlag(client))
		return false;
	
	return true;
}

stock void HuntStart(int client)
{
	m_goalEntity[client] = m_nearestEnemy[client];
	DeletePathNodes(client);
}

stock void HuntUpdate(int client)
{
	int x = m_goalEntity[client];

	if (!IsValidClient(x) || !IsPlayerAlive(x))
	{
		FinishCurrentProcess(client, false, "victim is not valid or already dead");
		return;
	}

	if (m_hasEnemiesNear[client] && m_nearestEnemy[client] == x)
		SetProcess(client, PRO_ATTACK, 180.0, "", true);

	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	LookUpdate(client);
	CheckHideFromDanger(client);
	CheckHealth(client);
	CheckAmmo(client);

	FollowPath(client, GetOrigin(x));
}

stock void HuntEnd(int client)
{
	m_goalEntity[client] = -1;
	RememberedProcess[client] = PRO_DEFAULT;
	RememberedProcessTime[client] = 0.0;
}