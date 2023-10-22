stock bool HideRequirements(int client)
{
	if (!m_hasWaypoints)
		return false;
	
	if (m_class[client] == TFClass_Spy)
		return false;
	
	if (TF2_HasTheFlag(client))
		return false;

	return true;
}

stock void HideStart(int client)
{
	DeletePathNodes(client);
}

stock void HideUpdate(int client)
{
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	LookUpdate(client);

	if (IsValidClient(m_nearestEnemy[client]))
		FollowPath(client, GetOrigin(m_nearestEnemy[client]));
	else if (IsValidEntity(m_nearestEntity[client]))
		FollowPath(client, GetOrigin(m_nearestEntity[client]));
	else
		FinishCurrentProcess(client, false, "Nothing to hide");

	if (m_lastEnemySeen[client] + 4.0 < GetGameTime() && m_lastEntitySeen[client] + 4.0 < GetGameTime())
		FinishCurrentProcess(client, false, "I found a safe place");
}

stock void HideEnd(int client)
{
	// DED suprise not big
}
