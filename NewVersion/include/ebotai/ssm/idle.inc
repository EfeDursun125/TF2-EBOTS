stock bool IdleRequirements(const int client)
{
	if (TF2_HasTheFlag(client))
		return false;

	if (m_hasEnemiesNear[client])
		return false;

	return true;
}

stock void IdleStart(const int client)
{
	m_nextStuckCheck[client] += 5.0;
	m_moveVel[client][0] = 0.0;
	m_moveVel[client][1] = 0.0;
	m_moveVel[client][2] = 0.0;
}

stock void IdleUpdate(const int client)
{
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	LookUpdate(client, false);
	CheckHideFromDanger(client);

	if (TF2_IsPlayerInCondition(client, TFCond_Cloaked))
		m_buttons[client] |= IN_ATTACK2;
}

stock void IdleEnd(const int client)
{
	m_goalEntity[client] = -1;
}