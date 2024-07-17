stock bool AttackRequirements(const int client)
{
    if (isOT)
        return false;

	if (isVSH)
		return false;

	if (m_lowHealth[client])
		return false;

	if (m_lowAmmo[client])
		return false;

	if (NoDodge[client] > GetGameTime())
		return false;

	switch (m_class[client])
	{
		case TFClass_Sniper:
		{
		}
		case TFClass_Scout:
		{
		}
		case TFClass_Pyro:
		{
		}
		case TFClass_Spy:
		{
			if (TF2_IsPlayerInCondition(client, TFCond_Cloaked) || TF2_IsPlayerInCondition(client, TFCond_Disguising))
				return false;
		}
		case TFClass_Heavy:
		{
			if (TF2_IsPlayerInCondition(client, TFCond_Overhealed))
        		return false;
		}
		case TFClass_Medic:
		{
			if (TF2_IsPlayerInCondition(client, TFCond_Ubercharged))
        		return false;
		}
		default:
		{
			if (m_enemyDistance[client] > Squared(m_eBotDodgeRange[client]))
				return false;
		}
	}

	if (IsWeaponSlotActive(client, 2))
		return false;

	if (m_currentIndex[client] != -1 && m_paths[m_currentIndex[client]].flags & WAYPOINT_NOSTRAFE)
		return false;

	if (TF2_HasTheFlag(client))
		return false;

	return true;
}

stock void AttackStart(const int client)
{

}

float m_wasdTimer[TFMaxPlayers];
stock void AttackUpdate(const int client)
{
    if (!IsMedic(client) && TF2_IsPlayerInCondition(client, TFCond_Ubercharged))
	{
		int enemy = FindNearestEnemy(client);
		if (IsValidClient(enemy) && IsPlayerAlive(enemy) && m_lastFailedEntity[client] != enemy)
		{
			m_goalIndex[client] = -1;
			m_goalEntity[client] = enemy;
			m_goalPosition[client] = GetOrigin(enemy);
			if (SetProcess(client, PRO_HUNTENEMY, 15.0, "", true))
				return;
		}
	}
	else if (m_class[client] == TFClass_Spy)
	{
		if (m_lowHealth[client] && ActiveCloak(client))
		{
			FinishCurrentProcess(client, true);
			return;
		}

		if ((TF2_IsPlayerInCondition(client, TFCond_Disguised) || IsStealthed(client)))
		{
			FinishCurrentProcess(client, true);
			return;
		}
	}

	if (m_isSlowThink[client] || (IsValidClient(m_nearestEnemy[client]) && !IsPlayerAlive(m_nearestEnemy[client])))
	{
		FindFriendsAndEnemiens(client);
		FindEnemyEntities(client);
	}

	if (!m_hasEnemiesNear[client])
	{
		FinishCurrentProcess(client, true);
		if (m_lowHealth[client])
			SetProcess(client, PRO_IDLE, crandomfloat(2.0, 4.0), "", true);
	}

	CheckHideFromDanger(client);
	CheckHealth(client);
	CheckAmmo(client);
	LookUpdate(client);

	m_nextStuckCheck[client] = GetGameTime() + 6.0;

	if (IsSniper(client))
	{
		if (TF2_IsPlayerInCondition(m_nearestEnemy[client], TFCond_Zoomed) && ClientViews(m_nearestEnemy[client], client))
			RandomMovement(client);
		else if (m_enemyDistance[client] < Squared(m_eBotDodgeRange[client]) && IsClientAimingToMe(m_nearestEnemy[client], client) && IsAttacking(m_nearestEnemy[client]))
			RandomMovement(client);
		else
		{
			m_moveVel[client][0] = 0.0;
			m_moveVel[client][1] = 0.0;
			m_moveVel[client][2] = 0.0;
		}
	}
	else if (m_class[client] == TFClass_Scout || m_class[client] == TFClass_Pyro)
	{
		if (m_enemyDistance[client] > Squared(GetConVarInt(m_eBotDodgeRangeMin)))
		{
			if (IsValidClient(m_goalEntity[client]))
				FollowPath(client, GetOrigin(m_goalEntity[client]));
			else if (m_lastFailedEntity[client] != m_nearestEnemy[client])
				m_goalEntity[client] = m_nearestEnemy[client];
		}
		else if (ClientViews(m_nearestEnemy[client], client))
		{
			RandomMovement(client);

			if (m_class[client] == TFClass_Scout && m_primaryID[client] != 772 && crandomint(1, 2) == 1)
			{
				m_buttons[client] |= IN_DUCK;
				m_buttons[client] |= IN_JUMP;
			}
		}

		return;
	}
	else if (m_enemyDistance[client] > Squared(m_eBotDodgeRange[client]))
		FinishCurrentProcess(client, true);

	if (ClientViews(m_nearestEnemy[client], client))
	{
		RandomMovement(client);
		m_goalEntity[client] = m_nearestEnemy[client];
	}
}

stock void RandomMovement(const int client)
{
	if (m_wasdTimer[client] < GetGameTime())
	{
		float val;
		if (m_difficulty[client] >= 2)
		{
			if (m_nearestEnemy[client] > 0)
			{
				val = GetVectorDistance(GetOrigin(client), GetOrigin(m_nearestEnemy[client]), true);
				if (m_class[client] == TFClass_Pyro && IsWeaponSlotActive(client, 0) && val < Squaredf(512.0))
				{
					if (m_class[m_nearestEnemy[client]] == TFClass_Pyro && GetClientHealth(client) <= GetClientHealth(m_nearestEnemy[client]))
					{
						MoveOut(client, GetOrigin(m_nearestEnemy[client]));
						if (m_isSlowThink[client])
							m_buttons[client] |= IN_ATTACK2;
					}
					else
						BackstabMove(client, m_nearestEnemy[client], true);

					m_wasdTimer[client] = GetGameTime() + crandomfloat(0.128, 0.384);
					return;
				}
				else if ((m_class[m_nearestEnemy[client]] == TFClass_Pyro || IsWeaponSlotActive(m_nearestEnemy[client], 2)) && val < Squaredf(512.0))
				{
					MoveOut(client, GetOrigin(m_nearestEnemy[client]));
					m_wasdTimer[client] = GetGameTime() + crandomfloat(0.128, 0.384);
					return;
				}
			}
		}

		val = GetMaxSpeed(client);
		m_moveVel[client][0] = crandomint(1, 2) == 1 ? -val : val;
		m_moveVel[client][1] = crandomint(1, 2) == 1 ? -val : val;
		m_wasdTimer[client] = GetGameTime() + crandomfloat(0.128, 0.512);
	}
}

stock void AttackEnd(const int client)
{
	if (ChanceOf(GetConVarInt(m_eBotDodgeRangeChance)))
		m_eBotDodgeRange[client] = crandomint(GetConVarInt(m_eBotDodgeRangeMin), GetConVarInt(m_eBotDodgeRangeMax));

	DeletePathNodes(client);
}
