stock bool GetHealthRequirements(int client)
{
	if (healthpacks <= 0)
		return false;

	if (!m_lowHealth[client])
		return false;

	if (m_hasWaypoints && !m_hasHealthWaypoints)
		return false;
	
	if (CurrentProcess[client] == PRO_HIDE)
		return false;
	
	if (m_hasEnemiesNear[client])
		return false;

	if (TF2_HasTheFlag(client))
		return false;
	
	return true;
}

stock void GetHealthStart(int client)
{
	m_goalEntity[client] = -1;
	m_goalIndex[client] = -1;
	m_goalPosition[client] = NULL_VECTOR;

	FindNearestHealthPack(client);
	DeletePathNodes(client);
}

stock void GetHealthUpdate(int client)
{
	if (GetClientHealth(client) >= GetMaxHealth(client))
	{
		FinishCurrentProcess(client);
		return;
	}

	CheckHideFromDanger(client);	
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	LookUpdate(client);
	
	if (IsValidEntity(m_goalEntity[client]))
	{
		if (!IsValidHealthPack(m_goalEntity[client]))
		{
			FindNearestHealthPack(client);
			return;
		}

		m_goalPosition[client] = GetOrigin(m_goalEntity[client]);
		if (GetVectorDistance(GetOrigin(client), m_goalPosition[client], true) <= Squared(48))
			MoveTo(client, m_goalPosition[client], false);
		else
			FollowPath(client, m_goalPosition[client], m_goalEntity[client]);
	}
	else
	{
		FindNearestHealthPack(client);

		// check again
		if (!IsValidEntity(m_goalEntity[client])) // no avaliable health packs
			SetFailedProcess(client, true);
	}
}

stock void GetHealthEnd(int client)
{
	m_goalEntity[client] = -1;
	m_goalIndex[client] = -1;
	m_goalPosition[client] = NULL_VECTOR;
}

stock void FindNearestHealthPack(int client)
{
	float distance = -1.0;
	m_goalEntity[client] = -1;
	for (int x = 0; x <= GetEntityCount(); x++)
	{
		if (m_lastFailedEntity[client] == x)
			continue;
		
		if (!IsValidHealthPack(x))
			continue;
		
		// only get safe ammo packs
		if (m_hasEnemiesNear[client] && GetVectorDistance(GetOrigin(x), GetOrigin(m_nearestEnemy[client]), true) <= GetVectorDistance(GetOrigin(x), GetOrigin(client), true))
			continue;
		
		int index = FindNearestWaypoint(GetOrigin(x), 64.0, client);
		if (index == -1)
			continue;
		
		float edict_distance = GetFastDistance(GetOrigin(client), GetOrigin(x));
		if (edict_distance < distance || distance == -1.0)
		{
			distance = edict_distance;
			m_goalEntity[client] = x;
		}
	}

	m_goalIndex[client] = FindNearestWaypoint(GetOrigin(m_goalEntity[client]), 64.0, client);
}

stock bool IsValidHealthPack(int entity)
{
	if (IsValidEntity(entity))
	{
		char ClassName[32];
		GetEdictClassname(entity, ClassName, 32);
		
		if (StrContains(ClassName, "prop_dynamic", false) == -1 && !HasEntProp(entity, Prop_Send, "m_fEffects"))
			return false;
		
		if (StrContains(ClassName, "prop_dynamic", false) == -1 && GetEntProp(entity, Prop_Send, "m_fEffects") != 0)
			return false;
		
		if (StrContains(ClassName, "item_health", false) != -1 || StrContains(ClassName, "obj_dispenser", false) != -1 || StrContains(ClassName, "func_regen", false) != -1 || StrContains(ClassName, "rd_robot_dispenser", false) != -1 || StrContains(ClassName, "pd_dispenser", false) != -1)
			return true;
	}
	
	return false;
}