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
	if (m_hasEnemiesNear[client] && TF2_IsPlayerInCondition(client, TFCond_Cloaked))
	{
		SetProcess(client, PRO_HIDE, 60.0, "", true);
		return;
	}

	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	CheckHealth(client);
	CheckAmmo(client);

	m_goalEntity[client] = m_knownSentry[client];
	if (!IsValidEntity(m_goalEntity[client]) || !HasEntProp(m_goalEntity[client], Prop_Send, "m_bHasSapper"))
	{
		FinishCurrentProcess(client);
		return;
	}

	if (m_hasEnemiesNear[client])
	{
		float distance = GetVectorDistance(GetOrigin(client), GetOrigin(m_nearestEnemy[client]), true);
		int range = Squared(256);
		if (distance <= range)
		{
			BackstabMove(client, m_nearestEnemy[client]);
			return;
		}
	}

	FollowPath(client, GetOrigin(m_goalEntity[client]));
	
	bool lookAt = true;
	if (HasEntProp(m_goalEntity[client], Prop_Send, "m_bHasSapper") && GetEntProp(m_goalEntity[client], Prop_Send, "m_bHasSapper") == 0)
	{
		float distance = GetVectorDistance(GetOrigin(client), GetCenter(m_goalEntity[client]), true);
		int range = Squared(256);
		if (distance <= range)
		{
			m_lookAt[client] = GetCenter(m_goalEntity[client]);
			EquipWeaponSlot(client, 5);
			FakeClientCommandThrottled(client, "build 3 0"); // place sapper
			m_buttons[client] |= IN_ATTACK;
		}
	}

	if (lookAt)
		SpyAimLogic(client);
	
	if (!m_hasEntitiesNear[client])
		SpyReactChecker(client);
}

public void SpySapEnd(int client)
{
	if (IsWeaponSlotActive(client, 5))
	{
		int weapon = GetRandomInt(0, 1);
		EquipWeaponSlot(client, weapon);
	}
	m_knownSentry[client] = -1; // what if sentry is unreachable???
}