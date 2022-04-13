stock void ThinkAI(int client)
{
	UpdateProcess(client);
}

// automatic look update
stock void LookUpdate(int client, bool lookaround = true)
{
	if (TF2_GetPlayerClass(client) == TFClass_Pyro && m_hasFriendsNear[client] && TF2_IsPlayerInCondition(m_nearestFriend[client], TFCond_OnFire) && m_friendDistance[client] <= Squared(512))
	{
		m_lookAt[client] = GetEyePosition(m_nearestFriend[client]);
		if (m_friendDistance[client] <= Squared(192))
			m_attack2Timer[client] = GetGameTime() + 0.2;
		else
			MoveTo(client, GetOrigin(m_nearestFriend[client]));
		return;
	}

	if (m_hasEnemiesNear[client])
	{
		LookAtEnemiens(client);
		AutoAttack(client);
		SelectBestCombatWeapon(client);
		m_pauseTime[client] = GetGameTime() + GetRandomFloat(1.25, 2.25);
		return;
	}

	if (lookaround)
		LookAround(client);
}