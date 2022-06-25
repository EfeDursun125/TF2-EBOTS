stock bool AttackRequirements(int client)
{
	if (m_lowHealth[client])
		return false;
	
	if (NoDodge[client] >= GetGameTime())
		return false;
	
	if (m_enemyDistance[client] > Squared(m_eBotDodgeRange[client]))
		return false;
	
	if (!(GetEntityFlags(client) & FL_ONGROUND))
		return false;
	
	if (GetEntProp(client, Prop_Send, "m_bJumping"))
		return false;
	
	if (IsWeaponSlotActive(client, 2))
		return false;
	
	if (m_currentIndex[client] != -1 && m_paths[m_currentIndex[client]].flags == WAYPOINT_NOSTRAFE)
		return false;
	
	if (TF2_HasTheFlag(client))
		return false;
	
	if (TF2_GetPlayerClass(client) == TFClass_Spy && TF2_IsPlayerInCondition(client, TFCond_Cloaked))
		return false;
		
	return true;
}

stock void AttackStart(int client)
{
	m_goalEntity[client] = -1;
}

float m_wasdTimer[MAXPLAYERS + 1];
stock void AttackUpdate(int client)
{
	if (m_enemyDistance[client] > Squared(m_eBotDodgeRange[client]))
		FinishCurrentProcess(client, true);

	if (TF2_GetPlayerClass(client) == TFClass_Spy && m_lowHealth[client] && !TF2_IsPlayerInCondition(client, TFCond_Cloaked))
		m_buttons[client] |= IN_ATTACK2;

	if (m_isSlowThink[client] || !IsPlayerAlive(m_nearestEnemy[client]))
		FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	
	if (!m_hasEnemiesNear[client])
	{
		FinishCurrentProcess(client, true);
		if (!IsVisible(GetCenter(client), m_pathAhead[client])) // repath required
			DeletePathNodes(client);
		if (m_lowHealth[client])
			SetProcess(client, PRO_IDLE, GetRandomFloat(2.0, 4.0), "", true);
	}

	if (TF2_GetPlayerClass(client) == TFClass_Spy && (TF2_IsPlayerInCondition(client, TFCond_Disguised) || TF2_IsPlayerInCondition(client, TFCond_Cloaked)))
		FinishCurrentProcess(client, true);
	
	CheckHideFromDanger(client);
	CheckHealth(client);
	CheckAmmo(client);
	LookUpdate(client);

	if (TF2_GetPlayerClass(client) == TFClass_Scout || TF2_GetPlayerClass(client) == TFClass_Pyro)
	{
		if (m_enemyDistance[client] > Squared(GetConVarInt(m_eBotDodgeRangeMin)) || TF2_GetPlayerClass(m_nearestEnemy[client]) == TFClass_Spy)
		{
			if (IsValidClient(m_goalEntity[client]))
				FollowPath(client, GetOrigin(m_goalEntity[client]));
			else if (m_lastFailedEntity[client] != m_nearestEnemy[client])
				m_goalEntity[client] = m_nearestEnemy[client];
		}
		else if (ClientViews(m_nearestEnemy[client], client))
			RandomMovement(client);
		return;
	}

	if (ClientViews(m_nearestEnemy[client], client))
	{
		RandomMovement(client);
		m_goalEntity[client] = m_nearestEnemy[client];
	}
}

stock void RandomMovement(int client)
{
	if (m_wasdTimer[client] <= GetGameTime())
	{
		float maxspeed = GetMaxSpeed(client);
		float xmaxspeed = maxspeed * -1.0;
		m_moveVel[client][0] = GetRandomInt(1, 2) == 1 ? xmaxspeed : maxspeed;
		m_moveVel[client][1] = GetRandomInt(1, 2) == 1 ? xmaxspeed : maxspeed;
		m_wasdTimer[client] = GetGameTime() + GetRandomFloat(0.256, 0.768);
	}
}

stock void AttackEnd(int client)
{
	if (ChanceOf(GetConVarInt(m_eBotDodgeRangeChance)))
		m_eBotDodgeRange[client] = GetRandomInt(GetConVarInt(m_eBotDodgeRangeMin), GetConVarInt(m_eBotDodgeRangeMax));
}