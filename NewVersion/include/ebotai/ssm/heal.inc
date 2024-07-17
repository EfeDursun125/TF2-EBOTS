public bool HealRequirements(const int client)
{
	if (TF2_HasTheFlag(client))
		return false;

	return true;
}

public void HealStart(const int client)
{
	m_goalEntity[client] = -1;
	EquipWeaponSlot(client, 1);
}

public void HealUpdate(const int client)
{
	if (TF2_HasTheFlag(client))
	{
		FinishCurrentProcess(client);
		return;
	}

	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);

	if (m_hasEnemiesNear[client] && m_hasFriendsNear[client] && m_friendDistance[client] > m_enemyDistance[client])
	{
		if (GetConVarInt(EBotDifficulty) <= 1 || (GetConVarInt(EBotDifficulty) == 2 && ChanceOf(m_eBotSenseChance[client])) || (GetConVarInt(EBotDifficulty) == 3 && ChanceOf(m_eBotSenseChance[client]) && !m_lowHealth[client]))
			SetProcess(client, PRO_ATTACK, 180.0, "", true);
	}

	if (m_hasFriendsNear[client])
	{
		m_healTarget[client] = m_nearestFriend[client];
		CurrentProcessTime[client] = GetGameTime() + m_difficulty[client] + 8.0; // no timeout if we have friends nearby
	}

	if (IsValidClient(m_healTarget[client]))
	{
		if (GetVectorDistance(GetOrigin(client), GetOrigin(m_healTarget[client]), true) < Squared(GetConVarInt(EBotMedicFollowRange)))
		{
			if (!m_isSlowThink[client])
			{
				if (!IsWeaponSlotActive(client, 1))
				{
					EquipWeaponSlot(client, 1);
					return;
				}

				if (IsWeaponSlotActive(client, 1) || (m_primaryID[client] == 1079 || m_primaryID[client] == 305))
				{
					m_lookAt[client] = GetEyePosition(m_healTarget[client]);
					m_attackTimer[client] = GetGameTime() + 0.4;

					if (IsValidClient(TF2_GetHealingTarget(client)) && TF2_GetHealingTarget(client) != m_healTarget[client])
					{
						m_attackTimer[client] = 0.0;
						BackstabMove(client, m_healTarget[client], false);
					}
					else if (m_hasEnemiesNear[client])
					{
						if (!ClientViews(client, m_nearestEnemy[client]) || TF2_IsPlayerInCondition(m_nearestEnemy[client], TFCond_Zoomed) || m_class[m_nearestEnemy[client]] == TFClass_Spy)
							BackstabMove(client, m_healTarget[client], false);

						if (GetClientHealth(client) < 62 || m_class[m_nearestEnemy[client]] == TFClass_Spy)
							m_buttons[client] |= IN_ATTACK2;
					} // i don't see anyone?!
					else if (IsAttacking(m_healTarget[client]))
						BackstabMove(client, m_healTarget[client], false);
				}
			}
			else if (m_hasEnemiesNear[client] && IsValidClient(m_nearestEnemy[client]))
			{
				float duckduck[3], height[3];
				duckduck = GetOrigin(client);
				height = duckduck;
				duckduck[2] += 36.0;
				height[2] += 72.0;
				if (!IsVisible(duckduck, GetEyePosition(m_nearestEnemy[client])) && IsVisible(height, GetEyePosition(m_nearestEnemy[client])))
					m_duckTimer[client] = GetGameTime() + 1.25;
				else
				    m_duckTimer[client] = 0.0;
			}
		}
		else
		{
			if (IsWeaponSlotActive(client, 1) || (m_primaryID[client] == 1079 || m_primaryID[client] == 305))
			{
				m_attackTimer[client] = GetGameTime() + 0.4;
				m_lookAt[client] = GetEyePosition(m_healTarget[client]);
			}

			if (IsWaypointReachable(GetCenter(client), GetCenter(m_healTarget[client])))
			{
				DeletePathNodes(client);
				MoveTo(client, GetOrigin(m_healTarget[client]));
			}
			else
				FollowPath(client, GetOrigin(m_healTarget[client]));
		}
	}

	if (!IsValidClient(m_healTarget[client]) || !IsPlayerAlive(m_healTarget[client]) || (GameRules_GetProp("m_bPlayingMedieval") && GetClientHealth(m_healTarget[client]) >= GetMaxHealth(m_healTarget[client])))
	{
		m_healTarget[client] = -1;
		FinishCurrentProcess(client);
	}
}

public void HealEnd(const int client)
{
	m_healTarget[client] = -1;
	m_goalEntity[client] = -1;
	EquipWeaponSlot(client, 0);
}
