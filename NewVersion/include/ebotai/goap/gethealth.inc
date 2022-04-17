stock bool GetHealthRequirements(int client)
{
	if (healthpacks <= 0)
		return false;

	if (pathdelayer >= GetGameTime())
		return false;

	if (!m_lowHealth[client])
		return false;

	if (m_hasWaypoints && !m_hasHealthWaypoints)
	
	if (TF2_HasTheFlag(client))
		return false;
	
	return true;
}

stock void GetHealthStart(int client)
{
	FindNearestHealthPack(client);
	DeletePathNodes(client);
}

stock void GetHealthUpdate(int client)
{
	if ((GetClientHealth(client) > (GetMaxHealth(client) / 1.4)))
	{
		m_lowHealth[client] = false;
		FinishCurrentProcess(client, true);
		return;
	}
	
	FindFriendsAndEnemiens(client);
	LookUpdate(client);
	
	if (m_hasWaypoints)
	{
		int index = FindHealthWaypoint(client);
		if (index != -1)
		{
			m_goalPosition[client] = VectorAsFloat(m_paths[index].origin);
			if (GetVectorDistance(GetOrigin(client), m_goalPosition[client], true) <= Squared(48))
				MoveTo(client, m_goalPosition[client], false);
			else
				FollowPath(client, m_goalPosition[client]);
		}
		else
		{
			FindNearestHealthPack(client);
			// check again
			if (!IsValidEntity(m_goalEntity[client])) // no avaliable health packs.
				SetProcess(client, PRO_DEFAULT, true, 9999.0, "| process cancelled -> no health packs found");
		}
	}
	else if (IsValidEntity(m_goalEntity[client]))
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
			FollowPath(client, m_goalPosition[client]);
	}
	else
	{
		FindNearestHealthPack(client);
		// check again
		if (!IsValidEntity(m_goalEntity[client])) // no avaliable health packs.
			SetProcess(client, PRO_DEFAULT, true, 9999.0, "| process cancelled -> no health packs found");
	}
}

stock void GetHealthEnd(int client)
{
	m_goalEntity[client] = -1;
}

stock int FindHealthWaypoint(int client)
{
	int index = -1;
	ArrayList NearHealthPacks = new ArrayList();
	ArrayList MedHealthPacks = new ArrayList();
	ArrayList AllHealthPacks = new ArrayList();
	for (int i = 0; i < m_waypointNumber; i++)
	{
		// blocked waypoint
		if (m_paths[i].activeArea != 0 && m_paths[i].activeArea != currentActiveArea)
			continue;
		
		if (m_paths[i].flags != _:WAYPOINT_HEALTH && m_paths[i].flags != _:WAYPOINT_RESUPPLY)
			continue;
		
		// not for our team
    	if (m_paths[i].team == GetEnemyTeam(GetClientTeam(client)))
      		continue;
		
		// only get safe health packs
		float distance = GetVectorDistance(VectorAsFloat(m_paths[i].origin), GetOrigin(m_nearestEnemy[client]), true);
		if (m_hasEnemiesNear[client] && distance <= Squared(768))
			continue;
		
		if (m_hasEnemiesNear[client] && distance <= GetVectorDistance(VectorAsFloat(m_paths[i].origin), GetOrigin(client)))
			continue;
		
		if (GetVectorDistance(VectorAsFloat(m_paths[i].origin), GetOrigin(client), true) <= Squared(1024))
			NearHealthPacks.Push(i);
		else if (GetVectorDistance(VectorAsFloat(m_paths[i].origin), GetOrigin(client), true) <= Squared(2048))
			MedHealthPacks.Push(i);
		else
			AllHealthPacks.Push(i);
	}

	if (NearHealthPacks.Length > 1)
		index = NearHealthPacks.Get(GetRandomInt(0, NearHealthPacks.Length - 1));
	else if (MedHealthPacks.Length > 1)
		index = MedHealthPacks.Get(GetRandomInt(0, MedHealthPacks.Length - 1));
	else if (AllHealthPacks.Length > 0)
		index = AllHealthPacks.Get(GetRandomInt(0, AllHealthPacks.Length - 1));

	delete NearHealthPacks;
	delete MedHealthPacks;
	delete AllHealthPacks;

	m_goalIndex[client] = index;
	return index;
}

stock void FindNearestHealthPack(int client)
{
	float distance = -1.0;
	m_goalEntity[client] = -1;
	for (int x = 0; x <= GetMaxEntities(); x++)
	{
		if (!IsValidHealthPack(x))
			continue;
		
		// only get safe health packs
		if (m_hasEnemiesNear[client] && GetVectorDistance(GetOrigin(x), GetOrigin(m_nearestEnemy[client]), true) <= Squared(1536))
			continue;
		
		if (!IsPathPossible(GetOrigin(client), GetOrigin(x)))
			continue;
		
		float edict_distance = GetFastDistance(GetOrigin(client), GetOrigin(x));
		if (edict_distance < distance || distance == -1.0)
		{
			distance = edict_distance;
			m_goalEntity[client] = x;
		}
	}
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