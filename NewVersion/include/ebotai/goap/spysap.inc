public bool SpySapRequirements(int client)
{
	if (TF2_IsPlayerInCondition(client, TFCond_Cloaked))
		return false;
	
	if (!TF2_IsPlayerInCondition(client, TFCond_Disguised))
		return false;
	
	if (!IsValidEntity(m_knownSentry[client]))
		return false;
	
	if (!HasEntProp(m_knownSentry[client], Prop_Send, "m_bHasSapper"))
		return false;
	
	if (TF2_HasTheFlag(client))
		return false;
	
	// no sapper, WHAT?
	if (!IsValidEntity(GetPlayerWeaponSlot(client, 1)))
		return false;
	
	return true;
}

public void SpySapStart(int client)
{
	EquipWeaponSlot(client, 2);
	if (!m_hasEnemiesNear[client] && TF2_IsPlayerInCondition(client, TFCond_Cloaked))
		m_buttons[client] |= IN_ATTACK2;
}

public void SpySapUpdate(int client)
{
	if (!TF2_IsPlayerInCondition(client, TFCond_Disguised))
		ActiveCloak(client);

	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);

	m_goalEntity[client] = m_knownSentry[client];

	if (!DeactiveCloak(client))
	{
		LookAround(client);
		if (IsValidClient(m_goalEntity[client]) || IsValidEntity(m_goalEntity[client]))
			FollowPath(client, GetOrigin(m_goalEntity[client]));
		
		return;
	}

	CheckHealth(client);
	CheckAmmo(client);
	SpyAimLogic(client);

	m_goalEntity[client] = m_knownSentry[client];
	if (!IsValidEntity(m_goalEntity[client]) || !HasEntProp(m_goalEntity[client], Prop_Send, "m_bHasSapper"))
	{
		FinishCurrentProcess(client);
		return;
	}

	FollowPath(client, GetOrigin(m_goalEntity[client]));

	if (HasEntProp(m_goalEntity[client], Prop_Send, "m_bHasSapper") && GetEntProp(m_goalEntity[client], Prop_Send, "m_bHasSapper") == 0)
	{
		float distance = GetVectorDistance(GetOrigin(client), GetCenter(m_goalEntity[client]), true);
		float range = Squaredf(256.0);

		if (distance <= range)
		{
			if (IsWeaponSlotActive(client, 1))
			{
				m_lookAt[client] = GetCenter(m_goalEntity[client]);
				m_attackTimer[client] = GetGameTime() + 0.2;
			}
			else
				FakeClientCommandThrottled(client, "build 3 0"); // place sapper
		}
	}

	if (HasEntProp(m_goalEntity[client], Prop_Send, "m_bHasSapper") && GetEntProp(m_goalEntity[client], Prop_Send, "m_bHasSapper") != 0)
		FinishCurrentProcess(client);
	
	if (!m_hasEntitiesNear[client])
		SpyReactChecker(client);
}

public void SpySapEnd(int client)
{
	if (IsWeaponSlotActive(client, 1))
	{
		int weapon = GetRandomInt(0, 1);
		if (weapon == 1)
			EquipWeaponSlot(client, 0);
		else
			EquipWeaponSlot(client, 2);
	}
	
	m_knownSentry[client] = -1; // what if sentry is unreachable???
}