stock bool GetAmmoRequirements(int client)
{
	if (ammopacks <= 0)
		return false;
	
	if (pathdelayer >= GetGameTime())
		return false;
	
	if (!m_lowHealth[client])
		return false;
	
	if (TF2_HasTheFlag(client))
		return false;
	
	return true;
}

stock void GetAmmoStart(int client)
{
	FindNearestAmmoPack(client);
	DeletePathNodes(client);
}

stock void GetAmmoUpdate(int client)
{
	if (GetAmmo(client, 0) > 3 && GetAmmo(client, 1) > 6)
	{
		m_lowAmmo[client] = false;
		FinishCurrentProcess(client, true);
	}
	
	FindFriendsAndEnemiens(client);
	LookUpdate(client);
	
	if (IsValidEntity(m_goalEntity[client]))
	{
		if (!IsValidAmmoPack(m_goalEntity[client]))
		{
			FindNearestAmmoPack(client);
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
		FindNearestAmmoPack(client);
		// check again
		if (!IsValidEntity(m_goalEntity[client])) // no avaliable ammo packs.
			SetProcess(client, PRO_DEFAULT, true, 9999.0, "| process cancelled -> no ammo packs found");
	}
}

stock void GetAmmoEnd(int client)
{
	m_goalEntity[client] = -1;
}

stock void FindNearestAmmoPack(int client)
{
	float distance = -1.0;
	m_goalEntity[client] = -1;
	for (int x = 0; x <= GetMaxEntities(); x++)
	{
		if (!IsValidAmmoPack(x))
			continue;
		
		// only get safe ammo packs
		if (m_hasEnemiesNear[client] && GetVectorDistance(GetOrigin(x), GetOrigin(m_nearestEnemy[client]), true) <= Squared(1536))
			continue;

		//if (!IsPathPossible(GetOrigin(client), GetOrigin(x)))
		//	continue;
		
		float edict_distance = GetFastDistance(GetOrigin(client), GetOrigin(x));
		if (edict_distance < distance || distance == -1.0)
		{
			distance = edict_distance;
			m_goalEntity[client] = x;
		}
	}
}

stock bool IsValidAmmoPack(int entity)
{
	if (IsValidEntity(entity))
	{
		char ClassName[32];
		GetEdictClassname(entity, ClassName, 32);
		
		if	(StrContains(ClassName, "prop_dynamic", false) == -1 && !HasEntProp(entity, Prop_Send, "m_fEffects"))
			return false;
		
		if	(StrContains(ClassName, "prop_dynamic", false) == -1 && GetEntProp(entity, Prop_Send, "m_fEffects") != 0)
			return false;
		
		if	(StrContains(ClassName, "item_ammo", false) != -1 || StrContains(ClassName, "tf_ammo_pack", false) != -1 || StrContains(ClassName, "obj_dispenser", false) != -1 || StrContains(ClassName, "func_regen", false) != -1 || StrContains(ClassName, "pd_dispenser", false) != -1 || StrContains(ClassName, "rd_robot_dispenser", false) != -1 || StrContains(ClassName, "item_healthammokit", false) != -1)
			return true;
	}
	
	return false;
}